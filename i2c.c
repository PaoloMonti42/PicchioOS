
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ev3.h"
#include "ev3_port.h"
#include "ev3_tacho.h"
#include "ev3_sensor.h"
// WIN32 /////////////////////////////////////////
#ifdef __WIN32__

#include <windows.h>

// UNIX //////////////////////////////////////////
#else

#include <unistd.h>
#define Sleep( msec ) usleep(( msec ) * 1000 )

//////////////////////////////////////////////////
#endif
const char const *color[] = { "?", "BLACK", "BLUE", "GREEN", "YELLOW", "RED", "WHITE", "BROWN" };
#define COLOR_COUNT  (( int )( sizeof( color ) / sizeof( color[ 0 ])))

static bool _check_pressed( uint8_t sn )
{
	int val;

	if ( sn == SENSOR__NONE_ ) {
		return ( ev3_read_keys(( uint8_t *) &val ) && ( val & EV3_KEY_UP ));
	}
	return ( get_sensor_value( 0, sn, &val ) && ( val != 0 ));
}


int main( void )
{
	
#ifndef __ARM_ARCH_4T__
	/* Disable auto-detection of the brick (you have to set the correct address below) */
	ev3_brick_addr = "192.168.0.204";

#endif
	if ( ev3_init() == -1 ) return ( 1 );

#ifndef __ARM_ARCH_4T__
	printf( "The EV3 brick auto-detection is DISABLED,\nwaiting %s online with plugged tacho...\n", ev3_brick_addr );

#else
	printf( "Waiting tacho is plugged...\n" );

#endif
	while ( ev3_tacho_init() < 1 ) Sleep( 1000 );

	printf( "*** ( EV3 ) Hello! ***\n" );
	char raw[100];


	int count;
	//Get 20 compass measurements, 1 second between each
	for (count=0; count<20; count++){
	//Get measurement with i2cdump. Be careful with the i2ctools
	FILE *fp = popen("sudo i2cdump -y -r 0x42-0x44 3 0x01 c","r");
	while (fgets(raw, sizeof(raw)-1, fp)!=NULL){
	}
	//Convert result to find only the measurement
	char b1[2];
	char b2[2];
	char b3[2];	
//	printf("RAW data:%s", raw);
	char* tmp = strtok(raw, " ");
	tmp = strtok(NULL, " ");
	strcpy(b1,tmp);
	tmp = strtok(NULL, " ");
	strcpy(b2,tmp);
	tmp = strtok(NULL, " ");
	strcpy(b3,tmp);

	char num[6];
	strcpy(num,b1);
	strcat(num,b2);
	strcat(num,b3);

//	Convert hex string into integer

	int number= (int) strtol(num, NULL, 16);

	printf("number: %d \n", number);
	
	pclose(fp);
	Sleep(1000);
	}	
	ev3_uninit();
	printf( "*** ( EV3 ) Bye! ***\n" );

	return ( 0 );
}
