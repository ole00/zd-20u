/*
=====================================================================================
Moded USB Soldering iron ZD20U
=====================================================================================
WORK IN PROGRESS - use at your own risk!

Version 1.0
- uses Attiny13a MCU to set the heat on the soldering tip
- 3 heat modes: Full, Medium, Low and an Off/Sleep state.
- switch between the heat modes by pressing a button (not the flaky metal stud!)
- visual indication of the heat modes via LEDs:
  Full      : Red + Green
  Medium    : Red + Dimmed Green
  Low       : Red
  Off/Sleep : no LED is on
- heat modes/temperatures are user configurable via heat profile.  Lower value of
  the heat profile means lower temepratures for Medium and Low mode.
  TODO - measure the thermal values of the tip for each of the heat profile value
- timer to turn off the heat after configurable amount of seconds. 
- timeouts are user configurable via timeout profiles. Lower value of the 
  timeout profile means shorter time to switch to Off/Sleep mode.
  Value 1: 32 seconds, value 2: 64 seconds, value 3: 96 secs, value 4: 128 secs 
- LED indication (blinking) that the heat will be turned off shortly.
  During that time pressing the button snoozes the the heat-off timer.
  The bigger the timeout profile value the longer the indication time. 
  Value 1: 10 seconds, value 2: 12 seconds, value 3: 14 secs, value 4: 16 secs 
- sleep profile allows the iron to completely turn off and cool down (value 1)
  or to get asleep to draw some current every 18 seconds (value 2). Sleeping
  saves some energy while keeping the iron hot. It also tricks some 'smart' power 
  banks not to auto turn off. Default value of the sleep profile is 1 (full off).  
- enter profile selection / configuration  by holding the button pressed
  for 5 seconds right after the power-up
  RED blinks R times to indicate which profile is being set:
  * 1 RED blink -> the heat profile
  * 2 RED blinks -> the timeout profile
  * 3 RED blinks -> the sleep profile
  After the RED blink, the GREEN led starts to blink. The number of GREEN flashes
  indicates the profile value. After the GREEN flashes the iron expects the user
  to press the button to select the value she desires. If a button is not pressed
  within ~1 second the next value is presented (more GREEN blinks).
  Example:
   1) RED flashes once: heat profile is being configured
   2) user waits until GREEN flashes 3x
   3) user presses the button -> she now selected value 3 of the heat profile
   4) RED flashes twice: timeout profile is being configured
   5) user waits until GREEN flashes 2x
   6) user presses the button -> she now selected value 2 of the timeout profile
   7) RED flashes three times: sleep profile is being configured
   8) user waits until GREEN flashes 1x
   9) user presses the button -> she now selected value 1 of the sleep profile
   10) configuration is done and the iron goes to normal operation mode.

   **************** !!! WARNING  !!!  *****************
   NEVER ENTER THE CONFIGURATION/SETUP MODE WHEN 
   THE IRON IS HOT AS YOU MAY BURN YOURSELF WHILE
   FIDDLING WITH THE PROFILES
   ****************************************************
   
- current configuration profile is displayed during start-up.
  RED blinks X times to indicate current heat profile
  GREEN blinks Y times to indicate current timeout profile
  Sleep profile is not indicated.

=====================================================================================
NOTE: to keep correct timings ensure Attiny13a runs at 1.2MHz

NOTE: to fix linking error: multiple definition of `__vector_3'
edit hardware/attiny/avr/cores/core13/wiring.c and temporarily change
 ISR(TIM0_OVF_vect)
 to
 ISR(TIMER0_OVF_vect)

 after you recompile and upload the code to MCU you can change it back.
=====================================================================================
 License: MIT
 Copyright: 2017 olin
=====================================================================================

*/


#define MOSFET_G    4 /* PB 4 */
#define BUTTON      0 /* PB 0 */ 
#define LED         3 /* PB 3 */

#define RED         MOSFET_G
#define GREEN       LED

// OFF - no volatege is output on MOSFET gate - heating element disconnected
#define STATE_OFF 0

// ON - full voltage is output on MOSFET gate - heating element at full power
#define STATE_HI  1

// MID - sigma-delta based voltage is output on MOSFET gate - heating element at medium power
#define STATE_MID  2

// LOW - sigma-delta based voltage is output on MOSFET gate - heating element at low power
#define STATE_LOW  3

#define INVALID_STATE 4

//internal pull up resistor keeps the button pin high, when pressed the pin goes to 0
#define BUTTON_PRESSED 0
#define BUTTON_RELEASED 1

