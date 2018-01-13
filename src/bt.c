#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

//#define SERV_ADDR   "40:e2:30:50:e9:4c"     /* Valerio */
#define SERV_ADDR   "00:1a:7d:da:71:06"     /* Letitia */
#define TEAM_ID     1                       /* Your team ID */

#define MSG_ACK     0
#define MSG_START    1
#define MSG_STOP   2
#define MSG_KICK    3
#define MSG_POSITION 4
#define MSG_MAPDATA 	5
#define MSG_MAPDONE 6
#define MSG_OBSTACLE 7


void debug (const char *fmt, ...) {
  va_list argp;

  va_start (argp, fmt);

  vprintf (fmt, argp);

  va_end (argp);
}


int bt_sock;

uint16_t msgId = 0;

int read_from_server (int sock, char *buffer, size_t maxSize) {
  int bytes_read = read (sock, buffer, maxSize);

  if (bytes_read <= 0) {
    fprintf (stderr, "[BT] -  Server unexpectedly closed connection...\n");
    close (bt_sock);
    exit (EXIT_FAILURE);
  }

  printf ("[BT] - received %d bytes\n", bytes_read);

  return bytes_read;
}

void robot () {
  char string[58];
  char type;
  printf ("[BT] - I'm navigating...\n");

	srand(time(NULL));
    /* Send 3 POSITION messages, a BALL message, 1 position message, then a NEXT message */
  int i, j;
  for (i=0; i<3; i++){
    string[0] = msgId % 0xFF;
    string[1] = msgId >> 8;
  	msgId++;
 	  string[2] = TEAM_ID;
    string[3] = 0xFF;
    string[4] = MSG_POSITION;
    string[5] = i;          /* x */
		string[6] = 0x00;
    string[7] = i;		/* y */
		string[8]= 0x00;
		write(bt_sock, string, 9);
  }

  printf ("[BT] - I'm sending my map...\n");
	/* MAP data is in the form MAPDATA | X  X |Y  Y | R | G | B */

	for (i=0; i<10; i++){
		for (j=0; j<10; j++){
      string[0] = msgId % 0xFF;
      string[1] = msgId >> 8;
      msgId++;
			string[2] = TEAM_ID;
	    string[3] = 0xFF;
	    string[4] = MSG_MAPDATA;
    	string[5] = i;          /* x */
			string[6] = 0x00;
    	string[7] = j;		/* y */
			string[8]= 0x00;
			string[9]= rand() % 255;
			string[10]=rand() % 255;
			string[11]= rand()% 255;
			write(bt_sock, string, 12);
		}
	}
	printf("[BT] - Done sending map");
  string[0] = msgId % 0xFF;
  string[1] = msgId >> 8;
  msgId++;
	string[2] = TEAM_ID;
	string[3] = 0xFF;
	string[4] = MSG_MAPDONE;
	write(bt_sock, string, 5);

	printf("[BT] - I'm waiting for the stop message");
	while(1){
		//Wait for stop message
		read_from_server (bt_sock, string, 58);
      type = string[4];
		if (type ==MSG_STOP){
			return;
		}
	}
}


int bt_init() {
  struct sockaddr_rc addr = { 0 };
  int status;

  /* allocate a socket */
  bt_sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

  /* set the connection parameters (who to connect to) */
  addr.rc_family = AF_BLUETOOTH;
  addr.rc_channel = (uint8_t) 1;
  str2ba (SERV_ADDR, &addr.rc_bdaddr);

  /* connect to server */
  status = connect(bt_sock, (struct sockaddr *)&addr, sizeof(addr));

  /* if connected */
  if( status == 0 ) {
    char string[58];

    /* Wait for START message */
    read_from_server (bt_sock, string, 9);
    if (string[4] == MSG_START) {
      printf ("[BT] - Received start message!\n");
    }
		//robot();
    // close(s);
    // sleep(5);
    return 0;

  } else {
    fprintf (stderr, "[BT] - Failed to connect to server...\n");
    sleep(2);
    return -1;
    //exit (EXIT_FAILURE);
  }

  // close(s);
  //return 0;
}

void send_pos () {
  char string[58];
  printf ("[BT] - I'm sending my position...\n");
  string[0] = msgId % 0xFF;
  string[1] = msgId >> 8;
  msgId++;
  string[2] = TEAM_ID;
  string[3] = 0xFF;
  string[4] = MSG_POSITION;
  string[5] = (int)(my_pos.x-P)/5;          /* x */
  string[6] = 0x00;
  string[7] = (int)(my_pos.y-P)/5;		/* y */
  string[8]= 0x00;
  write(bt_sock, string, 9);
}

