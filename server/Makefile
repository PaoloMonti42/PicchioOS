CC 		= gcc
CFLAGS 		= -Wall -Werror -pedantic -std=c89 -g
CFLAGS 		= -Wall -Werror -DGRAPHICS -pedantic -std=gnu89 -g $(shell sdl2-config --cflags)
LFLAGS 		= -lpng -lbluetooth -lcurses -lpthread $(shell sdl2-config --libs) -lGL -lGLEW -lGLU -lm
SRC_FILES 	= $(wildcard *.c)
OBJ_FILES	= $(SRC_FILES:%.c=%.o)
EXEC		= server

all: $(EXEC)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

$(EXEC): $(OBJ_FILES)
	$(CC) -o $@ $^ $(LFLAGS)

.PHONY: clean ultraclean

clean:
	rm -f $(OBJ_FILES)

ultraclean: clean
	rm -f $(EXEC)
