// enc.c
//
// OpenEncoder driver for OpenServo
//
// Copyright (C) 2009-2010  Darius Rad <alpha@area49.net>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include <stdint.h>
#include "enc.h"
#include "swspi.h"
#include "storage.h"

int32_t offset;
uint16_t angle;
int16_t revs;


static bool _reg_read_serial_interface_unit()
{
    

    swspi_save();
    //bool valid = swspi_read(0x8021, &data);
    if (swspi_read(0x8421, &angle) & swspi_read(0x8221, &revs)) {
        angle = angle & 0x7fff; // MSB is just new data indicator
        revs = (revs & 0x01ff);
        return 1;
    }
    else{
        return 0;
    }
}

void enc_init(void){
    //measure position without offset
    offset = 0<<11;
    uint32_t position = enc_get_position_value(); // this should be in first rev, so 0...32768
    //check current position against previous position and calculate offset
    // eeprom has saved previous session, so if pos is at 0.5 revs but eeprom is at 3.5 offset will be 3
    offset = ((eeprom_saved_pos<<7)-position);
}



int32_t enc_get_position_value(void) //
{
    int32_t ret = offset;
    if (_reg_read_serial_interface_unit()) {
        ret += 32768 * revs + angle; 
    }
    else { 
        ret = 0xFFFFFFFF;
    }
    return ret;
}