void send_obs () {
  char string[58];
  printf ("[BT] - I'm releasing my obstacle...\n");
  int x = (int)my_pos.x, y = (int)my_pos.y; int dir = (int)my_pos.dir;

  int obs_x = x - (TAIL - SIDEY_OBSTACLE/2)*sin(dir);
  int obs_y = y - (TAIL - SIDEY_OBSTACLE/2)*cos(dir);

  string[0] = msgId % 0xFF;
  string[1] = msgId >> 8;
  msgId++;
  string[2] = TEAM_ID;
  string[3] = 0xFF;
  string[4] = MSG_OBSTACLE;
  string[5] = 0x00; //obstacle dropped
  string[6] = (obs_x-P)/5;
  string[7] = 0x00;
  string[8] = (obs_y-P)/5;
  string[9] = 0x00;
  write(bt_sock, string, 10);
}


void send_map(){
  char string[58];
  printf ("[BT] - I'm sending my map...\n");
  int row_ext, col_ext, row_int, col_int, k;
  int average[MAP_SQUARE][MAP_SQUARE]={{0}};
  int average_square=0;
  int flag=0;

  for (row_ext = P+H+P; row_ext > 0; row_ext-=5) {
    //printf("row_ext=%d\n", row_ext);
    for (col_ext = 0; col_ext < P+L+P; col_ext+=5) {
      //printf("col_ext=%d\n", col_ext);
      for(row_int=row_ext-MAP_SQUARE; row_int<row_ext; row_int++){
        //printf("row_int=%d\n", row_int);
        for(col_int=col_ext; col_int < col_ext+MAP_SQUARE; col_int++){
          //printf("col_int=%d\n", col_int);
            if((mat[row_int][col_int] & 0b11)==0b01){
              if((((mat[row_int][col_int] >> 2) & 0b11))==HIT){
                average[row_int % MAP_SQUARE][col_int % MAP_SQUARE]=8;
                flag=1;
              } else {
                average[row_int % MAP_SQUARE][col_int % MAP_SQUARE]=-8;
                flag=1;
              }
            } else {
              for(k=0; k<8; k++){
                //printf("a\n");
                if((((mat[row_int][col_int] >> (2*k)) & 0b11))==HIT){
                  average[row_int% MAP_SQUARE][col_int% MAP_SQUARE]+=1;
                  flag=1;
                  //printf("FULL HERE\n");
                } else if((((mat[row_int][col_int] >> (2*k)) & 0b11))==MISS){
                  //average[row_int% MAP_SQUARE][col_int% MAP_SQUARE]-=1;
                  //flag=1;
                }
              }
            }
                //printf("average=%d\n", average[row_int][col_int]);
                if(flag==0){
                  average_square=0;
                }
                else if(average[row_int% MAP_SQUARE][col_int% MAP_SQUARE]>=0){
                  average_square+=1;
                  } else {
                    average_square-=1;
                  }
                average[row_int% MAP_SQUARE][col_int% MAP_SQUARE]=0;
                flag=0;
            }
        }
      string[0] = msgId % 0xFF;
      string[1] = msgId >> 8;
      msgId++;
			string[2] = TEAM_ID;
	    string[3] = 0xFF;
	    string[4] = MSG_MAPDATA;
    	string[5] = col_ext/5;          /* x */
			string[6] = 0x00;
    	string[7] = row_ext/5;		     /* y */
			string[8]= 0x00;
      //printf("average of the square is: %d\n", average_square);
      if(average_square>0 && average_square<=25){
        //printf("average_square=%d\n", average_square);
        // printf("@ ");
        string[9] = 255;
  			string[10] = 255;
  			string[11] = 255;
      } else if (average_square<0) {
        // printf("_ ");
        string[9] = 0;
  			string[10] = 0;
  			string[11] = 0;
      } else {
        // printf("? ");
        string[9]= 0;
  			string[10]= 255;
  			string[11]= 0;
      }
      average_square=0;
      write(bt_sock, string, 12);
      }
        // printf("\n");
    }

  //printf("finish\n");
  printf("[BT] - Done sending map");
  string[0] = msgId % 0xFF;
  string[1] = msgId >> 8;
  msgId++;
	string[2] = TEAM_ID;
	string[3] = 0xFF;
	string[4] = MSG_MAPDONE;
	write(bt_sock, string, 5);
  return;
}

void wait_stop() {
  char type;
  char string[58];
  printf("[BT] - I'm waiting for the stop message");
  while(1){
    //Wait for stop message
    read_from_server (bt_sock, string, 58);
      type = string[4];
    if (type ==MSG_STOP){
      return;
    }
  }
}
