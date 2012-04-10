/*
  Arduino library for MAX7456 video overlay IC

  based on code from Arduino forum members dfraser and zitron
  http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1220054359
  modified/extended by kg4wsv
  gmail: kg4wsv
*/

#include <WProgram.h>
#include "MAX7456.h"


byte MAX7456_spi_transfer(volatile char data)
{
  SPDR = data;                    // Start the transmission
  while (!(SPSR & (1<<SPIF)))     // Wait the end of the transmission
  {
  };
  return SPDR;                    // return the received byte
}


//#define MAX7456_spi_transfer(X) {SPDR = X; while (!(SPSR & (1<<SPIF))) {;}}

MAX7456::MAX7456()
{
  _slave_select = MAX7456SELECT;
  _char_attributes = 0x01;
  _cursor_x = CURSOR_X_MIN;
  _cursor_y = CURSOR_Y_MIN;
}


void MAX7456::begin(byte slave_select)
{
  _slave_select = slave_select;
  begin();
}


void MAX7456::begin()
{
  byte spi_junk;
  int x;

  pinMode(_slave_select,OUTPUT);
  digitalWrite(_slave_select,HIGH); //disable device

  pinMode(MAX7456_DATAOUT, OUTPUT);
  pinMode(MAX7456_DATAIN, INPUT);
  pinMode(MAX7456_SCK,OUTPUT);
//  pinMode(MAX7456_VSYNC, INPUT);

  // SPCR = 01010000
  //interrupt disabled,spi enabled,msb 1st,master,clk low when idle,
  //sample on leading edge of clk,system clock/4 rate (4 meg)

  MAX7456_previous_SPCR = SPCR;  // save SPCR, so we play nice with other SPI peripherals

  SPCR = (1<<SPE)|(1<<MSTR);
  spi_junk=SPSR;
  spi_junk=SPDR;
  delay(250);
  MAX7456_SPCR = SPCR;

  // force soft reset on MAX7456
  digitalWrite(_slave_select,LOW);
  MAX7456_spi_transfer(VM0_WRITE_ADDR);
  MAX7456_spi_transfer(MAX7456_reset);
  digitalWrite(_slave_select,HIGH);
  delay(500);

  // set all rows to same charactor white level, 90%
  digitalWrite(_slave_select,LOW);
  for (x = 0; x < MAX_screen_rows; x++)
  {
    MAX7456_spi_transfer(x + 0x10);
    MAX7456_spi_transfer(WHITE_level_90);
  }
  digitalWrite(_slave_select,HIGH);

  // make sure the MAX7456 is enabled
  digitalWrite(_slave_select,LOW);
  MAX7456_spi_transfer(VM0_WRITE_ADDR);
  MAX7456_spi_transfer(VERTICAL_SYNC_NEXT_VSYNC|OSD_ENABLE);
  digitalWrite(_slave_select,HIGH);
  delay(100);

  digitalWrite(_slave_select,LOW);
  MAX7456_spi_transfer(VM1_WRITE_ADDR);
  MAX7456_spi_transfer(BLINK_DUTY_CYCLE_75_25);
  digitalWrite(_slave_select,HIGH);
  delay(100);

  SPCR = MAX7456_previous_SPCR;   // restore SPCR
}

 
byte MAX7456::convert_ascii(int character) 
{
// for some reason the MAX7456 does not follow ascii letter
// placement, so you have to have this odd lookup table

  byte lookup_char;

  if (character == 32)
    lookup_char = 0x00; // blank space
  else if (character == 48)
    lookup_char = 0x0a; // 0
  else if ((character > 48) && (character < 58))
    lookup_char = (character - 48); // 1-9
  else if ((character > 64) && (character < 90))
    lookup_char = (character - 54); // A-Z
  else if ((character > 96) && (character < 123))
    lookup_char = (character - 60); // a-z
  else if (character == 34)
    lookup_char = 0x48; // "
  else if (character == 39)
    lookup_char = 0x46; // '
  else if (character == 40)
    lookup_char = 0x3f; // (
  else if (character == 41)
    lookup_char = 0x40; // )
  else if (character == 44)
    lookup_char = 0x45; // ,
  else if (character == 45)
    lookup_char = 0x49; // -
  else if (character == 46)
    lookup_char = 0x41; // .
  else if (character == 47)
    lookup_char = 0x47; // /
  else if (character == 58)
    lookup_char = 0x44; // :
  else if (character == 59)
    lookup_char = 0x43; // ;
  else if (character == 60)
    lookup_char = 0x4a; // <
  else if (character == 62)
    lookup_char = 0x4b; // >
  else if (character == 63)
    lookup_char = 0x42; // ?
//  else
//    lookup_char = 0x00; // out of range, blank space

 return (lookup_char);
}
// Adjust the horizontal and vertical offet
// Horizontal offset between -32 and +31
// Vertical offset between -15 and +16
void MAX7456::offset(int horizontal, int vertical)
{
  //Constrain horizontal between -32 and +31
  if (horizontal < -32) 
     horizontal = -32;
  else if (horizontal > 31)
    horizontal = 31;
   
  //Constrain vertical between -15 and +16
  if (vertical < -15) 
    vertical = -15;
  else if (vertical > 16)
    vertical = 16;

  // Write new offsets to the OSD
  MAX7456_previous_SPCR = SPCR;  // save SPCR, so we play nice with other SPI peripherals
  SPCR = MAX7456_SPCR;  // set SPCR to what we need
  
  digitalWrite(_slave_select,LOW);
  MAX7456_spi_transfer(HOS_WRITE_ADDR); //horizontal offset
  MAX7456_spi_transfer(horizontal);
  
  MAX7456_spi_transfer(VOS_WRITE_ADDR); //vertical offset
  MAX7456_spi_transfer(vertical);
  digitalWrite(_slave_select,HIGH);  
  SPCR = MAX7456_previous_SPCR;   // restore SPCR   
  
}
// clear the screen
void MAX7456::clear()
{
  MAX7456_previous_SPCR = SPCR;  // save SPCR, so we play nice with other SPI peripherals
  SPCR = MAX7456_SPCR;  // set SPCR to what we need

  digitalWrite(_slave_select,LOW);
  MAX7456_spi_transfer(DMM_WRITE_ADDR);
  MAX7456_spi_transfer(CLEAR_display);
  digitalWrite(_slave_select,HIGH);

  SPCR = MAX7456_previous_SPCR;   // restore SPCR

  _cursor_x = CURSOR_X_MIN;
  _cursor_y = CURSOR_Y_MIN;
} 

