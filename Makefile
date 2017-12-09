all:
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -Wno-unused-variable -Wno-unused-parameter -c tester.c -o tester.o
	gcc tester.o -Wall -lm -lev3dev-c -lpthread -o tester
	#gcc -std=gnu99 -W -Wall -c robotclient.c -o robotclient.o
	#gcc robotclient.o -Wall -lm -lev3dev-c -lbluetooth -o robotclient
	#gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c i2c.c -o i2c.o
	#gcc i2c.o -Wall -lm -lev3dev-c -o i2c

cross:
	arm-linux-gnueabi-gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -Wno-unused-variable -Wno-unused-parameter -c tester.c -o tester.o
	arm-linux-gnueabi-gcc tester.o -Wall -lm -lev3dev-c -lpthread -o tester
	#arm-linux-gnueabi-gcc -std=gnu99 -W -Wall -c robotclient.c -o robotclient.o
	#arm-linux-gnueabi-gcc robotclient.o -Wall -lm -lev3dev-c -lbluetooth -o robotclient
	#arm-linux-gnueabi-gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c i2c.c -o i2c.o
	#arm-linux-gnueabi-gcc i2c.o -Wall -lm -lev3dev-c -o i2c

i2c:
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c i2c.c -o i2c.o
	gcc i2c.o -Wall -lm -lev3dev-c -o i2c
	./i2c

client:
	gcc -std=gnu99 -W -Wall -c robotclient.c -o robotclient.o
	gcc robotclient.o -Wall -lm -lev3dev-c -lbluetooth -o robotclient
	./robotclient

run:
	#export LD_LIBRARY_PATH=~/client/ev3dev-c/lib
	./tester
