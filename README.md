# EURECOM OS Contest Server
# Main website: http://soc.eurecom.fr/OS/projects_fall2017.html


A simple server that enables bluetooth communication between a set EV3 Lego Mindstorm.

It is first intended to be used in the OS course of EURECOM but part or all of it can be freely reused by anyone.

## Setup

### For EV3

Follow the steps from [http://www.ev3dev.org/docs/getting-started/](http://www.ev3dev.org/docs/getting-started/) to download the ev3dev kernel image and flash it on the SD card.

If you do not have blueman, set it up as described: [http://www.ev3dev.org/docs/tutorials/connecting-to-the-internet-via-bluetooth/](http://www.ev3dev.org/docs/tutorials/connecting-to-the-internet-via-bluetooth/).

Turn on the EV3. Unpair the device on the server. Pair again from the server: enter PIN code and reenter it on EV3. Connect by SSH on the EV3. On both the server and the EV3 the command
```
$ hcitool scan
```

should show the connection.
### For the server

You can compile the server using the provided Makefile. If you run into an error stating that `bluetooth/bluetooth.h` was not found,
you will need to install the `libbluetooth-dev` package (on Debian-like UNIX).

Run the server with an appropriate team file. Each line of the team file - provided as input to the server - should be formatted as:
```
[TYPE] [ADDR] [NAME]
```
where:
* `[TYPE]` : `1` for bluetooth, `2` for network
* `[ADDR]` : in the form `aa:bb:cc:dd:ee:ff` is the bluetooth address if type is 1, or in the form `aaa.bbb.ccc.ddd` for IP address if type is 2
* `[NAME]` : is the name of the team

You can then run the server as
```
$ ./server teams
```
or
```
$ ./server -o log teams
```
if you wish to log the session.

## Protocol

To communicate between each other, EV3 must comply with the specified protocol. Invalid messages will be discarded by the
server so robots can always consider that received messages are well formatted (except for `CUSTOM` messages, whose structure is
not pre-determined).

Each message consists of a header and a body. Note that all numbers, unless explicitly stated otherwise, are unsigned integers whose formats are **little-endian**.

The protocol may evolve according to needs and proposals.

### Header

The header is 5-bytes long:

```
   0      1      2      3      4
+------+------+------+------+------+
|     ID      | src  | dst  | type |
+------+------+------+------+------+
```

Fields description:
* `ID` is a 2-byte number identifying the message (kind of like a sequence number). It is used when acknowledging messages.
* `src` is a 1-byte number identifying the team who sent the message (it is unique for the whole contest).
* `dst` is a 1-byte number identifying the team who should receive the message (it is unique for the whole contest).
* `type` is a 1-byte code that identifies the kind of message that is sent.

### Body

#### ACK

ACK messages are used to acknowledge the reception of messages. They are 8-byte long:
```
    0       1       2       3       4       5       6       7
+-------+-------+-------+-------+-------+-------+-------+-------+
|      ID       |  src  |  dst  |   0   |    ID ack     | state |
+-------+-------+-------+-------+-------+-------+-------+-------+
```

Fields description:
* `ID ack` is the ID of the message that is acknowledged
* `state` is a status code. `0 -> OK`, `1 -> error`. Other status codes may be used for acknowledging custom messages.

Messages sent by the server should not be acknowledged.

#### START

START messages can only be used by the server. One is sent to each team when the game starts. If the robot disconnects and reconnects
during the game, another START message will be sent to it right after it connects to the server. They are 8-byte long:
```
    0       1       2       3       4       5       6       7
+-------+-------+-------+-------+-------
|      ID       |  src  |  dst  |   1  | 
+-------+-------+-------+-------+-------
```



#### STOP

STOP messages are sent by server to every robot when the game ends. They are 5-bytes long:
```
    0       1       2       3       4
+-------+-------+-------+-------+-------+
|      ID       |  src  |  dst  |   2   |
+-------+-------+-------+-------+-------+
```
#### KICK

KICK messages can only be sent by the server. This message is used to advertise that a robot got kicked out of the game. It is sent
to every robot in the game. The message is 6-bytes long:
```
    0       1       2       3       4       5
+-------+-------+-------+-------+-------+-------+
|      ID       |  src  |  dst  |   3   |  id   |
+-------+-------+-------+-------+-------+-------+
```

Fields description:
* `id` is the id of the robot that was kicked.

#### POSITION

POSITION messages must be sent by robots every 2 seconds. This message is used to advertise the expected position of the robot.
The message is 9-bytes long:
```
    0       1       2       3       4       5       6       7       8
+-------+-------+-------+-------+-------+-------+-------+-------+-------+
|      ID       |  src  |  0XFF |   4   |       x       |       y       |
+-------+-------+-------+-------+-------+-------+-------+-------+-------+
```

Fields description:
* `x` is the x coordinate of the robot. This field is a **signed** 16-bits little-endian integer.
* `y` is the y coordinate of the robot. This field is a **signed** 16-bits little-endian integer.

#### MAPDATA

After the entire map has been generated, the robot sends the server each 5x5 cm grid one pixel at a time.
```
    0       1       2       3       4       5       6       7       8       9      10      11
+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
|      ID       |  src  |  0xFF |   5   |       x       |       y       |   R      G        B
+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
```

Fields description:

* `x` is the x coordinate of the pixel. This field is a **signed** 16-bits little-endian integer.
* `y` is the y coordinate of the pixel. This field is a **signed** 16-bits little-endian integer.
* 'R', 'G', and 'B' are the red, green, blue values of the pixel respectively, valued from 0 to 255. 

#### MAPDONE

These messages signal to the server that their map is finished.
```
    0       1       2       3       4
+-------+-------+-------+-------+-------+
|      ID       |  src  | 0xFF |   6  |
+-------+-------+-------+-------+-------+

```


#### OBSTACLE

OBSTACLE messages must be sent when a robot picks up or drop an obstacle. 
```
    0       1       2       3       4       5       6       7       8       9
+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
|      ID       |  src  |  0XFF |   7   |  act  |       x       |       y       |
+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
```

Fields description:

* `act` is `0` if the robot dropped obstacle or `1` if it picked it up.
* `x` is the x coordinate of the dropped obstacle. This field is a **signed** 16-bits little-endian integer.
* `y` is the y coordinate of the dropped obstacle. This field is a **signed** 16-bits little-endian integer.



#### CUSTOM

CUSTOM messages may be used to increment the protocol if teams wish to add their own custom messages. CUSTOM messages can not have
a size greater than 58 bytes (header included).
```
    0       1       2       3       4        5      . . .
+-------+-------+-------+-------+-------+-----------------
|      ID       |  src  |  dst  |   4   |   payload . . .
+-------+-------+-------+-------+-------+-----------------
```

