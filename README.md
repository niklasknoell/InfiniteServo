Old readme by Jelle:

    # MagneticEncoderOpenServo
    A programmable servo capable of turning a defined 1 turn 360° (or more)

    Video on youtube:

    [![1 turn 360° servo](http://img.youtube.com/vi/oHWwntjVp9A/0.jpg)](http://www.youtube.com/watch?v=oHWwntjVp9A "1 turn 360° servo")

    * A programmable servo, based on ATMEGA328P, TLE5012B, MC34931 or DRV8837, using the arduino enviroment
    * It's a defined travel servo, capable of going beyond the limits that a potentiometer has, without adding extra gears. It uses a magnet encoder to read it's position instead of a potentiometer.

    More information and images are to be found [on this rc groups thread](https://www.rcgroups.com/forums/showthread.php?3154439-1-turn-360%C2%B0-servo "1 turn 360° servo").

    ## Software

    The software programmed as is has a travel of 376° between the control pulses of 1000µs and 2000µs. The pulses can be wider but *as is* it is limited between around 800µs and 2200µs. providing a travel of more then 450°.

    The software can be reprogrammed to do the following:
    * Become a continious rotating servo.
    * Be a multiple turn servo, like a 10 turn sail winch servo.
    * Be current limited, the hardware is capable but not software implemented.
    * Have a speed limited movement.
    * Be a digital classic servo with a travel of just 90° or 180°.
    * Be programmed as time lapse very slow turning servo since there are 2^15 steps of precision.
    * Be programmed to accept commands instead of a PWM signal, or have different interaction interfaces.

    Dificulty of implementing varies on the topic.

    Possible drawbacks:
    * Since it uses a magnet, it might be interfered with by an external magnet.
    * If the shaft is turned >180° degrees while unpowered the servo will skip a turn, it should not be turned >90° when powered down to prevent for this unwanted behaviour.
    * Since there is no endstop there is a software implementation to catch where in a turn it was depowered. This uses EEPROM which has a 100000 write cycle. In my software implementation every 1/8th of a circle the position is saved. This is done in a ring memory containing 256 bytes and requires the old value to be erased (written with 0) and the next position to be written with the new position value. This means technically after 1600000 turns the atmega chips EEPROM might stop working correctly. If you succeed in breaking the EEPROM there are software solutions to prolong the atmega328p's life.

    Possible improvements:
    * In a fast movement the servo has a tendency to overshoot it's target value. At this moment a P&D is implemented in the PID, but I feel like the I could help out to not overshoot the target and have a more gracefull reach to the target value.
    * Code could be improved to integrated safety features like current limiting.
    * Code could be cleaned up.

    ## Hardware

    The required parts next the pcb's and it's components are:
    * A donor servo for it's housing, gears, motor and shaft of the potentiometer.
    * A Diametrically Magnetised round magnet, hard to find can possibly be substitued by a cube magnet.
    * Basic components of the hardware pcb are: ATMEGA328P, TLE5012B, MC34931 or DRV8837

    ## Open Source

    This fork is software that is **open source** and is available free of charge without warranty to all users.

    The license is GPL3.

    ## Project/Fork History

    Based on OpenServo: https://github.com/ginge/OpenServo

# Guide on how to use this properly:


### PCB
Somewhere in this project (once finalized) there should be a zip file containing the gerber files for the PCB, as well as a BOM and pick and place file. Upload either only Gerbers or additionally the BOM and pick and place file to JLCPCB or similar to have them manufacture and assemble the boards. I opted to have JLC manufacture the board and then assemble only the topside, since that's where the majority of all components as well as the hard to solder ones are. The magnetic sensor I ordered seperately and assemble myself. That's for the PCB.

### Servo disassembly
The servo itself needs to be disassembled, the motor and potentiometer desoldered from the board and a diametric (!!!) magnet attached to the potentiometer in its original position. Basically, look at what Jelle's pictures look like, they're quite good. 

### Reassembly
The new PCB will be soldered to the motor, and cables attached to the small through hole pads where the old pcb had them as well. The pads right next to the motor are the programming pins, which will need to get a connection somehow for initially programming the bootloader, so they will likely need some temporary cables or something to connect them to an AVR programmer (i will be using ArduinoISP. If I dont manage to squeeze the labelling on the board look on the picture i hope to include in the future or the design files what the pinout is. This will require custom cable beuning to connect to conventional avr programmer as the pinout is not in the normal 3x2 format)

### Flashing the bootloader

Now that we hopefully can power and communicate with the board through the previously created temporary connections we should flash the bootloader, so that later we can reprogram the board using only the PWM signal cable thanks to Jelle's amazing bootloader and programmer mods. 

The bootloader can be compiled with the following command line: 
```
make LED_START_FLASHES=0 SOFT_UART=1 atmega328
```
The 4 seconds boot delay in case of a high signal can be changed in the code.

Upload the bootloader hex using an arduino running the ArduinoISP sketch with the following command:
Erase chip, unlock bootloader section and set fuses for clock and bootloader size
```
avrdude -c avrisp -p m328p  -v -e -U lock:w:0xFF:m -U efuse:w:0xFD:m -U hfuse:w:0xDA:m -U lfuse:w:0xFF:m
```
Program chip and lock bootloader section:
```
avrdude -c avrisp -p m328p  -v -e -U flash:w:bootloader.hex -U lock:w:0x0F:m
```
(Blatantly stolen/modified from the bootloader readme)

### Programming using bootloader and programmer

Now we should test if we can actually program the board through the newly installed bootloader, so we need to install the programmer.ino flash from the Programmer directory onto an Arduino. So I'll use the previously used Arduino to act as my programmer. We have to flash the programmer straight to it without any bootloader, such that it does not enter the bootloader itself. To do that we need to completely wipe that bootloader and any lockbits and stuff. First open arduino IDE and click verify to compile the sketch. Find where it saves that sketch (should be in the output stuff down in the arduino window)

copy that programmer.hex file over here and run avrdude like this to flash that sketch while removing the bootloader at the same time
```
avrdude -c USBasp -p m328p  -v -e -U lock:w:0x3F:m -U hfuse:w:0xD9:m -U flash:w:programmer.hex -U lock:w:0x0F:m
```


Now the programmer should be ready, connect up your servo PCB 5V, GND and signal to PB0. 

Now you should be able to upload the openServo program to your servoboard through the signal wire, however you first need to actually compile that, so simply go into the OpenServo directory and write 
```
make hex
```
(Probably you will get an error about some timer being used already. This is arduino using that timer for something already, which can be bypassed by modifying that source file locally as described here https://stackoverflow.com/questions/46573550/atmel-arduino-isrtimer0-ovf-vect-wont-compile-first-defined-in-vector )


With that small beunfix it should now work. It should give you some warnings about code in swspi.cpp, but still compile out OpenServo.hex.

With that we can now finally program the servo as follows:

```
avrdude.exe -v -p m328p -c arduino -P <COMPORT> -b 115200 -e -U flash:w:OpenServo.hex
```

Once we have this working we can feel comfortable with reassembling everything and closing up the servo. Anytime we want to change the software we simply recompile the OpenServo directory and rerun 
```
avrdude.exe -v -p m328p -c arduino -P <COMPORT> -b 115200 -e -U flash:w:OpenServo.hex
```
with the programmer attached. 


# 2024 Updates by Niklas (not tested yet, waiting for boards):

## Hardware: 

- Created single board version for typical 9g servos
- include LDO so can be powered by 2s lipo (pending test if motor will burn up)
- breaking out some more pins for possible software serial if wanted


## Software:
- Moved everything to pure cpp, using (scuffed) makefiles instead of arduino ide
- still using Arduino.h just for simplicity tho
- Increased the integer sizes storing all the position things
- Made EEPROM storage into words, to allow extended storage of position through power removal
- Now each rotation has 32768 steps, taking values straight from the angle sensor
- The EEPROM stored value has 256 steps per rotation, meaning up to 256 revolution servo can be implemented
- Pulse width range and corresponding position range refactored to config.h to allow reprogramming from single point. 
- Probably a few more things I am forgetting.