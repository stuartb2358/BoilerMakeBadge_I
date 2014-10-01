// Main bulk program for ATmega32u4
//
// PROTOTYPE BOARD HOT FIXES
// The following are a list of hot fixes that will need to be made on the first prototype board and fixed in later revisions
//
// Cut trace from ATmega-PD5 to nRF-CE. Reason: Thomas is a noob.
// Fly-wire from ATmega-PB5 to nRF-CE. Reason: Thomas is a noob.
// Cut trace from ATmega-SS to nRF-CSN. Reason: Thomas is a noob.
// Fly-wire from ATmega-PB6 to nRF-CSN. Reason: Thomas is a noob.


#include "nRF24L01.h" // nRF24L01 libraries
#include "RF24.h"
#include "RF24Network.h"
#include "SPI.h" // required SPI library (included in arduino IDE)

int SR_SS=3; // using digital pin 3 for the shift-register SPI slace select

//
// nRF24L01 radio static initialization
//

// Set up nRF25L01 on SPI bus plus pins 9 & 10 as CE & CSN, respectively
RF24 radio(9,10);

// Network uses that radio
RF24Network network(radio);

// Radio address for this node
const uint16_t this_node = 1;

// Radio address for other nodes
const uint16_t other_node = 0;

// Payload structure
struct payload_t
{
  int led_pattern;
};

void setup()
{
  // Setup and configure SPI
  pinMode(SR_SS, OUTPUT); // set SS digital pin as output
  SPI.begin(); // wake up the SPI bus
  SPI.setBitOrder(MSBFIRST); // arbitrarily using MSB (most significant bit) first
  SPI.setClockDivider(2); // set SPI clock to 8MHz / 2 = 4MHz
  
  // Setup and configure rf radio
  radio.begin(); // initialize radio layers
  network.begin(/*channel*/ 90, /*node address*/ this_node); // set network channel and node address
  
  // Transmit device serial number and status over nRF24L01
  payload_t payload = {0}; // set payload
  RF24NetworkHeader header(/*to node*/ other_node); // set receiver node address
  network.write(header,&payload,sizeof(payload)); // send message
  
  // Cycle through LED ring to indicate power on
  
}

void loop()
{
  // Network pumped regularly to keep the layer going
  network.update();
  
  // Check if a message has been received
  while(network.available())
  {
    // If message has been received, grab it and display led pattern
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header,&payload,sizeof(payload));
    ledDisplay(payload.led_pattern);
  }
}

void setValue(int value)
{
  digitalWrite(SR_SS, LOW); // enable shift registers
  SPI.transfer(value); // send value
  digitalWrite(SR_SS, HIGH); // disable shift registers
}

void ledDisplay(int pattern)
{
  if(pattern == 0)
  {
    int a = 0; // variables used in shifting process
    int b = 0;
    int del = 62; // ms value for delay between LED toggles
    for(int i=0; i<8; i++) {
      setValue(b); // shifts out b00000000 to second shift-register
      setValue(a+2^i); // shifts out: b10000000, b11000000, .. , b11111111 to first shift-register
      a = a+2^i;
      delay(del);
    }
    for(int i=0; i<8; i++) {
      setValue(b+2^i); // shifts out b10000000, b11000000, .. , b11111111 to second shift-register
      setValue(a); // shifts out b11111111 to first shift register
      b = b+2^i;
      delay(del);
    }
    for(int i=0; i<8; i++) {
      setValue(b); // shifts out b11111111 to second shift-register
      setValue(a-2^(8-i)); // shifts out b01111111, b00111111, .. , b00000000 to first shift-register
      a = a-2^(8-i);
      delay(del);
    }
    for(int i=0; i<8; i++) {
      setValue(b-2^(8-i)); // shifts out b01111111, b00111111, .. , b00000000 to second shift-register
      setValue(a); // shifts out b00000000 to first shift register
      b = b-2^(8-i);
      delay(del);
    }
  }
  else if(pattern == 1)
  {
    int a = 0; // variables used in shifting process
    int b = 0;
    int del = 62; // ms value for delay between LED toggles
    for(int j=0; j<4; j++) {  
      // forward circle
      for(int i=0; i<8; i++) {
        setValue(b); // shifts out b00000000 to second shift-register
        setValue(a+2^i); // shifts out: b10000000, b11000000, .. , b11111111 to first shift-register
        a = a+2^i;
        delay(del);
      }
      for(int i=0; i<8; i++) {
        setValue(b+2^i); // shifts out b10000000, b11000000, .. , b11111111 to second shift-register
        setValue(a); // shifts out b11111111 to first shift register
        b = b+2^i;
        delay(del);
      }
      for(int i=0; i<8; i++) {
        setValue(b); // shifts out b11111111 to second shift-register
        setValue(a-2^(8-i)); // shifts out b01111111, b00111111, .. , b00000000 to first shift-register
        a = a-2^(8-i);
        delay(del);
      }
      for(int i=0; i<8; i++) {
        setValue(b-2^(8-i)); // shifts out b01111111, b00111111, .. , b00000000 to second shift-register
        setValue(a); // shifts out b00000000 to first shift register
        b = b-2^(8-i);
        delay(del);
      }
      // reverse circle
      for(int i=0; i<8; i++) {
        setValue(b+2^(8-i)); // shifts out: b00000001, b00000011, .. , b11111111 to second shift-register
        setValue(a); // shifts out b00000000 to first shift register
        b = b+2^(8-i);
        delay(del);
      }
      for(int i=0; i<8; i++) {
        setValue(b); // shifts out b11111111 to second shift-register
        setValue(a+2^(8-i)); // shifts out: b00000001, b00000011, .. , b11111111 to first shift-register
        a = a+2^(8-i);
        delay(del);
      }
      for(int i=0; i<8; i++) {
        setValue(b-2^i); // shifts out b11111110, b11111100, .. , b00000000 to second shift-register
        setValue(a); // shifts out b11111111 to first shift-register
        b = b-2^i;
        delay(del);
      }
      for(int i=0; i<8; i++) {
        setValue(b); // shifts out b00000000 to second shift register
        setValue(a-2^i); // shifts out b11111110, b11111100, .. , b00000000 to first shift-register
        a = a-2^i;
        delay(del);
      } 
    }   
  }
  else
  {
    for(int j=0; j<4; j++) {
      setValue(255);
      setValue(255);
      delay(250);
      setValue(0);
      setValue(0);
      delay(250);
    }
  }
}
