# Picchio - cartography project for the OS class, group 1, Eurecom, 2017-2018
# [Website](https://eurecomos.000webhostapp.com/)

## How to compile and install the binaries

Please notice that the following guide takes for granted that you're running Linux. The source code for our project is hosted at https://github.com/PaoloMonti42/PicchioOS.

In order to download it enter the path where you would like to place the folder containing the code and run:

git clone https://github.com/PaoloMonti42/PicchioOS.git

Then you may either download the whole source code inside the robot and compile it internally or use docker on your machine in order to cross compile it and then download the code inside the robot.

First of all make sure that your computer is connected to the robot via either wi-fi or bluetooth connection. Then, if you want to compile the source code internally, from the chosen installation path type the following commands on the terminal:

scp -r ./PicchioOS robot@ROBOT_ADDRESS:~

ssh robot@ROBOT_ADDRESS

(insert ev3dev password)

cd PicchioOS

make

make run

If you choose instead to cross compile run the following commands, again assuming that you start from the chosen installation path:

cd PicchioOS

docker run -rm -it -h ev3 -v $(echo $PWD):/src -w /src ev3cc /bin/bash

cd ev3dev-c/source/ev3/

make

sudo make install

make shared

sudo make shared-install

sudo apt-get update

sudo apt-get install libbluetooth-dev

cd ../../../

make cross

exit

scp Makefile robot@ROBOT_ADDRESS:~

scp tester robot@ROBOT_ADDRESS:~

ssh robot@ROBOT_ADDRESS

(insert ev3dev password)

At this point you must make sure that the tester file is located inside the robot in the same directory where libev3dev-c is placed. If this is not the case make sure to respect this requirement. To start the robot enter the folder where the tester file, the ev3dev-c folder and the Makefile are located and run:

make run

Notice that ROBOT_ADDRESS can be substituted with ev3dev.local when connecting via bluetooth.

## Specifications

* Hardware: Lego Mindstorms ev3 plus sensors and lego blocks;

* Goal: the robot must realize the map of an arena without bumping into non movable obstacles or the walls of the arena.

* The robot must be contained in a cube of 35 cm maximum on each dimension at start-up;

* Robots can use up to four sensors and up to four engines. You are free to use the sensors you want to among the following ones: touch sensor, light sensor, color sensor, ultrasonic sensor (i.e., distance sensor), compass sensor, gyroscope sensor, magnetic sensor;

* The robot must have a flag on which the number of your group is clearly readable from at least two sides of the robot. The flag dimensions are at most 10x10cm. The flag may also contain a logo, a drawing and the name of the robot;

* The communication protocol between robots (bluetooth) will be given (you don't have to specify it). Basically, robots communicate to a server (and possibly other robots);

* A robot may change its shape during game phases (by deploying elements or grabbing them), but it should fit in a 35cm cube at the start of the game;

* Your robot is allowed to lose/throw parts on purpose during the game. Destructive weapons are NOT allowed;

* It is forbidden to send orders - remotely or not - to your robot while it is playing, apart from the BT communication with the server. The robot must be fully autonomous as soon as the game starts and until the game ends. Trying to hijack BT messages or performing any other kind of attacks is not allowed;

## Architecture

We used the following devices and sensors:

* 4 motors (connected to ports A, B, C, D)
* Gyroscope (connected to port 3)
* Ultrasonic sensor (connected to port 2)
* Color and light sensor (connected to port 4)
* Touch sensor (connected to port 1)

The number of motors and sensors available was limited by the number of ports and by the architecture itself, as adding too many components would yield serious issues in terms of mobility, speed and precision. For instance, we noticed that if the robot was particularly heavy the wheel's axel would be deformed as a consequence, yielding problems such as impossibility to maintain a certain direction precisely or causing the robot to turn slightly left or right when the motors were started. Moreover, we noticed that the motors would be more imprecise when it came to turn left or right if the weight over the wheels wouldn't be evenly distributed. In order to solve these problem we tried several architectures until we achieved a satisfying one.

The adopted architecture allows to distribute the weight in a more even way between the two wheels and the metal ball, as they are evenly distantiated with respect to the center of mass of the ev3dev board and as the distance between them is approximately the same. Also, stability is greatly improved by placing the ev3dev board in parallel with respect to the axel. This choice also allowed us to use all the available ports and to connect an additional motor used to turn the ultrasonic sensor, placed in the front. Lastly, the color sensor was mounted on top of the ultrasonic sensor while the gyroscope was mounted on top of the ev3dev board, close to the brick itself since we noticed that this reduced vibrations and increased precision.

#### Motors

Two motors where used to turn the wheels, which are placed in the front.

During the early testing phases we realized that the two motors would mostly start asynchronously, hence causing the robot to turn slightly left or right in a random fashion each time it would start moving. This was due to the axis distortion caused by the weight of the EV3 board and of the other robot parts mounted on top. Placing the wheels in the front and the metal ball in the back allowed us to greatly mitigate this problem, as they now occupy a central position and hence unwanted torques are minimized when turning, resulting in a higher precision.

The third motor was used to turn the axle connected to two LEGO mechanical arms mounted in the back. Turning the axle would result in either
lifting or lowering the mechanical arms. This mechanism was exploited in order to hold and release the non movable object within the arena.

The fourth motor was placed in the front, on top of the ultrasonic sensor, in order to allow its rotation. This apparently cumbersome choice was adopted during later stages of the project as we came to the conclusion that having the robot turning left and right in order to scan its surroundings was too risky, as it would often bump into non moveable objects or against the walls of the arena by doing so.

#### Gyroscope

The gyroscope was mounted on top of the robot. We used the gyroscope for recalibration purposes and in order to make the robot turn to a certain angle. For instance, we used the gyroscope to associate a certain angle to each ultrasonic sensor reading.

#### Ultrasonic sensor

The ultrasonic sensor was used to detect movable and non movable obstacles, borders and in order to generate the map. Although the sensor is in principle capable of measuring distances up to 250 cm, after careful testing we realized that, past 15 cm, the sensor is not reliable any longer. This issue was most likely due to echoes which would confound the sensor at distances larger than 15 cm.

We decided to make the robot stop when it is ca. 7 cm away from an obstacle. After this, thanks to an additional routine, the robot scans the surrounding area and then moves backward in order to make sure that it won't bump into additional walls or obstacles which may be around. When reading the variable referring to the distance detected by the sensor, we discarded any value corresponding to a distance larger than 15 cm for the aforementioned reason.

#### Color and light sensor

This sensor, placed in the front of the robot, was used to detect movable objects and acknowledge their presence, since the project assignment specified that moveable objects are always red balls. The output of this sensor is read every time the ultrasonic sensor detects an obstacle in front of the robot in order to distinguish between a non moveable object and a moveable one.

#### Touch sensor

Mounted on top of the light and color sensor, the touch sensor is used as a failsafe mechanism to realize whether the robot has collided against a non movable obstacle. Since the robot is turning his "head" left and right while moving it may happen that it is not fast enough to detect an obstacle in front of him before bumping into it. In order to augment the sensitivity of the touch sensor a crossbar inserted in a wheel was placed in the socket of the sensor.

## Credits
Project by group 1 -  Martina Fogliato, Valerio Lanieri, Paolo Monti, Luca Rossi.
