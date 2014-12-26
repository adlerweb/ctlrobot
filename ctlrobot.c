/*
 * ctlrobot.c - simple code to control USB dancing robot.
 *
 * based on
 * ctlmissile.c - simple code to control USB missile launchers.
 *
 * Copyright 2006 James Puderer <jpuderer@littlebox.ca>
 * Copyright 2006 Jonathan McDowell <noodles@earth.li>
 * Copyright 2014 Florian Knodt <git@adlerweb.info>
 *
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; version 2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usb.h>

int debug = 1;

/*
 * Command to control Dream Cheeky USB dancing robot
 */
void send_command_cheeky(struct usb_device *dev, char *mcmd, char *lcmd)
{
    usb_dev_handle *launcher;
    char data[8];
    int ret;
    unsigned int i=0;

	memset(data, 0, 8);

	launcher = usb_open(dev);
	if (launcher == NULL) {
		perror("Unable to open device");
		exit(EXIT_FAILURE);
	}

	/* Detach kernel driver (usbhid) from device interface and claim */
	usb_detach_kernel_driver_np(launcher, 0);
	usb_detach_kernel_driver_np(launcher, 1);

	ret = usb_set_configuration(launcher, 1);
	if (ret < 0) {
		perror("Unable to set device configuration");
		exit(EXIT_FAILURE);
	}
	ret = usb_claim_interface(launcher, 0);
	if (ret < 0) {
		perror("Unable to claim interface");
		exit(EXIT_FAILURE);
	}

/*     0b0000 0000
           ||   |-- Normal Move
           ||   --- Fast Move
           ||
           |------  LED Single Flash
		   -------  LED Double Flash
*/

	data[0] = 0x00;

	if (!strcmp(mcmd, "slow")) {
	    data[0] |= 0x01;
	} else if (!strcmp(mcmd, "fast")) {
	    data[0] |= 0x02;
	} else if (!strcmp(mcmd, "off")) {
	    //data[0] |= 0x00;
	}

	if (!strcmp(lcmd, "slow")) {
	    data[0] |= 0x10;
	} else if (!strcmp(lcmd, "fast")) {
	    data[0] |= 0x20;
	} else if (!strcmp(lcmd, "off")) {
	    //data[0] |= 0x00;
	}

	if (debug) {
		printf("Sending 0x%02x\n",
			data[0]);
	}

	ret = usb_control_msg(launcher,
			USB_DT_HID,
			USB_REQ_SET_CONFIGURATION,
			0x0200,
			0,
			data,
			8,		// Length of data.
			5000);		// Timeout
	if (ret != 8) {
		fprintf(stderr, "USB Send Error: %s\n", usb_strerror());
		exit(EXIT_FAILURE);
	}

	usb_release_interface(launcher, 0);
	usb_close(launcher);

}

int main(int argc, char *argv[])
{
	struct usb_bus *busses, *bus;
	struct usb_device *dev = NULL;

	if (argc != 3) {
		printf("Usage: ctlrobot [ slow | fast | off ] [ slow | fast | off ] \n");
		printf("                [      movement     ] [         LEDs      ] \n");
		exit(EXIT_FAILURE);
	}

	usb_init();
	usb_find_busses();
	usb_find_devices();

	busses = usb_get_busses();

	for (bus = busses; bus && !dev; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			if (debug) {
				printf("Checking 0x%04x:0x%04x\n",
					dev->descriptor.idVendor,
					dev->descriptor.idProduct);
			}
			if (dev->descriptor.idVendor == 0x1941 &&
				dev->descriptor.idProduct == 0x8021) {
				send_command_cheeky(dev, argv[1], argv[2]);
				break;
			}
		}
	}

	if (!dev) {
		fprintf(stderr, "Unable to find device.\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}

