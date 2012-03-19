#include <WProgram.h>

#include "MAX7456.h"
MAX7456 osd;


void setup()
{
  Serial.begin(9600);
  Serial.println("testing MAX7456 library");


  osd.begin();
  //adjust horiz & vert offset
  osd.offset(28,-15);
  info_screen();
  delay(5000);
  osd.clear();
}


void loop()
{
} 

void info_screen() 
{
  osd.clear();
  //  osd.println("First Line");
  osd.println("\n\n\n\n");
  osd.println("       MARS ROVER OSD");
  osd.println("\n\n\n");
  osd.println("Build Date: 18 Mar 2012");
  osd.println("Build SHA:  973f56acd3");
}

