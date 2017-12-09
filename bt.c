#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#define SERV_ADDR   "40:e2:30:50:e9:4c"     /* Whatever the address of the server is */
#define TEAM_ID     1                       /* Your team ID */

#define MSG_ACK     0
#define MSG_START    1
#define MSG_STOP   2
#define MSG_KICK    3
#define MSG_POSITION 4
#define MSG_MAPDATA 	5
#define MSG_MAPDONE 6


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
    fprintf (stderr, "Server unexpectedly closed connection...\n");
    close (bt_sock);
    exit (EXIT_FAILURE);
  }

  printf ("[DEBUG] received %d bytes\n", bytes_read);

  return bytes_read;
}

void robot () {
  char string[58];
  char type;
  printf ("I'm navigating...\n");

	srand(time(NULL));
    /* Send 3 POSITION messages, a BALL message, 1 position message, then a NEXT message */
  int i, j;
  for (i=0; i<3; i++){
    string[0] = msgId % 0xFF;
    string[0] = msgId >> 8;
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

  printf ("I'm sending my map...\n");
	/* MAP data is in the form MAPDATA | X  X |Y  Y | R | G | B */

	for (i=0; i<10; i++){
		for (j=0; j<10; j++){
      string[0] = msgId % 0xFF;
      string[0] = msgId >> 8;
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
	printf("Done sending map");
  string[0] = msgId % 0xFF;
  string[0] = msgId >> 8;
  msgId++;
	string[2] = TEAM_ID;
	string[3] = 0xFF;
	string[4] = MSG_MAPDONE;
	write(bt_sock, string, 5);

	printf("I'm waiting for the stop message");
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
      printf ("Received start message!\n");


    }
		//robot();
    // close(s);
    // sleep(5);
    return 0;

  } else {
    fprintf (stderr, "Failed to connect to server...\n");
    sleep(2);
    return -1;
    //exit (EXIT_FAILURE);
  }

  // close(s);
  //return 0;
}
