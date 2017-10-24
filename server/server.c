#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <png.h>


#include "server.h"
#include "ui.h"
#include "graphics.h"

#define CONNECT_DELAY       15
#define SELECT_TO           200
#define INET_PORT           8888

struct game game;

struct observerMethods GUI;


/* A coloured pixel. */
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} pixel_t;

/* A picture. */
typedef struct  {
    pixel_t *pixels;
    size_t width;
    size_t height;
} bitmap_t;


bitmap_t map[15];
FILE *logFile = NULL;
char debugLvl = 0;

#define log     __mylog

/* PNG code  from https://www.lemoda.net/c/write-png/ */

  




    

    
/* Given "bitmap", this returns the pixel of bitmap at the point 
   ("x", "y"). */

static pixel_t * pixel_at (bitmap_t * bitmap, int x, int y)
{
    return bitmap->pixels + bitmap->width * y + x;
}
    
/* Write "bitmap" to a PNG file specified by "path"; returns 0 on
   success, non-zero on error. */

static int save_png_to_file (bitmap_t *bitmap, const char *path)
{
    FILE * fp;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    size_t x, y;
    png_byte ** row_pointers = NULL;
    /* "status" contains the return value of this function. At first
       it is set to a value which means 'failure'. When the routine
       has finished its work, it is set to a value which means
       'success'. */
    int status = -1;
    /* The following number is set by trial and error only. I cannot
       see where it it is documented in the libpng manual.
    */
    int pixel_size = 3;
    int depth = 8;
    
    fp = fopen (path, "wb");
    if (! fp) {
        goto fopen_failed;
    }

    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        goto png_create_write_struct_failed;
    }
    
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL) {
        goto png_create_info_struct_failed;
    }
    
    /* Set up error handling. */

    if (setjmp (png_jmpbuf (png_ptr))) {
        goto png_failure;
    }
    
    /* Set image attributes. */

    png_set_IHDR (png_ptr,
                  info_ptr,
                  bitmap->width,
                  bitmap->height,
                  depth,
                  PNG_COLOR_TYPE_RGB,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_DEFAULT,
                  PNG_FILTER_TYPE_DEFAULT);
    
    /* Initialize rows of PNG. */

    row_pointers = png_malloc (png_ptr, bitmap->height * sizeof (png_byte *));
    for (y = 0; y < bitmap->height; y++) {
        png_byte *row = 
            png_malloc (png_ptr, sizeof (uint8_t) * bitmap->width * pixel_size);
        row_pointers[y] = row;
        for (x = 0; x < bitmap->width; x++) {
            pixel_t * pixel = pixel_at (bitmap, x, y);
            *row++ = pixel->red;
            *row++ = pixel->green;
            *row++ = pixel->blue;
        }
    }
    
    /* Write the image data to "fp". */

    png_init_io (png_ptr, fp);
    png_set_rows (png_ptr, info_ptr, row_pointers);
    png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    /* The routine has successfully written the file, so we set
       "status" to a value which indicates success. */

    status = 0;
    
    for (y = 0; y < bitmap->height; y++) {
        png_free (png_ptr, row_pointers[y]);
    }
    png_free (png_ptr, row_pointers);
    
 png_failure:
 png_create_info_struct_failed:
    png_destroy_write_struct (&png_ptr, &info_ptr);
 png_create_write_struct_failed:
    fclose (fp);
 fopen_failed:
    return status;
}


int read_from_client (struct team *t, char *buffer, int maxSize) {
    int nbytes;
    int i;

    nbytes = read (t->sock, buffer, maxSize);
    if (nbytes <= 0)
        /* Read error or End-of-file */
        return -1;

    debug (1, KNRM, "[DEBUG] received %d bytes : ", nbytes);
    for (i=0; i<nbytes; i++)
        debug (1, KNRM, "0x%02X ", (unsigned char) buffer[i]);
    debug (1, KNRM, "\n");

    return nbytes;
}

void write_to_client (struct team *t, const char *buf, size_t size) {
    write (t->sock, buf, size);
}

