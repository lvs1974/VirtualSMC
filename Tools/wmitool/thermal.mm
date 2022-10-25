//
//  thermal.mm
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

int PrintSupportedThermModes()
{
	printf("\n Print all the Available Thermal Information of your system: \n");
	printf("-------------------------------------------------------------------\n");

	calling_interface_buffer in  = {};
	calling_interface_buffer out = {};
	size_t outsize = sizeof(out);
	in.cmd_class  = 17;
	in.cmd_select = 19;

	kern_return_t ret = IOConnectCallStructMethod(connect, actionEvaluate, &in, sizeof(in), &out, &outsize);
	if (ret != KERN_SUCCESS) {
		printf("Can't connect to StructMethod to send commands\n");
		return 1;
	}
	else {
		if (out.output[0] != 0) {
			printf("Info: Unable to Get Thermal Information on this system\n");
			return 1;
		} else {
			printf(" \nSupported Thermal Modes: \n");
			if (is_set(out.output[1], 0)) printf("\t Balanced");
			if (is_set(out.output[1], 1)) printf("\t Cool Bottom");
			if (is_set(out.output[1], 2)) printf("\t Quiet");
			if (is_set(out.output[1], 3)) printf("\t Performance");
							
			printf(" \nSupported Active Acoustic Controller (AAC) modes: \n");
			if (is_set((get_byte(out.output[1], 1)), 0)) printf("\t AAC (Balanced)");
			if (is_set((get_byte(out.output[1], 1)), 1)) printf("\t AAC (Cool Bottom)");
			if (is_set((get_byte(out.output[1], 1)), 2)) printf("\t ACC (Quiet)");
			if (is_set((get_byte(out.output[1], 1)), 3)) printf("\t ACC (Performance)");
											
			printf(" \nSupported AAC Configuration type: \n");
			if (get_byte(out.output[2], 1) == 0) printf("\tGlobal (AAC enable/disable applies to all supported USTT modes)");
			if (get_byte(out.output[2], 1) == 1) printf("\tUser Selectable Thermal Table(USTT) mode specific");
			printf("\n");
		}
	}
	
	return 0;
}

int GetCurrentThermalMode(const char *flag)
{
	calling_interface_buffer in  = {};
	calling_interface_buffer out = {};
	size_t outsize = sizeof(out);
	in.cmd_class  = 17;
	in.cmd_select = 19;
	
	kern_return_t ret = IOConnectCallStructMethod(connect, actionEvaluate, &in, sizeof(in), &out, &outsize);
	if (ret != KERN_SUCCESS) {
		printf("Can't connect to StructMethod to send commands\n");
		return 1;
	}
	else {
		if (out.output[0] != 0) {
			printf("Info: Unable to Get Thermal Information on this system\n");
			return 1;
		} else {
			if (!strncmp(flag, "thermal-mode", 12)) return out.output[2];
			if (!strncmp(flag, "acc-mode", 8)) return (get_byte(out.output[2], 2));
			
			printf("\n Print Current Status of Thermal Information: \n");
			printf("-------------------------------------------------------------------\n");
			printf(" \nCurrent Thermal Modes: \n");
			if (is_set(out.output[2], 0)) printf("\t Balanced");
			if (is_set(out.output[2], 1)) printf("\t Cool Bottom");
			if (is_set(out.output[2], 2)) printf("\t Quiet");
			if (is_set(out.output[2], 3)) printf("\t Performance");
			
			printf(" \nCurrent Active Acoustic Controller (AAC) Mode: \n");
			if (get_byte(out.output[2], 2) == 0) printf("\t AAC mode Disabled");
			if (get_byte(out.output[2], 2) == 1) printf("\t AAC mode Enabled");
			
			printf(" \nCurrent Active Acoustic Controller (AAC) Mode: \n");
			if (get_byte(out.output[2], 1) == 0)
			{
				if (get_byte(out.output[2], 2) == 0) printf("\tGlobal (AAC enable/disable applies to all supported USTT modes)");
				if (get_byte(out.output[2], 2) == 1) printf("\tUSTT mode specific");
			}
			else if (get_byte(out.output[2], 1) == 1)
			{
				if (is_set ((get_byte(out.output[2], 2)), 0)) printf("\t AAC (Balanced)");
				if (is_set ((get_byte(out.output[2], 2)), 1)) printf("\t AAC (Cool Bottom)");
				if (is_set ((get_byte(out.output[2], 2)), 2)) printf("\t ACC (Quiet)");
				if (is_set ((get_byte(out.output[2], 2)), 3)) printf("\t ACC (Performance)");
			}

			printf(" \nCurrent Fan Failure Mode: \n");
			if (is_set((get_byte(out.output[2], 3)), 0)) printf("\tMinimal Fan Failure (at least one fan has failed, one fan working)");
			if (is_set((get_byte(out.output[2], 3)), 1)) printf("\tCatastrophic Fan Failure (all fans have failed)");
		}
	}
	
	return 0;
}

int SetCurrentThermalMode(const char *thermal_mode)
{
	uint32_t var_thermal_mode = 0;
	if      (!strcmp(thermal_mode, "balanced")) 	var_thermal_mode = 1;
	else if (!strcmp(thermal_mode, "cool-bottom")) 	var_thermal_mode = 1 << 1;
	else if (!strcmp(thermal_mode, "quiet")) 		var_thermal_mode = 1 << 2;
	else if (!strcmp(thermal_mode, "performance")) 	var_thermal_mode = 1 << 3;
	else {
		printf("Invalid thermal mode option specified: %s,  \
				\nSupported modes : balanced, cool-bottom, quiet, performance\n", thermal_mode);
		return 1;
	}

	uint32_t acc_mode = GetCurrentThermalMode("acc-mode");

	calling_interface_buffer in  = {};
	calling_interface_buffer out = {};
	size_t outsize = sizeof(out);
	in.cmd_class  = 17;
	in.cmd_select = 19;
	in.input[0] = 1;
	in.input[1] = acc_mode << 8 | var_thermal_mode;
	
	kern_return_t ret = IOConnectCallStructMethod(connect, actionEvaluate, &in, sizeof(in), &out, &outsize);
	if (ret != KERN_SUCCESS) {
		printf("Can't connect to StructMethod to send commands\n");
		return 1;
	}
	else {
		if (out.output[0] != 0) {
			printf("Info: Unable to Get Thermal Information on this system\n");
			return 1;
		} else {
			printf("Thermal Information Set successfully to: %s\n", thermal_mode);
		}
	}
	
	return 0;
}