// send the cursor to home
void MAX7456::home()
{
  _cursor_x = CURSOR_X_MIN;
  _cursor_y = CURSOR_Y_MIN;
} 



// this is probably inefficient, as i simply modified a more general function
// that wrote arbitrary length strings. need to check modes of writing
// characters to MAX7456 to see if there's a better way to write one at a time
void MAX7456::write(uint8_t c)
{
  unsigned int linepos;
  byte char_address_hi, char_address_lo;

  if (c == '\n')
    {
      _cursor_y++;
      if (_cursor_y > CURSOR_Y_MAX)
	_cursor_y = CURSOR_Y_MIN;
      _cursor_x = CURSOR_X_MIN;
      return;
    }

  if (c == '\r')
    {
      _cursor_x = CURSOR_X_MIN;
      return;
    }

  // To print non-ascii character, this line must either be commented out
  // or convert_ascii() needs to be modified to make sure it doesn't conflict
  // with the special characters being printed.
  c = convert_ascii(c);

  MAX7456_previous_SPCR = SPCR;  // save SPCR, so we play nice with other SPI peripherals
  SPCR = MAX7456_SPCR;  // set SPCR to what we need

  char_address_hi = 0;
  char_address_lo = 0;
    
  // convert x,y to line position
  linepos = _cursor_y * 30 + _cursor_x;
  _cursor_x++;
  if (_cursor_x >= CURSOR_X_MAX)
    {
      _cursor_y++;
      if (_cursor_y > CURSOR_Y_MAX)
	_cursor_y = CURSOR_Y_MIN;
      _cursor_x = CURSOR_X_MIN;
    }

  
  // divide in to hi & lo byte
  char_address_hi = linepos >> 8;
  char_address_lo = linepos;
  
  
  digitalWrite(_slave_select,LOW);

  MAX7456_spi_transfer(DMM_WRITE_ADDR); //dmm
  MAX7456_spi_transfer(_char_attributes);

  MAX7456_spi_transfer(DMAH_WRITE_ADDR); // set start address high
  MAX7456_spi_transfer(char_address_hi);

  MAX7456_spi_transfer(DMAL_WRITE_ADDR); // set start address low
  MAX7456_spi_transfer(char_address_lo);
  
  
  MAX7456_spi_transfer(DMDI_WRITE_ADDR);
  MAX7456_spi_transfer(c);
  
  MAX7456_spi_transfer(DMDI_WRITE_ADDR);
  MAX7456_spi_transfer(END_string);
  
  MAX7456_spi_transfer(DMM_WRITE_ADDR); //dmm
  MAX7456_spi_transfer(B00000000);

  digitalWrite(_slave_select,HIGH);

  SPCR = MAX7456_previous_SPCR;   // restore SPCR
  
}