//button debouncing counter max value
#define BUTTON_FULLY_PRESSED 64

#define HEAT_PROFILE 1
#define TIMEOUT_PROFILE 2
#define SLEEP_PROFILE 3

#define SLEEP_WAKEUP_TIME 18
#define CONFIG_SLEEP_BIT (1 << 4)


//comment out ENABLE_USER_CONFIG to get extra 200 bytes
//but you'll lose ability to enter user configuration
//after start-up
#define ENABLE_USER_CONFIG

//when user config is disabled this is your iron configuration 
#ifndef ENABLE_USER_CONFIG
#define CONFIG_VALUE  (TIMEOUT_PROFILE << 2) | HEAT_PROFILE
#endif /* ENABLE_USER_CONFIG */

// set/unset pins on port B
#define PIN_ON(X) PORTB |= (1 << X)
#define PIN_OFF(X) PORTB &= ~(1 << X)
#define PIN_TOGGLE(X) PORTB ^= (1 << X)
#define PIN_READ(X) (!!(PINB & (1 << X)))
#define PINMODE_INPUT(X) DDRB &= ~(1 << X) 
#define PINMODE_OUTPUT(X) DDRB |= (1 << X)
// pull up works only on input pins by writing 1 into the port on the specific pin
#define PINMODE_PULLUP(X) PORTB |= (1 << X)

unsigned int secondsCounter = 0;       //counter of seconds since the button was pressed
volatile unsigned char isrCounter = 0; //counter to calculate seconds via ISR

//function declaration
void appStart(unsigned char configData);

//condensed EEPROM writing and reading macros to fit into Attiny13a flash space

#define EEPROM_WRITE(X) \
  while(EECR & (1<<EEPE)); /* Wait for completion of previous write */ \
  EECR = EEAR = 0;    /* Set Programming mode - clear EEPM1 & EEPM0 bits, writing address set to 0 */ \
  EEDR = X;           /* set the data to be written */ \
  EECR |= (1<<EEMPE); /* Write logical one to EEMPE */ \
  EECR |= (1<<EEPE);  /* Start eeprom write by setting EEPE */

#define EEPROM_READ(X) \
  while(EECR & (1<<EEPE)); /* Wait for completion of previous write */ \
  EEAR = 0;           /* Set up address register */ \
  EECR |= (1<<EERE);  /* Start eeprom read by writing EERE */ \
  X = EEDR;           /* Copy the result of the reading into destination variable */



static void shortDelay(unsigned char tim) {
  while (tim--) {
    unsigned char t = 255;
    while (t--) {
      asm("nop");
      asm("nop");
      asm("nop");
    }
  }
}

static void blinkLed(unsigned char pin, unsigned char cnt) {
  PIN_OFF(pin);
  shortDelay(100);
  while (cnt--) {
    PIN_ON(pin);
    shortDelay(80);
    PIN_OFF(pin);
    shortDelay(200);
  }
}


static void waitForButtonReleased() {
  unsigned char bRead;
  //wait until the button is released
  while (1) {
    bRead = PIN_READ(BUTTON);
    //button released
    if (bRead == BUTTON_RELEASED) {
      return;
    }
    shortDelay(100);
  }
}


static unsigned char checkButtonPressed(unsigned char timeOut) {
  unsigned char bRead;
  //wait until the button is pressed
  while (timeOut--) {
    bRead = PIN_READ(BUTTON);
    //button pressed
    if (bRead == BUTTON_PRESSED) {
      return 1;
    }
    shortDelay(50);
  }
  return 0;
}

// visual value selector that:
// - blinks the RED X-times led to show which variable is being set (the index)
// - cycles and blinks the GREEN led to show values (from 1 to maxVal)
// - to select particular value user has to press button after the GREEN led
//   shows the value
unsigned char selectValue(unsigned char index, unsigned char maxVal) {
  unsigned char maxTimes = 12; //try 12 times to let user to select a value
  while(maxTimes--) {
    unsigned char t = 0;
    //blink the RED led X times to signify which type/index is being selected
    blinkLed(RED, index);
    shortDelay(250);
    waitForButtonReleased();
    
    //blink GEEN led and cycle all values
    while (++t <= maxVal) {
      blinkLed(GREEN, t);
      if (checkButtonPressed(20)) {
        waitForButtonReleased();
        return t - 1;
      }
    }
  }
  return 0xFF; // timed out - user doesn't care 
}

