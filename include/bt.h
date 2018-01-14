#include "bt.c"

int read_from_server (int sock, char *buffer, size_t maxSize);

int bt_init();

void send_pos();

void send_obs ();

void send_map();

void wait_stop();

void send_matrix(int matrix[H_AVG][L_AVG]);
