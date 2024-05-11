# this is the makefile from SRP since I have no idea how to work with one
# slightly modified to work for openservo stuff and this project
# that's why there's a ton of stuff referencing SRP and attiny1634
# removed stuff from lbp

COMPILER = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
AVRDUDE = avrdude



# Change the DEVICE to "attiny4313" or "attiny1634" depending on the SRP version being used
CFLAGS = -Wall -Wextra -Os -flto -fno-exceptions -fpermissive -mmcu=atmega328p -fpermissive -Wno-sign-compare -Wno-unused-variable
FILES = OpenServo.cpp enc.cpp pid.cpp pulsectl.cpp pwm.cpp storage.cpp swspi.cpp timer.cpp

NAME = "Servo"

INC = -I C:\Users\Niklas\AppData\Local\Arduino15\packages\arduino\hardware\avr\1.8.6\libraries\SoftwareSerial\src -I C:\Users\Niklas\AppData\Local\Arduino15\packages\arduino\hardware\avr\1.8.6\cores\arduino -I C:\Users\Niklas\AppData\Local\Arduino15\packages\arduino\hardware\avr\1.8.6\variants\standard

DEF = -DF_CPU=16000000

PROGRAMMER = "avrisp"
# Change the DEVICE to "t4313" or "attiny1634" depending on the SRP version being used
DEVICE = "m328p"
PORT = "usb"
BAUD = 57600

AVRCONFIG =  -p $(DEVICE) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -v

all: elf

elf:
	$(COMPILER) $(FILES) $(INC) $(CFLAGS) $(DEF) -o $(NAME).elf

hex: elf
	$(OBJCOPY) -j .text -j .data -O ihex $(NAME).elf $(NAME).flash.hex
	$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex $(NAME).elf $(NAME).eeprom.hex

# Note: these sleep statements are there because avrdude fails at freeing the usb programmer fast enough for some reason.
# 
fuses: hex
	timeout 2
	$(AVRDUDE) -p $(DEVICE) -c $(PROGRAMMER) -P $(PORT) -b $(FUSEBAUD) -v -U lfuse:w:$(NAME).lfuse.hex -U hfuse:w:$(NAME).hfuse.hex -U efuse:w:$(NAME).efuse.hex


flash: hex
	timeout 2
	$(AVRDUDE) $(AVRCONFIG) -U flash:w:$(NAME).flash.hex

initialize: fuses flash

program: flash


disasm: elf
	$(OBJDUMP) -d $(NAME).elf > $(NAME).s

size:
	$(SIZE) $(NAME).elf

clean:
	rm $(NAME).elf $(NAME).s $(NAME).*.hex configuretool.zip