unsigned char configureIron(void) {
  unsigned char val;
  unsigned char t = 10;

  shortDelay(250);
  shortDelay(250);
  
  //check the button is pressed for ~5 sconds
  while (t--) {
    val = PIN_READ(BUTTON);
    //button released (or was never pressed) -> exit the configuration without saving
    if (val == BUTTON_RELEASED) {
        return 0xFF;
    }
    shortDelay(250);
  }
  
  waitForButtonReleased();

  //select the first value -> heat profile
  val = selectValue(1, 4);
  //selection timed out -> early exit without saving
  if (val == 0xFF) {
    return val;
  }
  t = val;

  //select second value -> switch off timeouts
  val = selectValue(2, 4);
  //selection timed out -> early exit without saving
  if (val == 0xFF) {
    return val;
  } 
  t |= (val << 2);
  
  //select third value -> sleep mode (1 - off / 2 - on)
  val = selectValue(3, 2);
  //selection timed out -> early exit without saving
  if (val == 0xFF) {
    return val;
  } 
  t |= (val << 4);
  
  //save the configuration
  EEPROM_WRITE(t);
  return t;

}


//interrupt routine triggered via the timer, as explained here:
//https://arduinodiy.wordpress.com/2015/06/22/flashing-an-led-with-attiny13/
ISR(TIM0_OVF_vect) {
  isrCounter++;
  // interrupt occurs 73.2421 times per second, that is ~ 73 + 1/4
  if (isrCounter > 74) {
    //once every 4 iterations make the wait longer
    if (isrCounter & 0x3 == 0) {
      isrCounter = 0;
    } else {
      isrCounter = 1;
    }
    secondsCounter++;
   }
}

// the setup function runs once when you press reset or power the board
void setup() {
  unsigned char configData;

  cli(); //clear interrupts
  
  // initialize MOSFET_GATE pin 3 (PB4) as output
  PINMODE_OUTPUT(MOSFET_G);
  // initialize LED pin 2 (PB3) as output
  PINMODE_OUTPUT(LED);
  // initialize BUTTON pin 5 (PB0) as input, pulled up
  PINMODE_INPUT(BUTTON);
  PINMODE_PULLUP(BUTTON);

  // Configuration of the timer to be able to measure elapsed seconds.
  // set prescale timer to 1/64th. At 1.2MHz the ISR call rate is 73.2421 time / sec.
  TCCR0B |= (1 << CS01) | (1 << CS00);// set CS01 and CS00 bit in TCCR0B
  TCCR0B &= ~(1 << CS02); // clear CS02 bit from TCCR0B

  // enable timer overflow interrupt
  TIMSK0 |= 1 << TOIE0;// set TOIE0 bit in TIMSK

#ifdef ENABLE_USER_CONFIG
  //check whether button is pressed for 5 seconds and if so, enter the configuration mode
  configData = configureIron();
  // button was not pressed -> read the config data from EEPROM 
  if (configData == 0xFF) {
    EEPROM_READ(configData);
    //config has not been set yet 
    if (configData == 0xFF) {
      configData = 0; //use the default profile
    }
  }
#else
 configData = CONFIG_VALUE;
#endif

  sei(); //start the timer

  appStart(configData); //start the main iron heating code
}


//this function is never reached
void loop() {};


//controll the LED status
static void led(unsigned char state, unsigned char autoOffStart) {
  if (secondsCounter <= autoOffStart) {
    if (state) {
      PIN_ON(LED);
    } else {
      PIN_OFF(LED);
    }
  }
}

//controll the MOSFET gate
static void mosfet(unsigned char state) {
    if (state) {
      PIN_ON(MOSFET_G);
    } else {
      PIN_OFF(MOSFET_G);
    }  
}