void MAX7456::write_to_screen(char s[], byte x, byte y)
{
  write_to_screen(s, x, y, 0, 0);
}

void MAX7456::write_to_screen(char s[], byte line)
{
  write_to_screen(s, 1, line, 0, 0);
}


void MAX7456::write_to_screen(char s[], byte x, byte y, byte blink, byte invert){
  unsigned int linepos;
  byte local_count;
  byte settings, char_address_hi, char_address_lo;
  byte screen_char;


  MAX7456_previous_SPCR = SPCR;  // save SPCR, so we play nice with other SPI peripherals
  SPCR = MAX7456_SPCR;  // set SPCR to what we need

  local_count = 0;

  char_address_hi = 0;
  char_address_lo = 0;
    
  // convert x,y to line position
  linepos = y*30+x;
  
  // divide in to hi & lo byte
  char_address_hi = linepos >> 8;
  char_address_lo = linepos;
  
  
  settings = B00000001;
  
  // set blink bit
  if (blink) {
    settings |= (1 << 4);       // forces nth bit of x to be 1.  all other bits left alone.
    //x &= ~(1 << n);      // forces nth bit of x to be 0.  all other bits left alone.  
  }
  // set invert bit
  if (invert){
    settings |= (1 << 3);       // forces nth bit of x to be 1.  all other bits left alone.
  }

  
  digitalWrite(_slave_select,LOW);

  MAX7456_spi_transfer(DMM_WRITE_ADDR); //dmm
  MAX7456_spi_transfer(settings);

  MAX7456_spi_transfer(DMAH_WRITE_ADDR); // set start address high
  MAX7456_spi_transfer(char_address_hi);

  MAX7456_spi_transfer(DMAL_WRITE_ADDR); // set start address low
  MAX7456_spi_transfer(char_address_lo);
  
  
  while(s[local_count]!='\0') // write out full screen
  {
    screen_char = convert_ascii(s[local_count]);
    MAX7456_spi_transfer(DMDI_WRITE_ADDR);
    MAX7456_spi_transfer(screen_char);
    local_count++;
  }
  
  MAX7456_spi_transfer(DMDI_WRITE_ADDR);
  MAX7456_spi_transfer(END_string);
  
  MAX7456_spi_transfer(DMM_WRITE_ADDR); //dmm
  MAX7456_spi_transfer(B00000000);

  digitalWrite(_slave_select,HIGH);

  SPCR = MAX7456_previous_SPCR;   // restore SPCR
} 


void MAX7456::blink(byte onoff)
{
  if (onoff)
    {
      _char_attributes |= 0x10;
    }
  else
    {
      _char_attributes &= ~0x10;
    }
}

void MAX7456::blink()
{
  blink(1);
}

void MAX7456::noBlink()
{
  blink(0);
}


void MAX7456::invert(byte onoff)
{
  if (onoff)
    {
      _char_attributes |= 0x08;
    }
  else
    {
      _char_attributes &= ~0x08;
    }
}

void MAX7456::invert()
{
  invert(1);
}

void MAX7456::noInvert()
{
  invert(0);
}

