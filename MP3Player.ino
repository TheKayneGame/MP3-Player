//------------------------------------------------------------------------------
// MP3Player
//
// This sketch uses libraries SFEMP3Shield and SdFat. Make sure these libraries
// are in the libraries directory in your Arduino installation.
//
// The MP3Player uses MP3-files on the SD-card. 

#define VERSION "1.0"

#include <SdFat.h>
#include <SFEMP3Shield.h>

#define LED_STOP  0x01
#define LED_PLAY  0x02
#define LED_PAUSE 0x04

#define CMD_NONE  0
#define CMD_PREV  1
#define CMD_NEXT  2
#define CMD_PLAY  3
#define CMD_PAUSE 4
#define CMD_STOP  5

#define MAX_MUSIC 8

SdFat sd;
SFEMP3Shield MP3player;

extern int lcdInterface;

struct MP3
{
  char filename[13];
  char artist[16];
  char title[16];
};

MP3 musicList[MAX_MUSIC];
int musicSelected = 0;
int musicCount;

//------------------------------------------------------------------------------
// This function initializes the interface with the LCD, the SD-card, 
// the MP3-shield and the timer. It displays information on the LCD 
// and flashes the leds.

void setup() 
{
  Serial.begin(9600);
  interfaceSetup();
  lcdInit();
    
  // Initialize the SdCard
  if (!sd.begin(SD_SEL, SPI_FULL_SPEED)) sd.initErrorHalt();
  if (!sd.chdir("/")) sd.errorHalt("sd.chdir");

  // Initialize the MP3 shield
  MP3player.begin();
  musicCount = findMusic(musicList);

  ledsRun();
  ledsWrite(LED_STOP); 

  Serial.println("AVANS MP3player " VERSION "\n");
  Serial.print(musicList[musicSelected].artist);
  Serial.print(" - ");
  Serial.println(musicList[musicSelected].title);

  lcdWriteString(0, "AVANS MP3Player");
  timerInit();
}

//------------------------------------------------------------------------------
// This function initializes timer 1 to generate interrupt requests twice 
// a second. This is used to scroll information on the LCD.

void timerInit()
{
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 34286;            // preload timer 65536-16MHz/256/2Hz
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
}

//------------------------------------------------------------------------------
// This functions finds the character to display when the text (artist - title)
// is shifted by 'shift' characters.

char getChar(int *shift, int index)
{
  int x = *shift + index;

  int lenArtist = strlen(musicList[musicSelected].artist);
  int lenTitle = strlen(musicList[musicSelected].title);

  if (*shift >= lenArtist + lenTitle + 6) *shift = 0;
  
  while (1)
  {
    if (x < lenArtist) return musicList[musicSelected].artist[x];
    x -= lenArtist;
  
    if (x < 3) return " - "[x];
    x -= 3;

    if (x < lenTitle) return musicList[musicSelected].title[x];
    x -= lenTitle;
    
    if (x < 3) return " - "[x];
    x -= 3;
  }
    
  return 0;
}

//------------------------------------------------------------------------------
// This function is the interrupt service routine for timer 1, which generates
// an interrupt request twice a second. The function scrolls artist and title 
// on the lower line of the LCD.

ISR(TIMER1_OVF_vect)        
{
  static char displayStr[16];
  static int shift = 0;

  TCNT1 = 34286;            

  for (int i = 0; i < 16; i++)
  {
    displayStr[i] = getChar(&shift, i);
  }

  lcdWriteString(1, displayStr);
  shift++;
}

//------------------------------------------------------------------------------
// The loop-function first checks if a button on the interface is pressed. 
// If not, it checks if a key sent through the serial monitor. It uses the
// information from the interface and the serial line to handle commands
// like NONE, PREV, NEXT, PLAY, PAUSE and STOP. 

