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
 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// this is libusb, see http://libusb.sourceforge.net/ 
#include <usb.h>

// same as in main.c
#define USB_GET_TEMP 0
#define USB_FAN_ON  1
#define USB_FAN_OFF 2
#define USB_GET_STUDENT 3
#define USB_SET_STUDENT 4
#define USB_GET_TEACHER 5
#define USB_SET_TEACHER 6

// used to get descriptor strings for device identification 
static int usbGetDescriptorString(usb_dev_handle *dev, int index, int langid, 
                                  char *buf, int buflen) {
    char buffer[256];
    int rval, i;

	// make standard request GET_DESCRIPTOR, type string and given index 
    // (e.g. dev->iProduct)
	rval = usb_control_msg(dev, 
        USB_TYPE_STANDARD | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
        USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) + index, langid, 
        buffer, sizeof(buffer), 1000);
        
    if(rval < 0) // error
		return rval;
	
    // rval should be bytes read, but buffer[0] contains the actual response size
	if((unsigned char)buffer[0] < rval)
		rval = (unsigned char)buffer[0]; // string is shorter than bytes read
	
	if(buffer[1] != USB_DT_STRING) // second byte is the data type
		return 0; // invalid return type
		
	// we're dealing with UTF-16LE here so actual chars is half of rval,
	// and index 0 doesn't count
	rval /= 2;
	
	// lossy conversion to ISO Latin1 
	for(i = 1; i < rval && i < buflen; i++) {
		if(buffer[2 * i + 1] == 0)
			buf[i-1] = buffer[2 * i];
		else
			buf[i-1] = '?'; // outside of ISO Latin1 range
	}
	buf[i-1] = 0;
	
	return i-1;
}

static usb_dev_handle * usbOpenDevice(int vendor, char *vendorName, 
                                      int product, char *productName) {
	struct usb_bus *bus;
	struct usb_device *dev;
	char devVendor[256], devProduct[256];
    
	usb_dev_handle * handle = NULL;
	
	usb_init();
	usb_find_busses();
	usb_find_devices();
	
	for(bus=usb_get_busses(); bus; bus=bus->next) {
		for(dev=bus->devices; dev; dev=dev->next) {			
			if(dev->descriptor.idVendor != vendor ||
               dev->descriptor.idProduct != product)
                continue;
                
            // we need to open the device in order to query strings 
            if(!(handle = usb_open(dev))) {
                fprintf(stderr, "Warning: cannot open USB device: %s\n",
                    usb_strerror());
                continue;
            }
            
            // get vendor name 
            if(usbGetDescriptorString(handle, dev->descriptor.iManufacturer, 0x0409, devVendor, sizeof(devVendor)) < 0) {
                fprintf(stderr, 
                    "Warning: cannot query manufacturer for device: %s\n", 
                    usb_strerror());
                usb_close(handle);
                continue;
            }
            
            // get product name 
            if(usbGetDescriptorString(handle, dev->descriptor.iProduct, 
               0x0409, devProduct, sizeof(devVendor)) < 0) {
                fprintf(stderr, 
                    "Warning: cannot query product for device: %s\n", 
                    usb_strerror());
                usb_close(handle);
                continue;
            }
            
            if(strcmp(devVendor, vendorName) == 0 && 
               strcmp(devProduct, productName) == 0)
                return handle;
            else
                usb_close(handle);
		}
	}
	
	return NULL;
}

int main(int argc, char **argv) {
	usb_dev_handle *handle = NULL;
    int nBytes = 0;
    char buffer[256];

	if(argc < 2) {
		printf("Usage:\n");
		printf("usbtext.exe getTemp\n");
		printf("usbtext.exe fanOn\n");
		printf("usbtext.exe fanOff\n");
		printf("usbtext.exe getStudent\n");
		printf("usbtext.exe setStudent\n");
		printf("usbtext.exe getTeacher\n");
		printf("usbtext.exe setTeacher\n");
		exit(1);
	}
	
	handle = usbOpenDevice(0x16C0, "codeandlife.com", 0x05DC, "USBexample");
	
	if(handle == NULL) {
		fprintf(stderr, "Could not find USB device!\n");
		exit(1);
	}

	if(strcmp(argv[1], "getTemp") == 0) { //on
		nBytes = usb_control_msg(handle, 
            USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
			USB_GET_TEMP, 0, 0, (char *)buffer, sizeof(buffer), 5000);
        printf("Got %d bytes ... Temparature : %s\n", nBytes, buffer);
	} else if(strcmp(argv[1], "fanOn") == 0) { //off
		nBytes = usb_control_msg(handle, 
            USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
			USB_FAN_ON, 0, 0, (char *)buffer, sizeof(buffer), 5000);
	} else if(strcmp(argv[1], "fanOff") == 0) { //out
		nBytes = usb_control_msg(handle, 
            USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
			USB_FAN_OFF, 0, 0, (char *)buffer, sizeof(buffer), 5000);
	} else if(strcmp(argv[1], "getStudent") == 0) {  //write
		nBytes = usb_control_msg(handle, 
            USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
			USB_GET_STUDENT, 0, 0, (char *)buffer, sizeof(buffer), 5000);
        printf("Got %d bytes ... Student : %s\n", nBytes, buffer);
	} else if(strcmp(argv[1], "setStudent") == 0 && argc > 2) { //in
		nBytes = usb_control_msg(handle, 
            USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 
			USB_SET_STUDENT, 0, 0, argv[2], strlen(argv[2])+1, 5000);
	} else if(strcmp(argv[1], "getTeacher") == 0) {  //write
		nBytes = usb_control_msg(handle, 
            USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
			USB_GET_TEACHER, 0, 0, (char *)buffer, sizeof(buffer), 5000);
        printf("Got %d bytes ... Teacher : %s\n", nBytes, buffer);
	} else if(strcmp(argv[1], "setTeacher") == 0 && argc > 2) { //in
		nBytes = usb_control_msg(handle, 
            USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 
			USB_SET_TEACHER, 0, 0, argv[2], strlen(argv[2])+1, 5000);
	}
	
	if(nBytes < 0)
		fprintf(stderr, "USB error: %s\n", usb_strerror());
		
	usb_close(handle);
	
	return 0;
}
