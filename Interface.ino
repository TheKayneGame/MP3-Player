//------------------------------------------------------------------------------
// Interface for MP3Player
// 
// Version 1.0: Initial version with empty functions

//------------------------------------------------------------------------------
// Set the value of lcdInterface to 4 when using the 4-bit interface, 
// or to 8 when using the 8-bit interface of the LCD. 

int lcdInterface = 0; 

//------------------------------------------------------------------------------
// This function initializes the pins used by the interface. Note that 
// the interface should only use pins A0 through A5 of the Arduino. 

void interfaceSetup() 
{

}

//------------------------------------------------------------------------------
// This function drives the leds using the lower nibble of parameter 
// data. A '1' will turn on the led. 
// 
// +------+------+------+------+------+------+------+------+
// |   0  |   0  |   0  |   0  | led3 | led2 | led1 | led0 |
// +------+------+------+------+------+------+------+------+

void ledsWrite(byte data)
{
  
}

//------------------------------------------------------------------------------
// This function reads and returns the status of the four buttons. A pressed 
// button is represented by a '1' in the lower nibble of the returned value:
//
// +------+------+------+------+------+------+------+------+
// |   0  |   0  |   0  |   0  | prev | stop | play | next |
// +------+------+------+------+------+------+------+------+

byte buttonsRead()
{
	return 0;
}

//------------------------------------------------------------------------------
// This function sets RS (0 = instr, 1 = data), places data at the DB-lines 
// and generates a rising edge on the E-line of the LCD.

void lcdWrite(bool rs, byte data) 
{

}

//------------------------------------------------------------------------------

