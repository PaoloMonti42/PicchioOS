#include "bt.c"

int read_from_server (int sock, char *buffer, size_t maxSize);

void debug (const char *fmt, ...);

void robot ();

int bt_init();

void send_pos();

void send_map();
