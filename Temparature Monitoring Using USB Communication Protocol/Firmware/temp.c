//be nam _e_ khoda

#include <avr/io.h>

unsigned char temparatue;

int main(void)
{
	DDRD = 0xFF;
	DDRA = 0;
	ADCSRA = 0x87;
	ADMUX = 0xE0;
	
	usart_init();

	while(1){
		
		ADCSRA |= (1<<ADSC);
		while((ADCSRA & (1<<ADIF)) == 0);
		temparatue = ADCH;
		
		usart_send(temparatue);
		
	}

	return 0;

}