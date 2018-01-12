all:
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c src/main.c -o main.o
	gcc main.o -Wall -lm -lev3dev-c -lpthread -lbluetooth -o main
	rm main.o

cross:
	arm-linux-gnueabi-gcc -I./ev3dev-c/source/ev3 -I./include -I./src -O2 -std=gnu99 -W -Wall -Wno-comment -c src/main.c -o main.o
	arm-linux-gnueabi-gcc main.o -Wall -L./include -lm -lev3dev-c -lpthread -lbluetooth -o main
	rm main.o

run:
	./main
