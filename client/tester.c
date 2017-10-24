#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ev3.h"
#include "ev3_port.h"
#include "ev3_tacho.h"
#include "ev3_sensor.h"

#include "motor_lib.h"
#define millisleep( msec ) usleep(( msec ) * 1000 )

const char const *color[] = { "?", "BLACK", "BLUE", "GREEN", "YELLOW", "RED", "WHITE", "BROWN" };
#define COLOR_COUNT  (( int )( sizeof( color ) / sizeof( color[ 0 ])))

int main( void )
{
	int i;
	char s[256];
	uint8_t motors[2];

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

	while ( ev3_tacho_init() < 1 ) millisleep( 1000 );
	printf( "*** ( PICCHIO ) Hello! ***\n" );

	printf( "Found tacho motors:\n" );
	for ( i = 0; i < DESC_LIMIT; i++ ) {
		if ( ev3_tacho[ i ].type_inx != TACHO_TYPE__NONE_ ) {
			printf( "  type = %s\n", ev3_tacho_type( ev3_tacho[ i ].type_inx ));
			printf( "  port = %s\n", ev3_tacho_port_name( i, s ));
			printf("  port = %d %d\n", ev3_tacho_desc_port(i), ev3_tacho_desc_extport(i));
		}
	}

	if ( !ev3_search_tacho_plugged_in(65,0, &motors[0], 0 )) {
		printf( "Motor SX not found!\n" );
	}
	if ( !ev3_search_tacho_plugged_in(68,0, &motors[1], 0 )) {
		printf( "Motor DX not found!\n" );
	}

	int max_speed;
	get_tacho_max_speed( motors[0], &max_speed );
	printf("Motor max speed = %d\n", max_speed );


	// printf("Going forwards...\n");
  // go_forwards(motors, 2000, max_speed/2);
  // wait_motor_stop(motors[0]);
	// wait_motor_stop(motors[1]);

  // printf("Going backwards...\n");
	// go_forwards(motors, 2000, -max_speed/2);
	// wait_motor_stop(motors[0]);
	// wait_motor_stop(motors[1]);

	// printf("Turning right...\n");
	// turn_right(motors, max_speed/2);
	// wait_motor_stop(motors[0]);
	// wait_motor_stop(motors[1]);
	// millisleep(2000);

	  for (i = 0; i < 4; i++) {
			printf("Going forwards...\n");
		  go_forwards(motors, 1000, max_speed/2);
		  wait_motor_stop(motors[0]);
			wait_motor_stop(motors[1]);
		  millisleep(1000);
			printf("Turning left...\n");
			turn_left(motors, max_speed/2);
			wait_motor_stop(motors[0]);
			wait_motor_stop(motors[1]);
			millisleep(1000);
  }

	ev3_uninit();
	printf( "*** ( PICCHIO ) Bye! ***\n" );
	return ( 0 );
}