int load_teams_file (const char *filename) {
    char buf[21+MAXNAMESIZE];
    int i,j;
    char comment = 0, lineFollow = 0;
    FILE * teamsFile = fopen (filename, "r");

    char str[] = "Reading team file '                                                          ";
    size_t len = strlen (filename);
    len = len > 25 ? 25 : len;

    strncpy (str+19, filename, 25);
    strncpy (str+19+len, "'...                                                          ", 58-len);

    printf ("%s", str);
    fflush (stdout);

    if (teamsFile == NULL) {
        printf ("[" "\x1B[31m" "KO" RESET "]\n");
        fprintf (stderr, "Could not open file %s.\n", filename);
        exit (EXIT_FAILURE);
    }

    memset (buf, 0, sizeof (buf));

    for (i = 0, j = 0; i < MAXTEAM && fgets (buf, 21+MAXNAMESIZE, teamsFile); memset (buf, 0, sizeof (buf)), j++) {
        size_t l;
        char * name;

        if (buf[0] == '\n') {
            lineFollow = 0;
            continue;
        }

        comment = (buf[0] == '#' || (comment && lineFollow));

        l = strlen (buf);
        if (buf[l-1] == '\n') {
            lineFollow = 0;
            buf[--l] = '\0';
        } else
            lineFollow = 1;

        if (comment)
            continue;

        name = strchr (buf+2, ' ');

        if (name == NULL) {
            printf ("[" "\x1B[31m" "KO" RESET "]\n");
            fprintf (stderr, "Error in team file %s (l.%d)\n", filename, j);
            exit (EXIT_FAILURE);
        }

        *name = '\0';
        name++;

        if (l-(name-buf) == 0) {
            printf ("[" "\x1B[31m" "KO" RESET "]\n");
            fprintf (stderr, "Error in team file %s (l.%d)\n", filename, j);
            exit (EXIT_FAILURE);
        }

        game.teams[i].connected = 0;

        if (buf[0] - '0' == RBT_EV3_BT)
            game.teams[i].robotType = RBT_EV3_BT;
        else if (buf[0] - '0' == RBT_EV3_IN)
            game.teams[i].robotType = RBT_EV3_IN;
        else {
            printf ("[" "\x1B[31m" "KO" RESET "]\n");
            fprintf (stderr, "Error in team file %s (l.%d)\n", filename, j);
            exit (EXIT_FAILURE);
        }

        if (game.teams[i].robotType != RBT_EV3_IN)
            str2ba (buf+2, &game.teams[i].address.bt);
        else
            if (inet_pton (AF_INET, buf+2, &game.teams[i].address.in) == 0) {
                printf ("[" "\x1B[31m" "KO" RESET "]\n");
                fprintf (stderr, "Error in team file %s (l.%d)\n", filename, j);
                exit (EXIT_FAILURE);
            }

        strncpy (game.teams[i].name, name, MAXNAMESIZE);
        game.teams[i].name[MAXNAMESIZE] = '\0';

        i++;
    }

    fclose (teamsFile);

    printf ("[" "\x1B[32m" "OK" RESET "]\n");

    return i;
}

char alinea [MAXNAMESIZE+4] = {0};

void sendStop (int teamID) {
    char buf[8];

    if (game.teams[teamID].kicked)
        game.teams[teamID].ended = 1;

    if (!game.teams[teamID].active || game.teams[teamID].ended)
        return;

    game.teams[teamID].ended = 1;

    if (game.teams[teamID].connected) {
        buf[0] = game.teams[teamID].idServMsg % 0xFF;
        buf[1] = game.teams[teamID].idServMsg >> 8;
        buf[2] = -1;                        /* server ID is 0xFF */
        buf[3] = (char) (0xFF & (teamID+1));
        buf[4] = MSG_STOP;                  /* This is a STOP message */

        write_to_client (&game.teams[teamID], buf, 5);
    }
}

void sendKick (int teamID) {
    char buf[8];
    int i;

    if (!game.teams[teamID].active || game.teams[teamID].kicked || game.teams[teamID].ended)
        return;

    pthread_mutex_lock (&game.lock);

    buf[2] = -1;                        /* server ID is 0xFF */
    buf[4] = MSG_KICK;                  /* This is a KICK message */
    buf[5] = (teamID+1);

    for (i=0; i < game.nbTeams; i++)
        if (game.teams[i].connected) {
            buf[3] = (char) (0xFF & (i+1));             /* receiver */
            buf[0] = game.teams[i].idServMsg % 0xFF;
            buf[1] = game.teams[i].idServMsg >> 8;
            game.teams[i].idServMsg ++;
            write_to_client (&game.teams[i], buf, 6);
        }

    if (game.teams[teamID].connected) {
        close (game.teams[teamID].sock);
        game.teams[teamID].connected = 0;
    }
    game.teams[teamID].active = 0;
    game.teams[teamID].kicked = 1;

    log (KNRM, alinea);
    log (COL(teamID), game.teams[teamID].name);
    log (KRED, " has been kicked!\n");



    pthread_mutex_unlock (&game.lock);
}

