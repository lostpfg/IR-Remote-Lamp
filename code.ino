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
#define STROBE         0xFFE817  /* Strobe */
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
float    bright      = 5; /* Keeps track of the current led brightness, in range [1,5] */
boolean  doFade      = 0; /* Stores the power state of the led, { 0 --> Off, 1 --> On } */
boolean  doStrobe    = 0; /* Stores the power state of the led, { 0 --> Off, 1 --> On } */
boolean  doSmooth    = 0; /* Stores the power state of the led, { 0 --> Off, 1 --> On } */
boolean  doFlash     = 0; /* Stores the power state of the led, { 0 --> Off, 1 --> On } */
boolean  powerState  = 1; /* Stores the power state of the led, { 0 --> Off, 1 --> On } */

int16_t red   = 0x00;
int16_t green = 0x00; 
int16_t blue  = 0x00;
int  counter = 0;
long activeHexColor;
long activeBrightColor;
long lastHexColor;

unsigned long interval        = 0;
unsigned long previousMillis  = 0;       

const    long flash_interval   = 1000; /* Interval (ms) to flash the led */
const    long strobe_interval  = 500; /* Interval (ms) to flash the led */
const    long fade_interval    = 50;   /* Interval (ms) to fade the led */
const    long smooth_interval  = 200;  /* Interval (ms) to smooth the led*/

/* Lookup table of RGB color values, all colors have 5 hardcoded brightness levels */
const long  colors[17] = { 
                              0x000000, /* Black */
                              0xFFFFFF, /* White */
                              0xFF0000, /* Red */
                              0xFF4500, /* Orange */
                              0xFF8C00, /* Tomato */
                              0xFFA07A, /* Light Salmon */
                              0xFFFF00, /* Yellow */
                              0x00FF00, /* Green */
                              0x7CFF00, /* Lawn Green */
                              0x00FFFF, /* Cyan */
                              0x00FA7C, /* Medium Spring Green */
                              0x008080, /* Teal  */
                              0x0000FF, /* Blue */
                              0x1E90FF, /* Dodge Blue */
                              0x800080, /* Purple */
                              0x4B0080, /* Indigo */
                              0x663399  /* Rebecca purple */
                           };   


IRrecv irrecv( RECV_PIN ); /* Initialize the irrecv part of the IRremote  library */
decode_results ir_results; /* Stores IR received codes  */

void setup()
{
  irrecv.enableIRIn();   /* Start the IR receiver. */
  Serial.begin( 9600 );  /* Start Serial Communication for debug. */
}

void setRGB ( unsigned long inputColor ) {
  /* Write the RGB value's to output respectivly. */
  analogWrite( R_PIN, inputColor >> 16 );
  analogWrite( G_PIN, ( inputColor & 0xFF00  ) >> 8  );
  analogWrite( B_PIN, inputColor & 0xFF );  
}

void setBrightness ( boolean enb, boolean direct ) {
  if ( powerState ) { /* Check if led's state is active and system is currently not in Flash,Flade or Smooth mode. */
    if ( enb && !direct && bright != 1 ) bright--;                /* Decrease brightness if possible. */
    else if ( enb && direct && bright != 5 ) bright++;          /* Increase brightness if possible. */ 
      red =  ( activeHexColor >> 16 )*( (bright*20)/100 );
      green =  ( ( activeHexColor >> 8  ) & 0x00FF )*( (bright*20)/100 );
      blue =   ( activeHexColor & 0xFF )*( (bright*20)/100 );
      activeBrightColor = ( ( red& 0xFFFFFF) << 16 ) | ((green & 0xFFFFFF) << 8 ) | (blue);
      setRGB( activeBrightColor );
  }
}

void setColor ( short inColor ) {
  /* If system is currently in Flash,Fade or Smooth mode disable it. */
  if ( doFade ) doFade = 0;
  if ( doFlash ) doFlash = 0;
  if ( doSmooth ) doSmooth = 0;
  if ( powerState ) { /* Verify that Led's state is active, then update led's RGB color, keep a backup of it's HEX value and ...*/
    activeHexColor = colors[ activeColor = inColor ];
    setBrightness( 0, 0 );
  }
}

void setOnOff ( boolean input ) {
  if ( powerState && !input ) {     /* Led is active and Off is pressed */
    lastHexColor = activeHexColor;  /* Backup current led's color HEX format */
    setColor( BLACK );              /* Set led Off */
    powerState = !powerState;       /* Update state */
  }
  else if ( !powerState && input ){
    powerState = !powerState;
    activeHexColor = lastHexColor;
    setBrightness( 0, 0 );
  }
}

/* Loop constantly and check for any received IR code and handle it. */

void loop() {
  
  if ( irrecv.decode( &ir_results ) ) { /* An IR signal detected */
    Serial.println( ir_results.value, HEX ); /* Use for debug */
    switch ( ir_results.value ) { /* Switch between modes */
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
      case INC_BRIGTH:     setBrightness( 1, 1 ); break;
      case DEC_BRIGTH:     setBrightness( 1, 0 ); break;
      case FADE:           if ( !doFlash && !doSmooth && !doStrobe ) { doFade = !doFade; counter = 0; } break;
      case FLASH:          if ( !doFade && !doSmooth && !doStrobe )  doFlash = !doFlash; break;
      case SMOOTH:         if ( !doFade && !doFlash && !doStrobe )   { doSmooth = !doSmooth; counter = 0; } break;
      case STROBE:         if ( !doFade && !doFlash && !doSmooth ) doStrobe = !doStrobe; break;
    }
    irrecv.resume(); /* Receive the next IR value */ 
  }
  if ( doStrobe || doFlash ) {
    unsigned long currentMillis = millis();
    interval = ( doFlash ) ? flash_interval : strobe_interval;
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis; /* Save the last time the color changed */
      ( activeColor !=  REBPURPLE ) ? ( activeColor++ ) : ( activeColor = WHITE );
      activeHexColor = colors[ activeColor ];
      setBrightness(0,0);
    }
  }
 if ( doSmooth || doFade ) {
    unsigned long currentMillis = millis();
    interval = ( doSmooth ) ? smooth_interval : fade_interval;
    if ( currentMillis - previousMillis >= interval ) {
      previousMillis = currentMillis; /* Save the last time the color changed */
      counter = ( counter >= 3*0xFF ) ? 0 : counter+=5;
      activeHexColor = ( (  ( ( counter <= 0xFF ) ? 255-counter : ( counter > 2*255 ) ? counter-2*255 : 0 ) & 0xFFFFFF ) << 16 ) | /* Red */
                       ( ( ( ( counter <= 0xFF ) ? counter : ( counter <= 2*255 ) ? 2*255-counter : 0 ) & 0xFFFFFF ) << 8 ) | /* Green */
                       ( ( counter > 0xFF  ) ? ( counter <= 2*255 ) ? counter-255 : 3*255-counter : 0 );  /* Blue */
      setBrightness(0,0);
    }
  }

}