#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//set desired baud rate
#define BAUDRATE 1200
//calculate UBRR value
#define UBRRVAL ((F_CPU/(BAUDRATE*16UL))-1)

#define RXPWR PD7 //Location of transmitter mosfet gate
#define RXPWRDELAY 500 //Delay for powering up transmitter in uS
#define RXADDR 0x44 //Address of remote receive

uint8_t USART_vReceiveByte(void) {
    // Wait until a byte has been received
    while((UCSRA&(1<<RXC)) == 0);
    // Return received data
    return UDR;
}

ISR(USART_RXC_vect) {
 //define variables
 uint8_t raddress, data, chk;//transmitter address
 //receive destination address
 raddress=USART_vReceiveByte();
 //receive data
 data=USART_vReceiveByte();
 //receive checksum
 chk=USART_vReceiveByte();
 //compare received checksum with calculated
 if(chk==(raddress+data)) {//if match perform operations {
  //if transmitter address match
  if(raddress==RADDR) {
    if(data==LEDON) {
      PORTC&=~(1<<0);//LED ON
     }
    else if(data==LEDOFF) {
      PORTC|=(1<<0);//LED OFF
     }
    else {
     //blink led as error
     PORTC|=(1<<0);//LED OFF
     _delay_ms(10);
     PORTC&=~(1<<0);//LED ON 
    }
   }
 }
}

int main(void) {

 //Set up serial comms
 //Set baud rate
 UBRR0L=(uint8_t)UBRRVAL;  //low byte and
 UBRR0H=(UBRRVAL>>8); //high byte
 //Enable Transmitter and Receiver and Interrupt on receive complete
 UCSRB=(1<<RXEN)|(1<<RXCIE);

 return 0;
}
