// Control OSD chip and video MUX for rover
//#include <WProgram.h>

#include "MAX7456.h"

char serialData;
String received;
int horizOffset = 28;
int vertOffset = -15;

MAX7456 osd;


void setup()
{
  Serial.begin(9600);

  osd.begin();
  //adjust horiz & vert offset
  osd.offset(horizOffset, vertOffset);
  info_screen();

  //  test_screen();
  //  delay(5000);
  Serial.println("Device booted and ready for commands");
}


void loop()
{
  //  osd.clear();
  receive_commands();
} 

void info_screen() 
{
  osd.clear();
//    osd.println("First Line");
  osd.println("\n\n\n\n");
  osd.println("       MARS ROVER OSD");
  osd.println("\n\n\n");
  osd.println("Build Date: 18 Mar 2012");
  osd.println("Build SHA:  973f56acd3");

  delay(5000);
  osd.clear();
}

void test_screen()
{
  osd.clear();
  osd.println("1");
  osd.println("2");
  osd.println("3");
  osd.println("4");
  osd.println("5");
  osd.println("6");
  osd.println("7");
  osd.println("8");
  osd.println("9");
  osd.println("10");
  osd.println("11");
  osd.println("12");
  osd.println("13");
}

bool receive_commands() {
  if (Serial.available() > 0) {
    serialData = Serial.read();
    // Get command type (5 characters long each)
    if(serialData == '$')  {
      while(Serial.available() < 5);

      received = String(serialData);
      while(Serial.available() > 0){
        received += (char)Serial.read();
      }

      // DEBUG
      Serial.println("Received command");
      Serial.println(received);

      String exampleCommand = "";

      if(received == "$CLEAR") {
        osd.clear();
        Serial.println("Cearing screen");
        return true;
      }
      else if(received == "$PRINT") {
        //Command of the form $PRINT,00,Text to be printed to a line,
        // where 00 is the line number and everything in the third parameter is 
        // the string to be printed to the screen.
        exampleCommand = "$PRINT,01,Text to be printed to a line,";

        //Check ,00, portion and store line number
        while(Serial.available() < 4);

        serialData = Serial.read();
        received += (char)serialData;

        if(serialData != ',') {
          bad_command(received, exampleCommand);
          return false;
        }

        int lineNumber = 0;
        for(int i=0; i<2; i++) {
          serialData = Serial.read(); 
          received += (char)serialData;

          // Convert from ASCII to number
          serialData -= '0';
          // Check that it's in range
          if((serialData) > 9 || (serialData) < 0) {
            bad_command(received, exampleCommand);
            return false;
          }
          lineNumber = lineNumber*10 + serialData;
        }
        Serial.print("Print to line: ");
        Serial.println(lineNumber, DEC);

        serialData = Serial.read();
        received += (char)serialData;

        if(serialData != ',') {
          bad_command(received, exampleCommand);
          return false;
        }


        // Parse text to be printed
        String lineText = "";
        while(Serial.available() > 0) {
          serialData = Serial.read();
          received += (char)serialData;
          Serial.print((char)serialData);

          if(serialData == ',') {
            // Finished parsing command
            osd.write_to_screen( &lineText[0], (byte)(lineNumber + vertOffset) );
            Serial.println(received);
            return true;
          }
          lineText += (char)serialData;
        }

      }
      else if(received == "$OFFST") {
        /*
         * $OFFST,00,00,
         * first 00 is the horizontal offset
         * second 00 is the vertical offset
         */

        // Match the first ,00 of the command
        while(Serial.available() < 3);

        serialData = Serial.read();
        received += (char)serialData;

        if(serialData != ',') {
          bad_command(received, exampleCommand);
          return false;
        }

        horizOffset = 0;
        for(int i=0; i<2; i++) {
          serialData = Serial.read(); 
          received += (char)serialData;

          // Convert from ASCII to number
          serialData -= '0';
          // Check that it's in range
          if((serialData) > 9 || (serialData) < 0) {
            bad_command(received, exampleCommand);
            return false;
          }
          horizOffset  = horizOffset*10 + serialData;
        }

        // Match the second ,00, of the command
        while(Serial.available() < 4);

        serialData = Serial.read();
        received += (char)serialData;

        if(serialData != ',') {
          bad_command(received, exampleCommand);
          return false;
        }

        vertOffset = 0;
        for(int i=0; i<2; i++) {
          serialData = Serial.read(); 
          received += (char)serialData;

          // Convert from ASCII to number
          serialData -= '0';
          // Check that it's in range
          if((serialData) > 9 || (serialData) < 0) {
            bad_command(received, exampleCommand);
            return false;
          }
          horizOffset  = horizOffset*10 + serialData;
        }

        serialData = Serial.read();
        received += (char)serialData;
        
        if(serialData != ',') {
          bad_command(received, exampleCommand);
          return false;
        }

        // Now that the command is validated, validate the range of the offsets
        // Horizontal offset should be between 0 and 63
        // Corresponding to the hardware spec of -32 to +31 pixel offset
        // Vertical offset should be between 0 and 31
        // Corresponding to the hardware spec of -15 to +16 pixel offset
        if(horizOffset > 63)
          horizOffset = 63;
        else if(horizoffset < 0)
          horizOffset = 0; //This shouldn't be necessary, since it shouldn't parse negative numbers, but it doesn't hurt to be careful

        if(vertOffset > 31)
          vertOffset = 31;
        else if(vertoffset < 0)
          vertOffset = 0; //This shouldn't be necessary, since it shouldn't parse negative numbers, but it doesn't hurt to be careful

        // Convert to range expected by the hardware
        horizOffset -= 32;
        vertOffset -=  15;

        // Set the offset
        osd.offset(horizOffset, vertOffset);
      }
      else {
        bad_command(received);
        return false;
      }
    }
  }
}

void bad_command(String error) {
  Serial.println("Bad or malformed command: " + error);
}

void bad_command(String error, String example) {
  Serial.println("Bad or malformed command: " + error);
  Serial.println("It looks like you were trying to send a command like: " + example);
}
