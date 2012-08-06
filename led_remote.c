/* Led Remote Control
   By Jeremy Ruhland
   https://github.com/JeremyRuhland/saphrix-suit-glow
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

//set baud rate
#define BAUDRATE 1200
//calculate UBRR
#define UBRRVAL ((F_CPU/(BAUDRATE*16UL))-1)

#define DEBOUNCE 5 //Button press debounce in mS
#define TX PD1 //Location of UART Tx line
#define TXPWR PD7 //Location of transmitter mosfet gate
#define TXPWRDELAY 500 //Delay for powering up transmitter in uS
#define PREAMBLE 0xAA //0b10101010
#define RXADDR 0x44 //Address of remote receiver

volatile uint8_t portmemory; //Variable for port memory

void Send_Packet(uint8_t addr, uint8_t cmd) { //Send a datapacket over UART to transmitter, handle powerup/down of Tx
 PORTD |= (1<<TXPWR); //Turn on power to transmitter
 _delay_us(TXPWRDELAY); //Delay to power on transmitter
 UCSR0B |= (1<<TXEN0); //Enable UART transmitter
 while(!(UCSR0A & (1<<UDRE0))); //Wait if a byte is being transmitted
 UDR0 = PREAMBLE; //Transmit preamble
 while(!(UCSR0A & (1<<UDRE0))); //Wait if a byte is being transmitted
 UDR0 = addr; //Transmit address
 while(!(UCSR0A & (1<<UDRE0))); //Wait if a byte is being transmitted
 UDR0 = cmd; //Transmit command
 while(!(UCSR0A & (1<<UDRE0))); //Wait if a byte is being transmitted
 UDR0 = (addr^cmd); //Transmit chksum
 while(!(UCSR0A & (1<<UDRE0))); //Wait if a byte is being transmitted
 UCSR0B &= !(1<<TXEN0); //Disable UART transmitter
 PORTD &= !(1<<TXPWR); //Turn off power to transmitter
}

int main(void) {

//Power savings
 PRR |= (1<<PRTWI) | (1<<PRTIM2) | (1<<PRTIM0) | (1<<PRTIM1) | (1<<PRSPI) | (1<<PRADC); //Shut down everything unneeded

//Set up UART
//Asynchronous mode, no parity, 1 stop bit, 8 bit size
 UBRR0L=(uint8_t)UBRRVAL; //Set baud rate of low byte
 UBRR0H=(UBRRVAL>>8); //and high byte

//Set up I/O
 PORTB = 0xFF; //Turn on pull up resistors
 PORTC = 0xFF; //Turn on pull up resistors
 PORTD = !((1<<TX) | (1<<TXPWR)); //Turn on pull up resistors to all but Tx and TxPWR
 DDRD |= (1<<TXPWR); //Set TxPwr as output

//Set up interrupts
 PCMSK1 |= (1<<PCINT11) | (1<<PCINT10) | (1<<PCINT9) | (1<<PCINT8); //Turn on pin change interrupt 1 for all buttons

 for(;;) { //Loop forever
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Set powerdown sleep mode
  sei(); //Global enable interrupts
  sleep_mode(); //Go to sleep
 }
 return 0;
}

ISR(PCINT1_vect) { //Fires when button is pressed
 portmemory = PORTC; //Remember state of port C
 portmemory = portmemory & ((1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3)); //Remove extra bits from port C
 if (portmemory) //Only send packet of port C != 0
  Send_Packet(RXADDR, portmemory); //Send packet with port data
 _delay_ms(DEBOUNCE); //Debounce delay
}
