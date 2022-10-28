//
//  battery.mm
//  wmitool
//
//  Ported from libsmbios by lvs1974 on 18/10/2022.
//  Copyright © 2022 lvs1974. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <sstream>
#import <vector>
#import <string>
#import <ctype.h>
#import "wmi.h"

void print_express_charge_status(uint32_t status)
{
	if (status == 0) printf("\t Battery is not present");
	else if (status == 1) printf("\t Standard");
	else if (status == 2) printf("\t Express");
	else if (status == 3) printf("\t One-time express");
	else if (status == 0xFF) printf("\t Battery is present, and it does not support express charge");
	else printf("\t Unknown");
}

int PrintBatteryChargingState()
{
	calling_interface_buffer in  = {};
	calling_interface_buffer out = {};
	size_t outsize = sizeof(out);
	in.cmd_class  = 4;
	in.cmd_select = 12;

	kern_return_t ret = IOConnectCallStructMethod(connect, actionEvaluate, &in, sizeof(in), &out, &outsize);
	if (ret != KERN_SUCCESS) {
		printf("Can't connect to StructMethod to send commands\n");
		return 1;
	}
	else {
		if (out.output[0] != 0) {
			printf("Info: Unable to read Current Battery Charging State on this system\\n");
			return 1;
		} else {
			printf(" \nSupported battery charging features: \n");
			if (is_set(out.output[1], 0)) printf("\t Express Charging");
			if (is_set(out.output[1], 1)) printf("\t Charge Disable");
			if (get_byte(out.output[1], 0) == 0) printf("\t NIL");
							
			printf(" \nBattery charging Status: \n");
			if (is_set((get_byte(out.output[1], 1)), 0)) printf("\t One or more batteries are present that support Express charge");
			if (is_set((get_byte(out.output[1], 1)), 1)) printf("\t Charging is disabled");
			if (get_byte(out.output[1], 1) == 0) printf("\t NIL");
						
			for (int i=0; i<4; i++)
			{
				uint32_t expr_status = get_byte(out.output[2], i);
				if (expr_status)
				{
					printf("\n Battery %d Express Charge State:", i);
					print_express_charge_status(expr_status);
				}
			}
			
			printf("\n");
		}
	}
	
	return 0;
}