// Returns all 0s for some reason
void MAX7456::read_character(byte addr, char character[]) 
{
  // Enable the SPI
  // Write VM0[3] = 0 to disable the OSD image.
  Serial.println("Enable SPI, disable OSD output (section)");

  MAX7456_previous_SPCR = SPCR;  // save SPCR, so we play nice with other SPI peripherals
  SPCR = MAX7456_SPCR;  // set SPCR to what we need

  digitalWrite(_slave_select, LOW);
  //Serial.println("Enable SPI");
  MAX7456_spi_transfer(VM0_WRITE_ADDR);
  //Serial.println("Sent VM0_WRITE_ADDR");
  MAX7456_spi_transfer(0x00|VERTICAL_SYNC_NEXT_VSYNC); // double check that the other bits can be zero
  //delay(100); // Not sure if this delay is needed. It's used in the init code
  
  //Serial.println("Select character by address");
  // Write CMAH[7:0]  = xxH  to select the character (0–255) to be read
  MAX7456_spi_transfer(CMAH_WRITE_ADDR);
  MAX7456_spi_transfer(addr);

  //Serial.println("Read character data from NVRAM to Shadow RAM");
  // Write CMM[7:0] = 0101xxxx to read the character data from the NVM to the shadow RAM
  MAX7456_spi_transfer(CMM_WRITE_ADDR);
  MAX7456_spi_transfer(0b01010000); // Double check that bits 0-3 can be zero
  
  char test[54];

  for(int i = 0; i < 64; i++)
  {
    // Write CMAL[7:0] = xxH to select the 4-pixel byte (0–63) in the character to be read
    MAX7456_spi_transfer(CMAL_WRITE_ADDR);
    MAX7456_spi_transfer(i);

    // Read CMDO[7:0] = xxH to read the selected 4-pixel byte of data
    test[i] = MAX7456_spi_transfer(CMDO_READ_ADDR);
    character[i] = MAX7456_spi_transfer(i);
  }

  // Write VM0[3] = 1 to enable the OSD image display.
  MAX7456_spi_transfer(VM0_WRITE_ADDR);
  MAX7456_spi_transfer(VERTICAL_SYNC_NEXT_VSYNC|OSD_ENABLE);
  
  digitalWrite(_slave_select, HIGH);
  SPCR = MAX7456_previous_SPCR;   // restore SPCR
  /*Serial.println("Done reading. Data collected.");
  
  Serial.println("Character: ");
  for(int i = 0; i < 54; i++) {
    Serial.print("0x");
    Serial.print(character[i], HEX);
    Serial.print(",");
  }
  Serial.println();
  Serial.println("Test: ");
  for(int i = 0; i < 54; i++) {
    Serial.print("0x");
    Serial.print(test[i], HEX);
    Serial.print(",");
  }
  Serial.println();
  */
}


void MAX7456::write_character(byte addr, char character[]) 
{
  // Enable the SPI
  // Write VM0[3] = 0 to disable the OSD image.
  Serial.println("Enable SPI, disable OSD output (section)");

  MAX7456_previous_SPCR = SPCR;  // save SPCR, so we play nice with other SPI peripherals
  SPCR = MAX7456_SPCR;  // set SPCR to what we need

  digitalWrite(_slave_select, LOW);
  MAX7456_spi_transfer(VM0_WRITE_ADDR);
  MAX7456_spi_transfer(0x00|VERTICAL_SYNC_NEXT_VSYNC); // double check that the other bits can be zero
  
  // Write CMAH[7:0]  = xxH  to select the character (0–255) to be read
  MAX7456_spi_transfer(CMAH_WRITE_ADDR);
  MAX7456_spi_transfer(addr);

  for(int i = 0; i < 54; i++)
  {
    // Write CMAL[7:0] = xxH to select the 4-pixel byte (0–63) in the character to be read
    MAX7456_spi_transfer(CMAL_WRITE_ADDR);
    MAX7456_spi_transfer(i);
    
    // Write CMDI[7:0] = xxH to set the pixel values of the selected part of the character
    MAX7456_spi_transfer(CMDI_WRITE_ADDR);
    MAX7456_spi_transfer(character[i]);
  }

  // Write CMM[7:0] = 1010xxxx to write the character data from the shadow RAM to the NVRAM
  MAX7456_spi_transfer(CMM_WRITE_ADDR);
  MAX7456_spi_transfer(0b10100000); // Double check that bits 0-3 can be zero
  
  // Wait at least 12ms to finish writing
  delay(150);

  // Write VM0[3] = 1 to enable the OSD image display.
  MAX7456_spi_transfer(VM0_WRITE_ADDR);
  MAX7456_spi_transfer(VERTICAL_SYNC_NEXT_VSYNC|OSD_ENABLE);
  
  digitalWrite(_slave_select, HIGH);
  SPCR = MAX7456_previous_SPCR;   // restore SPCR
}
