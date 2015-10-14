#include <IRremote.h> /* Include the IRremote library */

#define R_PIN     5   /* Define Pin used for the Red Led anode */
#define G_PIN     6   /* Define Pin used for the Green Led anode */
#define B_PIN     9   /* Define Pin used for the Blue Led anode */
#define RECV_PIN  10  /* Define Pin used for the IR Receiver Data */

/* All Hex codes from the IR Remote Control LED618 */
#define INC_BRIGTH     0xFFA05F  /* + */
#define DEC_BRIGTH     0xFF20DF  /* - */
#define ON             0xFFE01F  /* Power */
#define OFF            0xFF609F  /* Off */
#define FADE           0xFFD827  /* Fade */
#define FLASH          0xFFF00F  /* Flash */
#define SMOOTH         0xFFC837  /* Smooth */
#define WHITE_BTN      0xFFD02F  /* White */
#define RED_BTN        0xFF906F  /* Red */
#define ORANGE_BTN     0xFFB04F  /* Orange */
#define TOMATO_BTN     0xFFA857  /* Tomato */
#define LSALMON_BTN    0xFF9867  /* Light Salmon */
#define YELLOW_BTN     0xFF8877  /* Yellow */
#define GREEN_BTN      0xFF10EF  /* Green */
#define LGREEN_BTN     0xFF30CF  /* Lawn Green */
#define CYAN_BTN       0xFF28D7  /* Cyan */
#define MSGREEN_BTN    0xFF18E7  /* Medium Spring Green */
#define TEAL_BTN       0xFF08F7  /* Teal */
#define BLUE_BTN       0xFF50AF  /* Blue */
#define DODGEBLUE_BTN  0xFF708F  /* Dodge Blue */
#define PURPLE_BTN     0xFF6897  /* Purple */
#define INDIGO_BTN     0xFF58A7  /* Indigo */
#define REBPURPLE_BTN  0xFF48B7  /* Rebecca purple */

#define BLACK      0  /* Black */
#define WHITE      1  /* White */
#define RED        2  /* Red */
#define ORANGE     3  /* Orange */
#define TOMATO     4  /* Tomato */
#define LSALMON    5  /* Light Salmon */
#define YELLOW     6  /* Yellow */
#define GREEN      7  /* Green */
#define LGREEN     8  /* Lawn Green */
#define CYAN       9  /* Cyan */
#define MSGREEN    10  /* Medium Spring Green */
#define TEAL       11  /* Teal */
#define BLUE       12  /* Blue */
#define DODGEBLUE  13  /* Dodge Blue */
#define PURPLE     14  /* Purple */
#define INDIGO     15  /* Indigo */
#define REBPURPLE  16  /* Rebecca purple */


/* ------ Global Variables ----------------------------------------------------------------------- */
short    activeColor = BLACK; /* Stores the current led color, in range [0,16] */
short    lastColor   = BLACK; /* Stores the last active led color, due to color variable */
short    bright      = 0; /* Keeps track of the current led brightness, in range [0,5] */
boolean  doFade      = 0; /* Stores the power state of the led, { 0 --> Off, 1 --> On } */
boolean  doStrobe    = 0; /* Stores the power state of the led, { 0 --> Off, 1 --> On } */
boolean  doSmooth    = 0; /* Stores the power state of the led, { 0 --> Off, 1 --> On } */
boolean  doFlash     = 0; /* Stores the power state of the led, { 0 --> Off, 1 --> On } */
boolean  powerState  = 1; /* Stores the power state of the led, { 0 --> Off, 1 --> On } */

int16_t red   = 0x00;
int16_t green = 0x00; 
int16_t blue  = 0x00;
int counter = 0;
int64_t activeHexColor;
int64_t lastHexColor;

unsigned long testc;

unsigned long interval        = 0;
unsigned long previousMillis  = 0;       

