all:
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -Wno-unused-variable -c tester.c -o tester.o
	gcc tester.o -Wall -lm -lev3dev-c -lpthread -o tester
	#gcc -std=gnu99 -W -Wall -c robotclient.c -o robotclient.o
	#gcc robotclient.o -Wall -lm -lev3dev-c -lbluetooth -o robotclient
	#gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c i2c.c -o i2c.o
	#gcc i2c.o -Wall -lm -lev3dev-c -o i2c

i2c:
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c i2c.c -o i2c.o
	gcc i2c.o -Wall -lm -lev3dev-c -o i2c
	./i2c

client:
	gcc -std=gnu99 -W -Wall -c robotclient.c -o robotclient.o
	gcc robotclient.o -Wall -lm -lev3dev-c -lbluetooth -o robotclient
	./robotclient

run:
	./tester