void parseMessage (int sendingTeam, const unsigned char *buf, int nbbytes) {
    uint16_t id;

    
    char teamAlinea [MAXNAMESIZE+4] = {0};
    int i;
    int l = strlen (game.teams[sendingTeam].name);
    int consumedBytes = 5;

    id = *((uint16_t *) buf);

    for (i=0; i<MAXNAMESIZE-l; i++)
        teamAlinea [i] = ' ';

    teamAlinea [i] = '[';
    log (KNRM, teamAlinea);
    log (COL(sendingTeam),"%s", game.teams[sendingTeam].name);
    log (KNRM, "] ");

    if (nbbytes < 5) {
        log (KRED, "*** header is too short (%d bytes) ***\n", nbbytes);
        return;
    }

    if (buf[2]-1 != sendingTeam) {
        log (KRED, "*** mediocre spoofing attempt detected ***\n");
        if (nbbytes > 5)
            parseMessage (sendingTeam, buf+5, nbbytes-5);
        return;
    }

    if (buf[3] != 0xFF && (buf[3] == 0 || buf[3] > game.nbTeams || !game.teams[buf[3]-1].active)) {
        log (KRED, "*** unknown or inactive receiver (%d) ***\n", buf[3]);
        if (nbbytes > 5)
            parseMessage (sendingTeam, buf+5, nbbytes-5);
        return;
    }





    switch (buf[4]) {
        case MSG_ACK:
            {
                /* ACK */
                uint16_t idAck;

                if (nbbytes < 8) {
                    log (KRED, "*** ACK message is too short (%d bytes) ***\n", nbbytes);
                    return;
                }

                consumedBytes = 8;

                if (buf[3] == 0xFF) {
                    log (KRED, "*** ACK messages sent to server are ignored ***\n");
                    break;
                }
/*
                if (buf[3]-1 != game.teams[sendingTeam].ally) {
                    log (KRED, "*** Can't send ACK message to team ");
                    log (COL(buf[3]-1), "%s", game.teams[buf[3]-1].name);
                    log (KRED, " ***\n");
                    break;
                }*/

                idAck = *((uint16_t *) &buf[5]);

                log (KNRM, "id=%d dest=", id);
                log (COL(buf[3]-1), "%s\n", game.teams[buf[3]-1].name);
                log (KNRM, alinea);
                log (KNRM, "          ACK      idAck=%d status=%d\n", idAck, buf[7]);

                if (buf[3] != 0xFF && !game.teams[buf[3]-1].connected) {
                    log (KRED, "*** Team ");
                    log (COL(buf[3]-1), "%s", game.teams[buf[3]-1].name);
                    log (KRED, " is not connected. Message discarded ***\n");
                    break;
                }

                if (buf[3] != 0xFF) {
                    write_to_client (&game.teams[buf[3]-1], (char *) buf, 8);
                }

                break;
            }

        case MSG_START:
            {
                /* START */
                log (KRED, "*** Tried to START the game ***\n");
                break;
            }
        case MSG_STOP:
            {
                /* STOP */
                log (KRED, "*** Tried to STOP the game ***\n");
                break;
            }
    

		case MSG_CUSTOM:
            {
                /* CUSTOM */
                int i;

                consumedBytes = nbbytes;

                if (buf[3] == 0xFF) {
                    log (KRED, "*** Can't send CUSTOM message to server ***\n");
                    break;
                }

              /*  if (buf[3]-1 != game.teams[sendingTeam].ally) {
                    log (KRED, "*** Can't send CUSTOM message to team ");
                    log (COL(buf[3]-1), "%s", game.teams[buf[3]-1].name);
                    log (KRED, " ***\n");
                    break;
                }*/

                log (KNRM, "id=%d dest=", id);
                log (COL(buf[3]-1), "%s\n", game.teams[buf[3]-1].name);
                log (KNRM, alinea);
                log (KNRM, "          CUSTOM   content=");
                for (i=5; i<nbbytes; i++) {
                    log (KNRM, "%02X", (unsigned char) buf[i]);
                    if ((i-5) % 4 == 3)
                        log (KNRM, " ");
                }
                log (KNRM, "\n");

                if (!game.teams[buf[3]-1].connected) {
                    log (KRED, "*** Team ");
                    log (COL(buf[3]-1), "%s", game.teams[buf[3]-1].name);
                    log (KRED, " is not connected. Message discarded ***\n");
                    break;
                }

                write_to_client (&game.teams[buf[3]-1], (char *) buf, nbbytes);

                break;
            }

        case MSG_POSITION:
            {
                /* POSITION */
                int16_t x, y;

                if (nbbytes < 9) {
                    log (KRED, "*** POSITION message is too short (%d bytes) ***\n", nbbytes);
                    return;
                }

                consumedBytes = 9;

                if (buf[3] != 0xFF) {
                    log (KRED, "*** Can't send POSITION message to team ");
                    log (COL(buf[3]-1), "%s", game.teams[buf[3]-1].name);
                    log (KRED, " ***\n");
                    break;
                }
                

                x = *((int16_t *) &buf[5]);
                y = *((int16_t *) &buf[7]);

                log (KNRM, "id=%d", id);
                log (KNRM, alinea);
                log (KNRM, "          POSITION x=%d y=%d\n", x, y);

                addCoordinate (sendingTeam, x, y);

                break;
            }
        case MSG_MAPDATA:
            {
                /* Get MAP DATA */                
			int16_t x, y;
			pixel_t * pixel;

            if (nbbytes < 12) {
                    log (KRED, "*** MAPDATA message is too short (%d bytes) ***\n", nbbytes);
                    return;
                }

            consumedBytes = 12;

            x = *((int16_t *) &buf[5]);
            y = *((int16_t *) &buf[7]);


			pixel = pixel_at (& map[sendingTeam], x, y);
            pixel->red = *((int8_t *) &buf[9]);
            pixel->green = *((int8_t *) &buf[10]);
            pixel->blue = *((int8_t *) &buf[11]);
			

			log (KNRM, "id=%d", id);
            log (KNRM, alinea);
            log (KNRM, "          MAPDATA AT x=%d y=%d r=%d g=%d b=%d\n", x, y, pixel->red, pixel->green, pixel->blue);
                break;
            }
		case MSG_MAPDONE:
			{
			char filename[10];
            if (nbbytes < 5) {
                    log (KRED, "*** MAPDONE message is too short (%d bytes) ***\n", nbbytes);
                    return;
                }
			consumedBytes=5;
			/*Entire Map received, now draw it*/
             log (KRED, "Writing map\n");
			sprintf(filename, "map%d.png", sendingTeam);
 			save_png_to_file (& map[sendingTeam], filename);

		    /*free (map[sendingTeam].pixels);*/
			break;
			}
		case MSG_OBSTACLE:
			{
                /* OBSTACLE */
                int16_t x, y;

                if (nbbytes < 10) {
                    log (KRED, "*** OBSTACLE message is too short (%d bytes) ***\n", nbbytes);
                    return;
                }

                consumedBytes = 10;

				
                x = *((int16_t *) &buf[6]);
                y = *((int16_t *) &buf[8]);

				if (buf[5] > 1) {
                    log (KRED, "*** Illegal BALL action (%d) ***\n", buf[5]);
                    break;
                }


				else if (buf[5]==1){

					log (KNRM, "id=%d", id);
                	log (KNRM, alinea);
                	log (KNRM, "         PICKING UP OBSTACLE \n");
				}
				else {				
   		            log (KNRM, "id=%d", id);
					log (KNRM, alinea);
                	log (KNRM, "          DROPPING OBSTACLE AT x=%d y=%d\n", x, y);
				}
                break;
			}
        default:
            {
                log (KRED, "*** unknown message type 0x%02X ***\n", (unsigned char) buf[4]);
                break;
            }
    }

    if (nbbytes > consumedBytes)
        parseMessage (sendingTeam, buf+consumedBytes, nbbytes-consumedBytes);
}