const    long fade_interval   = 3000;    /* Interval (ms) to fade the led */
const    long flash_interval  = 1000;    /* Interval (ms) to flash the led */
const    long smooth_interval = 100;     /* Interval (ms) to smooth the led*/

/* Lookup table of RGB color values, all colors have 5 hardcoded brightness levels */
const long  colors[17][5] = { 
                                {0x000000, 0x000000, 0x000000, 0x000000, 0x000000}, /* Black */
                                {0xFFFFFF, 0xC3C3C3, 0x878787, 0x4B4B4B, 0x0F0F0F}, /* White */
                                {0xFF0000, 0xC30000, 0x870000, 0x4B0000, 0x0F0000}, /* Red */
                                {0xFF4500, 0x806700, 0x874E00, 0x3C3500, 0x0F1C00}, /* Orange */
                                {0xFF8C00, 0x806700, 0x874E00, 0x3C3500, 0x0F1C00}, /* Tomato */
                                {0xFFA07A, 0x808266, 0x876452, 0x3C463E, 0x0F282A}, /* Light Salmon */
                                {0xFFFF00, 0xC3C300, 0x878700, 0x4B4B00, 0x0F0F00}, /* Yellow */
                                {0x00FF00, 0x00C300, 0x008700, 0x004B00, 0x000F00}, /* Green */
                                {0x7CFF00, 0x68C300, 0x548700, 0x40FC00, 0x2CFC00}, /* Lawn Green */
                                {0x00FFFF, 0x00C3C3, 0x008787, 0x004B4B, 0x000F0F}, /* Cyan */
                                {0x00FA7C, 0x00C868, 0x009654, 0x006440, 0x00322C}, /* Medium Spring Green */
                                {0x008080, 0x006767, 0x004E4E, 0x003535, 0x001C1C}, /* Teal  */
                                {0x0000FF, 0x0000C3, 0x000087, 0x00004B, 0x00000F}, /* Blue */
                                {0x1E90FF, 0x1972C3, 0x145487, 0x0F364B, 0x0A240F}, /* Dodge Blue */
                                {0x800080, 0x670067, 0x4E004E, 0x350035, 0x1C001C}, /* Purple */
                                {0x4B0080, 0x410067, 0x37004E, 0x2D0035, 0x24001C}, /* Indigo */
                                {0x663399, 0x663399, 0x663399, 0x663399, 0x663399}, /* Rebecca purple */
                             };   


IRrecv irrecv( RECV_PIN ); /* Initialize the irrecv part of the IRremote  library */
decode_results ir_results; /* Stores IR received codes  */

void setup()
{
  irrecv.enableIRIn();   /* Start the IR receiver */
  Serial.begin( 9600 );  /* Start Serial Communication for debug */
}

/* Write the RGB value of the currently active color */
void setRGB ( unsigned long inputColor ) {
  /* Write the RGB value's to output respectivly */
  analogWrite( R_PIN, inputColor >> 16 );
  analogWrite( G_PIN, ( inputColor & 0xFF00  ) >> 8  );
  analogWrite( B_PIN, inputColor & 0xFF );  
}

void setBrightness ( boolean direct ) {
  if ( powerState && !doFade && !doFlash && !doSmooth ) { /* Led's state is active and not in Flash,Flade or Smooth mode */
    if ( direct && bright != 0 ) bright--;                /* Decrease brightness if possible */
    else if ( !direct && bright != 4 ) bright++;          /* Increase brightness if possible */ 
    setColor( activeColor );                              /* Update led */
  }
}

void setColor ( short inColor ) {
  /* If system is in Flash,Fade or Smooth mode disable them */
  if ( doFade ) doFade = 0;
  if ( doFlash ) doFlash = 0;
  if ( doSmooth ) doSmooth = 0;
  if ( powerState ) /* Verify that Led's state is active, then update led's RGB color, keep a backup of it's HEX value and ...*/
    setRGB( activeHexColor = colors[ activeColor = inColor ][bright] );
}

