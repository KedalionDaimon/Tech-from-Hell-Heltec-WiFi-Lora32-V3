# Heltec WiFi LoRa32 V3 / Serial to LoRa bridge / LoRa to Serial bridge

## Breaking compatibility and clobbering transmissions

These are two example sketches (once, SF12@250kHz with echo, once, SF10@250kHz without echo, pure LoRa point-to-point connections, NOT LoRaWAN) of how to program a "RS232 to LoRa bridge" (and, simultaneously, "LoRa to RS232 bridge") on the Heltec WiFi LoRa32 V3 board.

These are relatively cheap boards for LoRa (Long Range wireless) connections, presently flooding the market, but not equipped with adequate warnings, and with obtuse and intransparent examples in my view that do not fairly warn of the issues connected to them, in particular, incompatibility with the Arduino ecosystem around LoRa libraries like LoRa.h, instead demanding that you use LoRaWan_APP.h (due to a change of the LoRa controller from SX1276/SX1278 to SX1262).

If you leave such boards a negative review on Amazon - as they break many existing projects - the seller will come at you with "breach of Community Standards", which some mindless Amazon drone spouted at me simply for leaving a mediocre review. - Though, upon further questioning, Amazon could not state WHICH rule I have allegedly "broken". Boo, Amazon! Get your act together! Leaving a not-so-positive, but fair review is not a crime! :)

The problem with using LoRaWan_APP.h is that this is not just "a slight change of library calls", but an entire CHANGE OF THE PROGRAMMING PARADIGM from "polling" to "interrupt driven" - with all of the racing conditions arising therefrom.

Particularly unwise is the library call "Radio.Send()", used to send a LoRa packet. This roughly corresponds to, in LoRa.h, LoRa.endPacket(true). The default LoRa.endPacket() call is going to send a packet, BUT will "block" the board from doing whatsoever ere the packet transmission is completed. This is the "newbie-friendly" default. The more dangerous call - LoRa.endPacket(true) - will return immediately to further processing, just like the new Radio.Send() in LoRaWan_APP.h, i.e. it is "non-blocking". HOWEVER: should you call Radio.Send AGAIN before the previous Radio.Send() has terminated, then the new Radio.Send() will simply interrupt/clobber/destroy the previous Radio.Send() call - there is no internal queuing. This is a classical mistake of newbies on Heltec WiFi LoRa32 V3: they will call Radio.Send() repeatedly, NOT waiting for it to transmit, and then they will wonder, "why nothing arrived": well, because you "voided" each and every previous Radio.Send() call by re-transmitting too fast.

The fact that the Heltec WiFi LoRa32 V3 is not co-operating with LoRa.h and is changing the paradigm, breaking a lot of projects, and the sheer "trap" that Radio.Send() constitutes compared to "established practice" are in my view grave disadvantages - that need to be balanced with the excellent price of these boards.




## Remote control

If you use the SF10@250kHz Arduino sketch (this, as there must be NO echo - to avoid cycling back to the shell), then you can remote-control e.g. a Linux-computer by another like this:

Server:

stdbuf -e0 -o0 cat /dev/ttyUSB0 | sh >/dev/ttyUSB0 2>&1

or

stdbuf -e0 -o0 socat EXEC:/usr/bin/sh /dev/ttyUSB0,echo=1,b9600,crtscts=0,cstopb=0,parenb=0,ixon=1,igncr=1,onlcr=1,ctty

Client:

sudo picocom --baud 9600 --parity n --databits 8 --stopbits 1 --flow n --omap igncr --imap lfcrlf --emap igncr,lfcrlf,delbs --echo /dev/ttyUSB0

Use Ctrl-J for enter.

You will not see a prompt, but the connection will work. Run as root, if necessary.




## Excursus I: Soldering the accursed pins

The boards often arrive with their pins loose, you being expected to solder them on. But the screen cable passes basically to within less than a millimeter to the place you are supposed to solder - with a high risk of burning the cable during soldering.

Solution A: solder them in reverse. Just point the pins up. (If you do not mind and can handle issues with the enclosure.)

Solution B: unscrew the four screws of the screen and pull the screen farther away during soldering.




## Excursus II: Handling the accursed error, "Please provide a correct license!"

Should you, upon flashing in the Arduino IDE, enable the option (clearly disadvised!)

Tools --> Erase All Flash Before Sketch Upload

then you will not be able to use your LoRa-Board until you fix it, getting over the serial monitor repeatedly nothing but the message:

Please provide a correct license! For more information:
http://www.heltec.cn/search/
ESP32ChipID=HexadecimalChipIDBlabla

Sometimes this did, sometimes this did not forward you to the ACTUAL AND CORRECT website you should REALLY visit, and namely:

https://resource.heltec.cn/search

There, enter in the search field the HexadecimalChipIDBlabla that you got in the message on the serial monitor. It will return three hexadecimal numbers, starting with 0x, e.g. 0x1111AAAA,0x2222BBBB,0x3333CCCC

Copy and paste them into a text editor. There, remove the 0x and the commata and fuse them, in the example given:

1111AAAA2222BBBB3333CCCC

... and prefix them with AT+CDKEY= so that you, in said text editor, get:

AT+CDKEY=1111AAAA2222BBBB3333CCCC

Open a serial terminal connection, e.g.:

sudo picocom --baud 115200 --parity n --databits 8 --stopbits 1 --flow n --echo /dev/ttyUSB0

Then, as the machine is spouting at you:

Please provide a correct license! For more information:
http://www.heltec.cn/search/
ESP32ChipID=HexadecimalChipIDBlabla

simply enter (there is no prompt, so best copy-paste) the following:

AT+CDKEY=1111AAAA2222BBBB3333CCCC

and you should get on the serial port:

The board is actived

Ta-daa! Your Heltec WiFi LoRa32 V3 board is useable again.