void loop() 
{
  static bool isPlaying = false;
  static bool isPaused = false;
  static bool isNewMusic = false;
  static byte lastButtons = 0;

  byte command = CMD_NONE;  
  byte buttons = buttonsRead();
   
  if (buttons != lastButtons)
  {
    switch (buttons)
    {
      case 0x00: command = CMD_NONE;
                 break;
      case 0x01: command = CMD_PREV;
                 break;
      case 0x02: command = isPlaying ? CMD_PAUSE : CMD_PLAY;
                 break;
      case 0x04: command = CMD_STOP;
                 break;
      case 0x08: command = CMD_NEXT;
                 break;
    }
    lastButtons = buttons;
  }

  if ((command == CMD_NONE) && Serial.available()) 
  {
    switch (Serial.read())
    {
      case ',':
      case '<': command = CMD_PREV;
                break;
      case '.':
      case '>': command = CMD_NEXT;
                break;
      case 'p': command = CMD_PLAY;
                break;
      case 'a': command = CMD_PAUSE;
                break;
      case 's': command = CMD_STOP;
                break;
    }
  } 

  switch (command)
  {
    // No command
    case CMD_NONE:  break;
    
    // Previous track          
    case CMD_PREV:  if (--musicSelected < 0) 
                    {
                      musicSelected = musicCount - 1;
                    }
                    Serial.print(musicList[musicSelected].artist);
                    Serial.print(" - ");
                    Serial.println(musicList[musicSelected].title);
                    isNewMusic = true;
                    break; 
      
    // Next track 
    case CMD_NEXT:  if (++musicSelected >= musicCount) 
                    {
                      musicSelected = 0;
                    }
                    Serial.print(musicList[musicSelected].artist);
                    Serial.print(" - ");
                    Serial.println(musicList[musicSelected].title);
                    isNewMusic = true;
                    break;
              
    // Play track
    case CMD_PLAY:  if (isNewMusic)
                    {
                      MP3player.stopTrack();
                      isNewMusic = false;
                      isPlaying = false;
                    }
                    if (isPlaying)
                    {
                      if (isPaused)
                      {
                        MP3player.resumeMusic();
                        Serial.println("Resume");
                        ledsWrite(LED_PLAY);
                        isPaused = false;
                      }
                      break; 
                    }
                    MP3player.playMP3(musicList[musicSelected].filename);
                    Serial.println("Play");
                    ledsWrite(LED_PLAY);
                    isPlaying = true;
                    break;
      
    // Pause track
    case CMD_PAUSE: if (isPlaying)
                    {
                      if (isPaused)
                      {
                        MP3player.resumeMusic();
                        Serial.println("Resume");
                        ledsWrite(LED_PLAY);
                        isPaused = false;
                      } 
                      else
                      {
                        MP3player.pauseMusic();
                        Serial.println("Pause");
                        ledsWrite(LED_PAUSE);
                        isPaused = true;
                      }
                    }
                    break;
      
    // Stop track
    case CMD_STOP:  MP3player.stopTrack();
                    Serial.println("Stop");
                    ledsWrite(LED_STOP);
                    isPlaying = false;
                    break;
  }

  if (isPlaying && (MP3player.getState() == ready))
  {
    Serial.println("Ready");
    ledsWrite(LED_STOP);
    isPlaying = false;
  }
}

//------------------------------------------------------------------------------
// This function locates music-files on the SD-card and populates 
// the array musicList with filename, title and artist information.

int findMusic(MP3 *MusicList)
{
  char filename[13];
  char temp[32];
  SdFile file; 
  int n = 0;
    
  sd.chdir("/", true);
  
  while (file.openNext(sd.vwd(), O_READ))
  {
    file.getName(filename, sizeof(filename));
    if (isFnMusic(filename))  
    {
      strcpy(musicList[n].filename, filename);

      // Activate track to find title and artist
      MP3player.playMP3(filename);
      MP3player.trackTitle(temp);
      strncpy(musicList[n].title, temp, 15);
      MP3player.trackArtist(temp);
      strncpy(musicList[n].artist, temp, 15);
      MP3player.stopTrack();

      if (++n >= MAX_MUSIC)
      {
        file.close();
        break;
      }
    }
    file.close();
  }

  return n;
}