void setOnOff ( boolean input ) {
  if ( powerState && !input ) {     /* Led is active and Off is pressed */
    lastColor = activeColor;        /* Backup current led's color from table */
    lastHexColor = activeHexColor;  /* Backup current led's color HEX format */
    setColor( BLACK );              /* Set led Off */
    powerState = !powerState;       /* Update state */
  }
  else if ( !powerState && input ){
    powerState = !powerState;
    activeColor = lastColor;
    activeHexColor = lastHexColor;
    setColor( activeColor );
  }
}
/* Loop constantly, check for any received IR code and handle it. */

void loop() {
  
  if ( irrecv.decode( &ir_results ) ) { /* Ir signal detected */
    Serial.println( ir_results.value, HEX ); /* Use for debug */
    switch ( ir_results.value ) {
      case ON:             setOnOff( 1 ); break;
      case OFF:            setOnOff( 0 ); break;
      case WHITE_BTN:      setColor( WHITE ); break;
      case RED_BTN :       setColor( RED ); break;
      case ORANGE_BTN:     setColor( ORANGE ); break;
      case TOMATO_BTN:     setColor( TOMATO ); break;
      case LSALMON_BTN:    setColor( LSALMON ); break;
      case YELLOW_BTN:     setColor( YELLOW ); break;
      case GREEN_BTN:      setColor( GREEN ); break;
      case LGREEN_BTN:     setColor( LGREEN ); break;
      case CYAN_BTN:       setColor( CYAN ); break;
      case MSGREEN_BTN:    setColor( MSGREEN ); break;
      case TEAL_BTN:       setColor( TEAL ); break;
      case BLUE_BTN:       setColor( BLUE ); break;
      case DODGEBLUE_BTN:  setColor( DODGEBLUE ); break;
      case PURPLE_BTN:     setColor( PURPLE ); break;
      case INDIGO_BTN:     setColor( INDIGO ); break;
      case REBPURPLE_BTN:  setColor( REBPURPLE ); break;
      case INC_BRIGTH:     setBrightness( 1 ); break;
      case DEC_BRIGTH:     setBrightness( 0 ); break;
      case FADE:           if ( !doFlash && !doSmooth ) doFade = !doFade; break;
      case FLASH:          if ( !doFade && !doSmooth )  doFlash = !doFlash; break;
      case SMOOTH:         if ( !doFade && !doFlash )   doSmooth = !doSmooth; counter = 0; break;
      //case STROBE: doStrobe = !doStrobe; break;
    }
    irrecv.resume(); /* Receive the next value */ 
  }
  if ( doFade || doFlash ) {
    unsigned long currentMillis = millis();
    interval = ( doFade ) ? fade_interval : flash_interval;
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis; /* Save the last time the color changed */
      ( activeColor !=  REBPURPLE ) ? ( activeColor++ ) : ( activeColor = WHITE );
      setRGB( activeHexColor = colors[ activeColor ][bright] );
    }
  }
 if ( doSmooth ) {
    unsigned long currentMillis = millis();
    interval = smooth_interval;
    if ( currentMillis - previousMillis >= interval ) {
      previousMillis = currentMillis; /* Save the last time the color changed */
      counter = ( counter >= 3*0xFF ) ? 0 : counter+=5;
      activeHexColor = ( (  ( ( counter <= 0xFF ) ? 255-counter : ( counter > 2*255 ) ? counter-2*255 : 0 ) & 0xFFFFFF ) << 16 ) | /* Red */
                       ( ( ( ( counter <= 0xFF ) ? counter : ( counter <= 2*255 ) ? 2*255-counter : 0 ) & 0xFFFFFF ) << 8 ) | /* Green */
                       ( ( counter > 0xFF  ) ? ( counter <= 2*255 ) ? counter-255 : 3*255-counter : 0 );  /* Blue */
      setRGB( activeHexColor );
    }
  }

}