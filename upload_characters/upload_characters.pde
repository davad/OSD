// Upload characters to the MAX7456 

#include "MAX7456.h"
//#include <iostream>

MAX7456 osd;

void setup()
{
  Serial.begin(9600);
  osd.begin();
  Serial.println("Starting");
  // Including this in the header causes problems

  //Serial.println("Returning character data for address 1");
  
  //char chars[256] loaded from ModifiedCM1.c
  for(int addr = 0; addr < 0x0F; addr++) {
    Serial.print("Writing character 0x");
    Serial.println(addr, HEX);
//    osd.write_character(addr, chars[addr]);
  }
  Serial.println("Done");
 
/*
//%d
  for(int i = 0; i < 64; i++)
  {
    Serial.print("Character ");
    Serial.println(i);
    Serial.println(character[i],DEC);
//    char ascii[32];
//    sprintf(ascii, "%d", character[i]);
//    Serial.println(ascii);
  }
  Serial.println("Done transmitting character");
*/
}

void loop()
{
}