void usage (const char *execName) {
    fprintf (stderr, "Usage: %s [-hvx", execName);
#ifdef GRAPHICS
    fprintf (stderr, "d");
#endif
    fprintf (stderr, "] [-o outputFile] teamFile\n");
    fprintf (stderr, "             -h: print this message\n");
    fprintf (stderr, "             -v: verbose mode\n");
    fprintf (stderr, "             -x: use terminal instead of curses\n");
#ifdef GRAPHICS
    fprintf (stderr, "             -d: display robot's advertised path\n");
#endif
    fprintf (stderr, "  -o outputFile: log session into file\n");
}

int main (int argc, char **argv) {

    struct sockaddr_rc loc_addr_bt = { 0 }, rem_addr_bt = { 0 };
    struct sockaddr_in loc_addr_in = { 0 }, rem_addr_in = { 0 };
    int serverSockBT, serverSockIN, fdmax, i;
    socklen_t opt_bt = sizeof(rem_addr_bt);
    socklen_t opt_in = sizeof(rem_addr_in);
    fd_set read_fd_set;
    char curses = 1;
	int team;
    char displayPath = 0;
    int c;



    

    char buf[MAXMSG+1] = { 0 };

    memset(game.teams, 0, MAXTEAM * sizeof(struct team));

    opterr = 0;

/* Create an image. */
	for (team=0; team<15; team++){


    map[team].width = 40;
    map[team].height = 40;

    map[team].pixels = calloc (map[team].width * map[team].height, sizeof (pixel_t));

    if (! map[team].pixels) {
		return -1;
    }
	}
    while ((c = getopt (argc, argv, "hxdvo:")) != -1)
        switch (c) {
            case 'x':
                curses = 0;
                break;
            case 'd':
                displayPath = 1;
                break;
            case 'v':
                debugLvl = 1;
                break;
            case 'o':
                if (access (optarg, F_OK) != -1) {
                    char *p;
                    fprintf (stderr, "File '%s' already exists. Should it be deleted? (Y/n) ", optarg);
                    fflush (stdout);
                    p = fgets (buf, 3, stdin);
                    if (p != NULL && (*p == 'n' || *p == 'N'))
                        return 0;
                }

                logFile = fopen (optarg, "w");

                if (logFile == NULL) {
                    fprintf (stderr, "Could not open log file %s.\n", optarg);
                    exit (EXIT_FAILURE);
                }
                break;
            case 'h':
                usage (argv[0]);
                exit (0);
            case '?':
                if (optopt == 'o')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                exit (EXIT_FAILURE);
            default:
                exit (EXIT_FAILURE);
        }

    if (argc != optind + 1) {
        usage (argv[0]);
        exit (EXIT_FAILURE);
    }

    srand(time(NULL));

    game.state = GAM_INIT;

    printf ("\n\n");
    printf ("\x1B[31m                                           )  (      (                            \n");
    printf (        "                                        ( /(  )\\ )   )\\ )                         \n");
    printf (        "       (     (  (     (           )     )\\())(()/(  (()/(   (  (    )     (  (    \n");
    printf (        "       )\\   ))\\ )(   ))\\ (  (    (     ((_)\\  /(_))  /(_)) ))\\ )(  /((   ))\\ )(   \n");
    printf (        "      ((_) /((_|()\\ /((_))\\ )\\   )\\  '   ((_)(_))   (_))  /((_|()\\(_))\\ /((_|()\\  \n");
    printf (RESET   "      | __" "\x1B[31m" "(_))( ((_|_)) ((_|(_)_((_))   " RESET "/ _ \\/ __|  / __|" "\x1B[31m" "(_))  ((_))((_|_))  ((_) \n");
    printf (RESET   "      | _|| || | '_/ -_) _/ _ \\ '  \\" "\x1B[31m" "() " RESET "| (_) \\__ \\  \\__ \\/ -_)| '_\\ V // -_)| '_| \n");
    printf (        "      |___|\\_,_|_| \\___\\__\\___/_|_|_|   \\___/|___/  |___/\\___||_|  \\_/ \\___||_|  \n");
    printf ("\n\n");

    /* create server sockets */
    printf ("Creating server socket...                                                    ");
    fflush (stdout);
    serverSockBT = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    serverSockIN = socket(AF_INET, SOCK_STREAM, 0);

    /* bind bluetooth socket to port 1 of the first available local bluetooth adapter */
    loc_addr_bt.rc_family = AF_BLUETOOTH;
    loc_addr_bt.rc_bdaddr = *BDADDR_ANY;
    loc_addr_bt.rc_channel = (uint8_t) 1;
    bind (serverSockBT, (struct sockaddr *) &loc_addr_bt, sizeof (loc_addr_bt)); 

    /* bind INET socket */
    loc_addr_in.sin_family = AF_INET;
    loc_addr_in.sin_port = htons (INET_PORT);
    loc_addr_in.sin_addr.s_addr = INADDR_ANY;
    i = 1;
    if (setsockopt(serverSockIN, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int)) == -1) {
        printf ("[" "\x1B[31m" "KO" RESET "]\n");
        fprintf (stderr, "setsockopt failed.\n");
        exit (EXIT_FAILURE);
    }
    bind (serverSockIN, (struct sockaddr *) &loc_addr_in, sizeof (loc_addr_in));

    /* put sockets into listening mode */
    listen (serverSockBT, MAXTEAM);
    listen (serverSockIN, MAXTEAM);
    printf ("[" "\x1B[32m" "OK" RESET "]\n");

    /* load teams from file */
    game.nbTeams = load_teams_file (argv[optind]);
    if (game.nbTeams == 0) {
        fprintf (stderr, "Team file should at least define one team.\n");
        exit (EXIT_FAILURE);
    }
    printf ("    + %d teams have been loaded\n", game.nbTeams);

    c = init_gui (curses, &GUI);
    if (!c && curses) {
        fprintf (stderr, "Couldn't init curses. Falling back to standard terminal...\n");
        c = init_gui (0, &GUI);
    }
    if (!c) {
        fprintf (stderr, "Couln't init user interface...\n");
        exit (EXIT_FAILURE);
    }

    if (displayPath && graphicsInit () < 0) {
        fprintf (stderr, "Couln't init graphics...\n");
        exit (EXIT_FAILURE);
    }

    for (c=0; c<MAXNAMESIZE+3; c++)
        alinea[c] = ' ';

    log (KNRM, "\n");
    log (KNRM, alinea);
    log (KRED, "Contest starts now!\n");

	 while (game.state != GAM_END) {
        time_t startTime;
        int rankCmp;

        for (i=0; i<game.nbTeams; i++) {
            game.teams[i].kicked = 0;
            game.teams[i].ended = 0;
        }

        game.ballStatus[0] = 0;
        game.ballStatus[1] = 0;

        game.state = GAM_TEAM_SELECT;
        rankCmp = (*GUI.getTeamsForGame) ();

        if (rankCmp == 0) {
            game.state = GAM_END;
            break;
        }

        if (rankCmp == 2)
            game.leaders[1] = 0xFF;

        for (i=0; i<game.nbTeams; i++) {
            if (game.teams[i].active && game.leaders[game.teams[i].side] != i) {
                game.teams[i].ally = game.leaders[game.teams[i].side];
                game.teams[game.leaders[game.teams[i].side]].ally = i;
            }
        }

        log (KNRM, alinea);
        log (KRED, "Starting game with teams ");
        log (COL(game.leaders[0]), "%s", game.teams[game.leaders[0]].name);
        log (KRED, ", ");
        log (COL(game.teams[game.leaders[0]].ally), "%s", game.teams[game.teams[game.leaders[0]].ally].name);
        if (rankCmp == 4) {
            log (KRED, ", ");
            log (COL(game.leaders[1]), "%s", game.teams[game.leaders[1]].name);
            log (KRED, ", ");
            log (COL(game.teams[game.leaders[1]].ally), "%s", game.teams[game.teams[game.leaders[1]].ally].name);
        }
        log (KRED, ".\n");



        startTime = time (NULL) + CONNECT_DELAY;
        game.state = GAM_CONNECTING;

        (*GUI.monitorMaster) ();
  /*      if (displayPath && graphicsInitWindow (
                    game.leaders[0],
                    game.teams[game.robots[0]].ally,
                    rankCmp == 4 ? game.leaders[1] : -1,
                    rankCmp == 4 ? game.teams[game.leaders[1]].ally : -1
                    ) < 0) {
            log (KNRM, alinea);
            log (KRED, "Failed to init graphics window...\n");
        }

        if (displayPath) {
            if (rankCmp == 4) {
                addCoordinate (game.leaders[0], 30, 30);
                addCoordinate (game.teams[game.leaders[0]].ally, 90, 370);
                addCoordinate (game.leaders[1], -30, 30);
                addCoordinate (game.teams[game.leaders[1]].ally, -90, 370);
            } else {
                addCoordinate (game.leaders[0], 90, 30);
                addCoordinate (game.teams[game.leaders[0]].ally, 30, 170);
            }
        } */

        while (game.state == GAM_CONNECTING || game.state == GAM_RUNNING) {
            int selectRet, everyoneConnected;
            struct timeval tv;
            time_t now = time (NULL);

            fdmax = serverSockBT > serverSockIN ? serverSockBT : serverSockIN;
            FD_ZERO (&read_fd_set);
            FD_SET (serverSockBT, &read_fd_set);
            FD_SET (serverSockIN, &read_fd_set);

            everyoneConnected = 1;

            for (i=0; i<game.nbTeams; i++)
                if (game.teams[i].connected) {
                    FD_SET (game.teams[i].sock, &read_fd_set);
                    if (game.teams[i].sock > fdmax)
                        fdmax = game.teams[i].sock;
                } else if (game.teams[i].active)
                    everyoneConnected = 0;

            if ((now >= startTime || everyoneConnected) && game.state == GAM_CONNECTING) {
                char first = 1;

                log (KNRM, alinea);
                log (KRED, "Game starts NOW !\n");

                buf[0] = 0x00;                      /* ID of start message */
                buf[1] = 0x00;                      /*   is 0000 */
                buf[2] = -1;                        /* server ID is 0xFF */
                buf[4] = MSG_START;                 /* This is a START message */

                for (i=0; i<game.nbTeams; i++) {
                    if (game.teams[i].active) {
                        if (game.teams[i].connected) {
                            game.teams[i].idServMsg = 1;

                            buf[3] = (char) (0xFF & (i+1));                     /* receiver */

                            write_to_client (&game.teams[i], buf, 8);
                        } else {
                            if (first) {
                                log (KNRM, alinea);
                                first = 0;
                            } else
                                log (KRED, ", ");
                            log (COL(i), "%s", game.teams[i].name);
                        }
                    }
                }

                if (!first)
                    log (KRED, " failed to connect.\n");

                game.state = GAM_RUNNING;
            }

            tv.tv_sec = 0;
            tv.tv_usec = SELECT_TO;
            do {
                selectRet = select (fdmax+1, &read_fd_set, NULL, NULL, &tv);
                if (game.state == GAM_STOP)
                    break;
            } while (selectRet < 0);

            if (selectRet < 0) {
                break;
                /*
                if (game.state == GAM_CONNECTING || game.state == GAM_RUNNING) {
                    (*GUI.destroyUI) ();
                    sleep (5);
                    fprintf (stderr, "Error when select.\n");
                    exit (EXIT_FAILURE);
                }
                */
            } else {

                if (FD_ISSET (serverSockBT, &read_fd_set)) {
                    /* accept one connection from bluetooth */
                    int client = accept (serverSockBT, (struct sockaddr *) &rem_addr_bt, &opt_bt);

                    pthread_mutex_lock (&game.lock);

                    for (i=0; i<game.nbTeams; i++)
                        if (!game.teams[i].connected &&
                            game.teams[i].robotType == RBT_EV3_BT &&
                            memcmp (&game.teams[i].address.bt, &rem_addr_bt.rc_bdaddr, sizeof (bdaddr_t)) == 0) {

                            if (game.teams[i].active) {
                                game.teams[i].sock = client;
                                game.teams[i].connected = 1;
                                log (KNRM, alinea);
                                log (KRED, "Team ");
                                log (COL(i), "%s", game.teams[i].name);
                                log (KRED, " is now connected.\n");
                                if (game.state == GAM_RUNNING) {
                                    game.teams[i].idServMsg = 1;
                                    buf[0] = 0x00;                          /* ID of start message      */
                                    buf[1] = 0x00;                          /*   is 0000                */
                                    buf[2] = -1;                            /* server ID is 0xFF        */
                                    buf[3] = (char) (0xFF & (i+1));             /* receiver                 */
                                    buf[4] = MSG_START;                     /* This is a START message  */
                                 

                                    write_to_client (&game.teams[i], buf, 8);

                                    (*GUI.notify) ();
                                }
                            } else {
                                log (KNRM, alinea);
                                log (KRED, "Team ");
                                log (COL(i), "%s", game.teams[i].name);
                                log (KRED, " tried to connect while not taking part in this game!\n");
                                close (client);
                            }

                            break;
                        }

                    if (i == game.nbTeams) {
                        ba2str (&rem_addr_bt.rc_bdaddr, buf );
                        log (KNRM, alinea);
                        log (KRED, "Unknown connection from BT address %s.\n", buf);
                        close (client);
                    }

                    pthread_mutex_unlock (&game.lock);
                }

                if (FD_ISSET (serverSockIN, &read_fd_set)) {
                    /* accept one connection from network */
                    int client = accept (serverSockIN, (struct sockaddr *) &rem_addr_in, &opt_in);

                    pthread_mutex_lock (&game.lock);

                    for (i=0; i<game.nbTeams; i++)
                        if (!game.teams[i].connected &&
                            game.teams[i].robotType == RBT_EV3_IN &&
                            game.teams[i].address.in.s_addr == rem_addr_in.sin_addr.s_addr) {

                            if (game.teams[i].active) {
                                game.teams[i].sock = client;
                                game.teams[i].connected = 1;
                                log (KNRM, alinea);
                                log (KRED, "Team ");
                                log (COL(i), "%s", game.teams[i].name);
                                log (KRED, " is now connected.\n");
                                if (game.state == GAM_RUNNING) {
                                    game.teams[i].idServMsg = 1;
                                    buf[0] = 0x00;                          /* ID of start message      */
                                    buf[1] = 0x00;                          /*   is 0000                */
                                    buf[2] = -1;                            /* server ID is 0xFF        */
                                    buf[3] = (char) (0xFF & (i+1));             /* receiver                 */
                                    buf[4] = MSG_START;                     /* This is a START message  */
  
                                    write_to_client (&game.teams[i], buf, 8);

                                    (*GUI.notify) ();
                                }
                            } else {
                                log (KNRM, alinea);
                                log (KRED, "Team ");
                                log (COL(i), "%s", game.teams[i].name);
                                log (KRED, " tried to connect while not taking part in this game!\n");
                                close (client);
                            }

                            break;
                        }

                    if (i == game.nbTeams) {
                        /* FIXME: inet_ntoa is deprecated */
                        log (KNRM, alinea);
                        log (KRED, "Unknown connection from network address %s.\n", inet_ntoa(rem_addr_in.sin_addr));
                        close (client);
                    }

                    pthread_mutex_unlock (&game.lock);
                }

                for (i = 0; i <= game.nbTeams; ++i)
                    if (game.teams[i].connected && FD_ISSET (game.teams[i].sock, &read_fd_set)) {
                        int nbbytes;

                        pthread_mutex_lock (&game.lock);

                        memset(buf, 0, sizeof(buf));
                        if ((nbbytes = read_from_client (&game.teams[i], buf, MAXMSG)) < 0) {
                            close (game.teams[i].sock);
                            game.teams[i].connected = 0;
                            log (KNRM, alinea);
                            log (KRED, "Team ");
                            log (COL(i), "%s", game.teams[i].name);
                            log (KRED, " has disconnected.\n");

                            (*GUI.notify) ();
                        } else if (nbbytes != 0 && game.state == GAM_RUNNING)
                            parseMessage (i, (unsigned char *) buf, nbbytes);
                        pthread_mutex_unlock (&game.lock);
                    }
            }
        }

        if (displayPath)
            graphicsDestroyWindow ();

        log (KNRM, "\n");
        log (KNRM, alinea);
        log (KRED, "End of this game.\n\n");

        buf[2] = -1;                        /* server ID is 0xFF */
        buf[4] = MSG_STOP;                  /* This is a STOP message */

        for (i = 0; i < game.nbTeams; i++) {
            if (game.teams[i].connected) {
                buf[0] = game.teams[i].idServMsg % 0xFF;    /* ID of stop message */
                buf[1] = game.teams[i].idServMsg >> 8;
                buf[3] = (char) (0xFF & (i+1));                 /* receiver */
                write_to_client (&game.teams[i], buf, 5);

                close (game.teams[i].sock);
                game.teams[i].connected = 0;
            }

            if (game.teams[i].active)
                game.teams[i].active = 0;
        }
    }

    log (KNRM, "\n");
    log (KNRM, alinea);
    log (KRED, "End of the contest.\n");

    (*GUI.destroyUI) ();
    if (displayPath)
        graphicsQuit ();

    if (logFile)
        fclose (logFile);

    close (serverSockBT);
    close (serverSockIN);

    return 0;
}
