//Be nam _e_ khoda

/**
 * Temparature Monitoring Using USB Protocol
 * Interface Ciruit Course, Iran University of Science and Technology (IUST)
 * Author : Mehran Memarnejad (92521267)
 *
 *
 * Acknowledgment:
 *
 * To Accomplish this project, I have used some resource.
 * The Main resources are :
 * 		Project: AVR ATtiny USB Tutorial at http://codeandlife.com/
 * 		Author: Joonas Pihlajamaa, joonas.pihlajamaa@iki.fi
 * 		Inspired by V-USB example code by Christian Starkjohann
 * 		Copyright: (C) 2012 by Joonas Pihlajamaa
 * 		License: GNU GPL v3 (see License.txt)
 */
 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "usbdrv.h"

#define F_CPU 12000000L
#include <util/delay.h>

#define FAN_PORT PORTB
#define FAN_DDR DDRB

#define STUDENT 1
#define TEACHER 0

//Our Application Layer Protocol
#define USB_GET_TEMP 0
#define USB_FAN_ON  1
#define USB_FAN_OFF 2
#define USB_GET_STUDENT 3
#define USB_SET_STUDENT 4
#define USB_GET_TEACHER 5
#define USB_SET_TEACHER 6

static uchar student[18] = "Mehran Memarnejad";
static uchar teacher[12] = "DR.Patooghi";
static uchar whichString;
static uchar temparature;
static uchar dataReceived = 0, dataLength = 0; // for USB_DATA_IN

// this gets called when custom control message is received
USB_PUBLIC uchar usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (void *)data; // cast data to correct type
	
	switch(rq->bRequest) { // custom command is in the bRequest field
	
		case USB_GET_TEMP:
			usbMsgPtr = &temparature; // send Temparature to PC
			return sizeof(temparature);
			
		case USB_FAN_ON:
			FAN_PORT |= 1; // turn FAN on
			return 0;
			
		case USB_FAN_OFF: 
			FAN_PORT &= ~1; // turn FAN off
			return 0;
			
		case USB_GET_STUDENT: // send data to PC
			usbMsgPtr = student;
			return sizeof(student);
			
		case USB_SET_STUDENT: // receive data from PC
			dataLength  = (uchar)rq->wLength.word;
			dataReceived = 0;
			
			if(dataLength  > sizeof(student)){ // limit to buffer size
				dataLength  = sizeof(student);
				whichString = STUDENT;
			}
	
			return USB_NO_MSG; // usbFunctionWrite will be called now
			
		case USB_GET_TEACHER: // send data to PC
			usbMsgPtr = teacher;
			return sizeof(teacher);
			
		case USB_SET_TEACHER: // receive data from PC
			dataLength  = (uchar)rq->wLength.word;
			dataReceived = 0;
			
			if(dataLength  > sizeof(teacher)){// limit to buffer size
				dataLength  = sizeof(teacher);
				whichString = TEACHER;
			}
			
			return USB_NO_MSG; // usbFunctionWrite will be called now
    }

    return 0; // should not get here
}

// This gets called when data is sent from PC to the device
USB_PUBLIC uchar usbFunctionWrite(uchar *data, uchar len) {
	uchar i;
	
	if(whichString == STUDENT){
		for(i = 0; dataReceived < dataLength && i < len; i++, dataReceived++)
			student[dataReceived] = data[i];
	}
	
	else{
		for(i = 0; dataReceived < dataLength && i < len; i++, dataReceived++)
			teacher[dataReceived] = data[i];
	}
		
	return (dataReceived == dataLength); // 1 if we received it all, 0 if not
}

//This Function configure ADC
void ADCInit(void){
	
	ADCSRA = 0x87;
	ADMUX = 0xE0;

}

//This Function Calculates temparature
void getTemparature(uchar *temparature){
	
	ADCSRA |= (1<<ADSC);
	while((ADCSRA & (1<<ADIF)) == 0);
	*temparature = ADCH;
	
}


int main() {
	
    wdt_enable(WDTO_1S); // enable 1s watchdog timer
	
	FAN_DDR = 1;
	
	ADCInit();
	
    usbInit();
	
    usbDeviceDisconnect(); // enforce re-enumeration
    for(i = 0; i<250; i++) { // wait 500 ms
        wdt_reset(); // keep the watchdog happy
        _delay_ms(2);
    }
    usbDeviceConnect();
	
    sei(); // Enable interrupts after re-enumeration
	
    while(1) {
        wdt_reset(); // keep the watchdog happy
		getTemparature(&temparature);
        usbPoll();
    }
	
    return 0;
}
