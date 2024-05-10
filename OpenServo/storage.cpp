// storage.cpp
//
// Software SPI for OpenServo
//
// Copyright (C) 2018  Jelle L
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

#include <inttypes.h>
#include <avr/eeprom.h>

// byte 0 is 69 to show that we have initialized eeprom
// bytes 1 to End are data

#define MIN_INDEX 1
#define MAX_INDEX (E2END-1) // Atmega328P has 1024 bytes of eeprom
uint16_t index;
uint16_t eeprom_saved_pos;



int eeprom_initialized() {
    return (eeprom_read_byte(0) == 69);
}

// basically flushes all of eeprom between min and max indices to 0 if not already set to 0
void eeprom_flush(void){
    //initialize all fully accessible words
    uint16_t totalNumWords = (MAX_INDEX>>1) - MIN_INDEX;
    for(uint16_t i=0; i < totalNumWords; i++){
        if(eeprom_read_word((uint16_t *) (MIN_INDEX + (i<<1)))!=0){
            eeprom_update_word((uint16_t *) (MIN_INDEX + (i<<1)), 0);
        }
    }

    eeprom_write_byte(0, 69); // indicate we've been around and initialized the eeprom
    //write good first value to register, should only happen once in lifetime of servo
    eeprom_write_word((uint16_t *) MIN_INDEX, 32);
}

void storage_init(void){
    index = MIN_INDEX;
    //find index find first non empty value
    while(eeprom_read_word((uint16_t *)index)==0){
        index++;
    }
    
    eeprom_saved_pos = eeprom_read_word((uint16_t *)index);
}

void storage_update(int32_t position){

    int16_t store_pos = position >> 7;

    if(store_pos < eeprom_saved_pos-2 || store_pos > eeprom_saved_pos+2 ){
        eeprom_update_word((uint16_t *) index, 0);
        index += 2;
        if(index>=MAX_INDEX)index=MIN_INDEX;
        eeprom_update_word((uint16_t *) index, (uint16_t) store_pos);        
        eeprom_saved_pos = store_pos;
    }
}
