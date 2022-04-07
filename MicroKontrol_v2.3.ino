//MIDI
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();
int modesetup;
int MIDICH = 1;
int CCPARAM;
int input_expression_actual; // Analog pin 
int input_sustain_actual; // Digital pin 7

int input_expression_ultimo;
int input_sustain_ultimo;

//EPROM write n read
#include <EEPROM.h>
int EEPROM_address_MIDICH = 0;
int EEPROM_address_CCPARAM = 1;

//time evaluation
int current;         // Current state of the button (LOW is pressed - pullup resistors)
long millis_held;    // How long the button was held (milliseconds)
long secs_held;      // How long the button was held (seconds)
long prev_secs_held; // How long the button was held in the previous check
byte previous = HIGH;
unsigned long firstTime; // how long since the button was first pressed


//Blink without delay
// Variables will change:
int ledState = LOW;             // ledState used to set the LED
int falseortrue;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 200;           // interval at which to blink (milliseconds)

//buttons
const int button_up_pin = 4;// (pin) // 
const int button_down_pin = 6; // (pin) // 
int button_up_state;
int button_down_state;
int button_up_laststate = HIGH;
int button_down_laststate = HIGH;

//sevensegments
#include <Arduino.h>
#include <TM1637Display.h>
#define CLK 11
#define DIO 8
TM1637Display display(CLK, DIO);

 
void setup() {  
  MIDI.begin(EEPROM_address_MIDICH);              // desactivar si usar serial print en 9600
  digitalWrite(button_up_pin, HIGH);                      // Turn on 20k pullup resistors to simplify switch input
  digitalWrite(button_down_pin, HIGH);                      // Turn on 20k pullup resistors to simplify switch input

  digitalWrite(input_sustain_actual, HIGH);              // Turn on 20k pullup resistors to simplify switch input

  MIDICH = EEPROM.read(EEPROM_address_MIDICH);    // leer MIDICH from EEPROM
  CCPARAM = EEPROM.read(EEPROM_address_CCPARAM);  // leer CCPARAM from EEPROM
  MIDICH = 1;
  CCPARAM = 1;
  //seven segments
  display.setBrightness(7, true); // Turn on
  const uint8_t SEG_HOLA[] = {
  SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,            // H
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,    // O
  SEG_D | SEG_E | SEG_F,                            // L
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G     // A
  };
  display.setSegments(SEG_HOLA);

  delay(1000);

  
}

void loop() {

  input_sustain_actual = map(digitalRead(2), HIGH, LOW, 0, 127);
  
  input_expression_actual = map(analogRead(A0), 250, 1023, 0, 127);
  //if (input_expression_actual<0){input_expression_actual=0;}
  
  MIDI.sendControlChange(64, input_sustain_actual, MIDICH);
  
  MIDI.sendControlChange(CCPARAM, input_expression_actual, MIDICH);


  //Change for CCPARAM
 button_up_state = digitalRead(button_up_pin);
 button_down_state= digitalRead(button_down_pin);
  if (button_up_state != button_up_laststate)
  {
    CCPARAM++;
  }

  if (button_down_state != button_down_laststate)
  {
    CCPARAM--;
  }

  //debug para mantener entre 1 - 127 y 1-16
  if (CCPARAM >= 128)
  {
    CCPARAM = 1;
  }
  if (CCPARAM <= 0)
  {
    CCPARAM = 127;
  }


  // screen print CCPARAM
  display.showNumberDec(CCPARAM, false); // Expect: d1__

  // **** HOLD BUTTON MODE *****
  // reads button to analize in the next wuan
  current = digitalRead(button_down_pin);

  // if the button state changes to pressed, remember the start time
  if (current == LOW && previous == HIGH && (millis() - firstTime) > 200) {
    firstTime = millis();
  }

  millis_held = (millis() - firstTime);
  secs_held = millis_held / 1000;

  // This if statement is a basic debouncing tool, the button must be pushed for at least
  // 100 milliseconds in a row for it to be considered as a push.
  if (millis_held > 100) {

    // Button held for 1 seconds, print out some info
    if (millis_held >= 1000 && current ==LOW) {
      modesetup = HIGH;
      
      while (current == LOW && modesetup == HIGH)
      {
        display.setBrightness(7, falseortrue);
        current = digitalRead(button_down_pin);
        button_up_state = digitalRead(button_up_pin);

       // MIDI Change
        if (button_up_state != button_up_laststate)
        {
          MIDICH++;
        }

        //if (button_down_state == LOW)
        //{
        //  MIDICH--;
        //}
        
        //Debug para constrain 1-16
        if (MIDICH == 17)
        {
          MIDICH = 1;
        }
        if (MIDICH == 0)
        {
          MIDICH = 16;
        }


        // screen prints
        display.showNumberDec(MIDICH, false); // MIDI CH VALUE
        display.showNumberHexEx(0xcd, 0, false, 2); // print "CH:"
        button_up_state = button_up_laststate;

        //Blink without Delay
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
          // save the last time you blinked the LED
          previousMillis = currentMillis;

          // if the LED is off turn it on and vice-versa:
          if (falseortrue == false) {
            falseortrue = true;
          } else {
            falseortrue = false;
          }
        }
      }

      // ****HOLD BUTTON MODE END****
      
      //modesetup = LOW;
    display.setBrightness(7, true); //cuando vuelve del else prende la pantalla por las dudas
    }
  }

  previous = current;
  prev_secs_held = secs_held;
  delay(100);
  button_up_state = button_up_laststate;
  button_down_state = button_down_laststate;
  //EEPROM.update(EEPROM_address_MIDICH, MIDICH);
  //EEPROM.update(EEPROM_address_CCPARAM, CCPARAM);
  
  
}