// Main iron heat-controlling function. This runs forever until you unplug the power.
// Note: the standard 'loop()' mechanism is not used to save code size (saved 160 bytes!)
// and also to improve CPU timings. The optimisation is achieved by using local variables.
void appStart(const unsigned char configData) {
  
  unsigned char state = STATE_OFF; //Always start with heat turned OFF for safety reasons
  unsigned char bRead;
  unsigned char blinkStatus = 0;
  
  unsigned char sdVal = 0;        //sigma delta value
  unsigned char button = 0;       //button value

  //configurable variables
  unsigned char autoOffStart;
  unsigned char autoOffEnd;
  // sigma delta increments for MID and LOW states ( in range of 0 - 127)
  unsigned char midSd;
  unsigned char lowSd;

  //======================================================
  // setup the heat and timeout variables 
  // according to the user config
  // +---+---+---+---+---+---+---+---+
  // | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 
  // +---+---+---+---+---+---+---+---+
  // | not used  |slp| time  | heat  |  
  // +-----------+---+-------+-------+ 
  //======================================================



  //set heat values
  midSd = configData & 0x3;
  //blink RED led to show current heat profile
  blinkLed(RED, midSd + 1);
  
  // 90, 98, 106, 114  sigma delta values
  midSd = 90 + (midSd << 3);
  // 60, 68, 76,  84  sigma delta values
  lowSd = midSd - 30; 

  //set timeout values
  autoOffStart = (configData >> 2) & 0x3;
  //blink GREEN led to show current timeout profile
  blinkLed(GREEN, autoOffStart + 1);
  
  // 32, 64, 96, 128 seconds 
  autoOffStart = (autoOffStart + 1) << 5;
  
  // 42, 76, 110, 144 seconds
  autoOffEnd =  autoOffStart + 10 + (((configData >> 2) & 0x3)  << 1);
  
  //endless loop...
  while(1) {
  //======================================================
  // handle output to MOSFET gate - controls heat
  //======================================================
  if (state == STATE_OFF) {
    PIN_OFF(MOSFET_G); //mosfet gate turned off
    // Sleep mode is enabled during OFF time (while not heating)
    // but only if user has configured it.
    if (configData & CONFIG_SLEEP_BIT) {
    	//it's time to wake up....
    	if (secondsCounter == SLEEP_WAKEUP_TIME) {
    		secondsCounter = 0;
    		// Turn MOSFET on for a short time.
    		// That will tickle the battery to keep supplying the iron.
    		PIN_ON(MOSFET_G);
    		shortDelay(250);
    		shortDelay(250);
    		shortDelay(250);
    		PIN_OFF(MOSFET_G);
    	}
    } else {
    	secondsCounter = 0;  //keep the seconds timer reset until we turn on the heat
    }
    //LED is already off (set in the button handler)
  } else
  if (state == STATE_HI) {
    PIN_ON(MOSFET_G); //mosfet gate ruturned on
    //LED is already on (set in the button handler)
  } else
  if (state == STATE_MID) {
    register unsigned char val;
    sdVal += midSd; //increment the sigma-delta value by the MID increment
    val = sdVal >> 7;
    mosfet(val); //either on or off depending on the top bit of 7bit sigma delta
    led(!val, autoOffStart);  //blink the LED with low frequency
    sdVal &= 0x7F;
  }
  //STATE_LOW
  else {
    sdVal += lowSd; //increment the sigma-delta value by the LOW increment
    mosfet(sdVal >> 7); //either on or off depending on the top bit of 7bit sigma delta
    sdVal &= 0x7F;
    //LED is already off (set in the button handler)
  }

  //======================================================
  // handle button press / release
  //======================================================
  bRead = PIN_READ(BUTTON);
  //button released
  if (bRead == BUTTON_RELEASED) {
    //button just changed state from fully pressed to released
    if (button == BUTTON_FULLY_PRESSED) {
      button = 0;
      //handle press during normal work  time (led is NOT blinking)
      if (secondsCounter <= autoOffStart) {
        state++; //jump to next state
        if (state == INVALID_STATE) {
          state = 0;
        }
      }
      //reset the seconds counter
      secondsCounter = 0;
      isrCounter = 0;
      //set the LED according to the current state
      if (state == STATE_HI) {
        PIN_ON(LED);
      } else {
        PIN_OFF(LED);
      }
    }
  }
  //button pressed 
  else
  //button debouncing: button is considered pressed once the button value accumulates to fully-pressed value
  if (button < BUTTON_FULLY_PRESSED){
    button++; //not fully pressed yet -> increase the button value
  }

  //======================================================
  // handle the timings
  //======================================================
  //check the time for auto off
  if (secondsCounter == autoOffEnd) {
      state = STATE_OFF;
      PIN_OFF(LED);
      isrCounter = 0;
      secondsCounter = 0;
  }
  else
  //blink the LED every second to notify the iron is going to be turned off shortly
  if (secondsCounter >= autoOffStart) {
      if (isrCounter == 5 || isrCounter == 25) {
        if (!blinkStatus) {
          PIN_TOGGLE(LED);
          blinkStatus = 1;
        }
      } else {
        blinkStatus = 0;
      }
   }
   
   } //end while
}