//------------------------------------------------------------------------------
// This functions just displays a short pattern on the leds.

void ledsRun()
{
  int i;
  
  for (i = 0; i < 4; i++)
  {
    ledsWrite(1 << i);
    delay(100);
  }
  for (i = 2; i >= 0; i--)
  {
    ledsWrite(1 << i);
    delay(100);
  }
}

//------------------------------------------------------------------------------
// This function initializes the LCD. It uses the global variable lcdInterface
// which indicates wether the 4-bit of 8-bit initialization should be used. 

void lcdInit()
{
  if ((lcdInterface == 4) || (lcdInterface == 8))
  {
    delay(40);                 // wait more than 40ms
    lcdWrite(0, 0x30);
    delay(5);                  // wait more than 4.1ms
    lcdWrite(0, 0x30);
    delayMicroseconds(100);    // wait more than 100us
    lcdWrite(0, 0x30);
    delayMicroseconds(37);     // execution time
  }
  
  if (lcdInterface == 8)
  {
    lcdWrite(0, 0x38);       // 8 bits, 2 lines, 5x8 dots
    delayMicroseconds(37);   // execution time
    lcdWrite(0, 0x0C);       // display on
    delayMicroseconds(37);   // execution time
    lcdWrite(0, 0x01);       // display clear
    delayMicroseconds(1000); // undocumented delay
    lcdWrite(0, 0x06);       // increment, no shift
    delayMicroseconds(37);   // execution time
  }

  if (lcdInterface == 4)
  {
    lcdWrite(0, 0x20);       // 4 bits interface
    delayMicroseconds(37);   // execution time
    lcdWrite4(0, 0x28);      // 4 bits, 2 lines, 5x8 dots
    delayMicroseconds(37);   // execution time
    lcdWrite4(0, 0x0C);      // display on
    delayMicroseconds(37);   // execution time
    lcdWrite4(0, 0x01);      // display clear
    delayMicroseconds(1000); // undocumented delay
    lcdWrite4(0, 0x06);      // increment, no shift
    delayMicroseconds(37);   // execution time  
  }
}

//------------------------------------------------------------------------------
// This function uses the lcdWrite from the Interface sketch. When using 
// the 4-bit interface of the LCD, the upper nibble of parameter data 
// contains the significant data. 

void lcdWrite4(bool rs, byte data)
{
  lcdWrite(rs, data);
  lcdWrite(rs, data << 4);
}

//------------------------------------------------------------------------------
// This function is used by lcdClear and lcdWriteString. It uses the lcdWrite
// from the Interface sketch or the lcdWrite4-function.

void _lcdWrite(bool rs, byte data)
{
  if (lcdInterface == 8)
  {
    lcdWrite(rs, data);
  }
  else 
  {
    lcdWrite4(rs, data);
  }
}

//------------------------------------------------------------------------------
// This functions clears the LCD. It seems that a delay of approximately 
// 1000 us is necessary for this command to function properly. 

void lcdClear()
{
  _lcdWrite(0, 0x01);
  delayMicroseconds(1000);          // undocumented delay
}

//------------------------------------------------------------------------------
// This function writes a string to display on the LCD. Parameter 'line' can 
// be 0 (for the upper line) or 1 (for the lower line). Data will be displayed 
// from the beginning of the line. Existing data will be overwritten, but  
// nothing will be cleared.

void lcdWriteString(int line, const char *str)
{
  _lcdWrite(0, line ? 0xC0 : 0x80); // set DDRAM address to start of line  
  delayMicroseconds(37);            // execution time
    
  for (int i = 0; str[i] && (i < 16); i++)
  {
    _lcdWrite(1, str[i]);
    delayMicroseconds(37 + 4);      // execution time
  }
}

//------------------------------------------------------------------------------


