#include <stdio.h>
#include <stdlib.h>
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
#define millisleep( msec ) usleep(( msec ) * 1000 )

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

void turn_motor_time(int sn, int speed, int time, int ramp_up, int ramp_down) {
	set_tacho_stop_action_inx( sn, TACHO_COAST );
	set_tacho_speed_sp( sn, speed );
	set_tacho_time_sp( sn, time );
	set_tacho_ramp_up_sp( sn, ramp_up );
	set_tacho_ramp_down_sp( sn, ramp_down );
	set_tacho_command_inx( sn, TACHO_RUN_TIMED );
}

void turn_motor_deg(int sn, int speed, int deg) {
	int compensation = 1;
	set_tacho_stop_action_inx( sn, TACHO_COAST );
	set_tacho_speed_sp( sn, speed * compensation );
	set_tacho_ramp_up_sp( sn, 0 );
	set_tacho_ramp_down_sp( sn, 0 );
	set_tacho_position_sp( sn, deg );
	set_tacho_command_inx( sn, TACHO_RUN_TO_REL_POS );
}

void turn_motor_to_pos(int sn, int speed, int pos) {
	int compensation = 1;
	set_tacho_stop_action_inx( sn, TACHO_COAST );
	set_tacho_speed_sp( sn, speed * compensation );
	set_tacho_ramp_up_sp( sn, 0 );
	set_tacho_ramp_down_sp( sn, 0 );
	set_tacho_position_sp( sn, pos );
	set_tacho_command_inx( sn, TACHO_RUN_TO_ABS_POS );
}

void wait_motor_stop(int sn) {
	int state;
	do {
		get_tacho_state_flags( sn, &state );
	} while ( state );
}

int main( void )
{
	int i;
	uint8_t sn;
	FLAGS_T state;
	uint8_t sn_touch;
	uint8_t sn_color;
	uint8_t sn_compass;
	uint8_t sn_sonar;
	uint8_t sn_mag;
	char s[ 256 ];
	int val;
	float value;
	uint32_t n, ii;
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
	//Run motors in order from port A to D
	int port=65;
	for (port=65; port<69; port++){
		if ( ev3_search_tacho_plugged_in(port,0, &sn, 0 )) {

			int max_speed;
			get_tacho_max_speed( sn, &max_speed );
			printf("  max speed = %d\n", max_speed );

			printf("Setting to position 0...\n");
			turn_motor_to_pos(sn, max_speed * 1/3, 0);
	    wait_motor_stop(sn);
			millisleep(2000);

			printf("Setting to position 90...\n");
			turn_motor_to_pos(sn, max_speed * 1/3, 90);
	    wait_motor_stop(sn);
			millisleep(2000);

	    printf("Turning clockwise...\n");
			turn_motor_time(sn, max_speed * 4/5, 3000, 1000, 1000);
	    wait_motor_stop(sn);
			millisleep(2000);

			printf("Turning counterclockwise...\n");
			turn_motor_time(sn, -max_speed * 4/5, 3000, 1000, 1000);
			wait_motor_stop(sn);
			millisleep(2000);

			printf("Degree turning clockwise...\n");
			turn_motor_deg(sn, max_speed * 4/5, 360);
			wait_motor_stop(sn);
			millisleep(2000);

	    printf("Degree turning counterclockwise...\n");
			turn_motor_deg(sn, max_speed * 4/5, -360);
			wait_motor_stop(sn);
			millisleep(2000);


		} else {
			printf( "LEGO_EV3_M_MOTOR 1 is NOT found\n" );
		}
	}

	ev3_uninit();
	printf( "*** ( PICCHIO ) Bye! ***\n" );

	return ( 0 );
}
