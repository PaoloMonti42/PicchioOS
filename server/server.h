#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define MAXMSG              58
#define MAXNAMESIZE         15
#define MAXTEAM             14

#define RBT_MISS            0
#define RBT_EV3_BT          1
#define RBT_EV3_IN          2

#define MSG_ACK				0
#define MSG_START		    1
#define MSG_STOP   			2
#define MSG_KICK    		3
#define MSG_POSITION 		4
#define MSG_MAPDATA 		5
#define MSG_MAPDONE 		6
#define MSG_OBSTACLE 		7
#define MSG_CUSTOM 			8

struct team {
    int sock;
    char name[MAXNAMESIZE+1];
    union {
        bdaddr_t bt;
        struct in_addr in;
    } address;
    char robotType;
    char active;
    char connected;
    char kicked;
    char ended;
    int idServMsg;
    unsigned char ally;
    unsigned char side;
    unsigned char role;
};

#define GAM_CONNECTING      0
#define GAM_RUNNING         1
#define GAM_TEAM_SELECT     2
#define GAM_INIT            3
#define GAM_STOP            4
#define GAM_END             5

struct game {
    struct team teams [MAXTEAM];
    int nbTeams;
    pthread_mutex_t lock;
    char state;
    unsigned char leaders[2];
    unsigned char ballStatus[2];
};

extern struct game game;

extern struct observerMethods GUI;

extern FILE *logFile;
extern char debugLvl;

void sendKick (int teamID);

#define RESET "\033[0m"

#endif
