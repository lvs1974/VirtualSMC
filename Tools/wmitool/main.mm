//
//  main.mm
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

#define kWMIDellSensorsClassName "WMIDellSensors"

io_connect_t connect = NULL;
io_service_t service = NULL;

enum { actionEvaluate = 0 };



using int_array = uint32_t[4];
typedef struct
{
	uint16_t     cmd_class;
	uint16_t     cmd_select;
	int_array    input;
	int_array    output;
} calling_interface_buffer;

io_service_t getService()
{
	io_service_t 	service = 0;
	mach_port_t 	masterPort;
	io_iterator_t 	iter;
	kern_return_t 	ret;
	io_string_t 	path;
	
	ret = IOMasterPort(MACH_PORT_NULL, &masterPort);
	if (ret != KERN_SUCCESS) {
		printf("Can't get masterport\n");
		goto failure;
	}
	
	ret = IOServiceGetMatchingServices(masterPort, IOServiceMatching(kWMIDellSensorsClassName), &iter);
	if (ret != KERN_SUCCESS) {
		printf("WMIDellSensors.kext is not running\n");
		goto failure;
	}
	
	service = IOIteratorNext(iter);
	IOObjectRelease(iter);
	
	if (!service) {
		printf("WMIDellSensors.kext IOService is null\n");
	}
	
	ret = IORegistryEntryGetPath(service, kIOServicePlane, path);
	if (ret != KERN_SUCCESS) {
		printf("Can't get registry-entry path\n");
		goto failure;
	}
	
failure:
	return service;
}

void usage(const char *name)
{
	printf("--------------------------------------------------------------------------\n");
	printf("wmitool WMI for Dell computers\n");
	printf("Copyright (C) 2022 lvs1974 \n");
	printf("--------------------------------------------------------------------------\n");

	printf("Usage:\n");
	printf("raw call:  \n    %s wmi <class> <selector> [input1] [input2] [input3] [input4] \n\n", name);
	printf("get current thermal info :\n    %s -g, --get-thermal-info   This will display the thermal information of a system \n\n", name);
	printf("set thermal mode: \n %s --set-thermal-mode   Option to set Thermal Mode; balanced, cool-bottom, quiet, performance \n\n", name);
}

unsigned long long hex2int(const char *s)
{
	return strtoull(s, NULL, 16);
}

uint32_t str2int(const char *s)
{
	if (strlen(s) > 2 && s[0] == '0' && s[1] == 'x')
		return static_cast<uint32_t>(strtoul(s, NULL, 16));
	return static_cast<uint32_t>(strtoul(s, NULL, 10));
}

bool is_set(uint32_t num, uint8_t pos)
{
	uint32_t mask = 1 << pos;
	return (num & mask) ? true : false;
}

// return a the byte specified by byte_pos from a double word num
uint32_t get_byte(uint32_t num, uint8_t byte_pos)
{
	if (byte_pos < 0 | byte_pos > 3)
		return 0xFFFFFFFF;
	return ((num >> (byte_pos*8)) & 0xFF);
}

uint32_t GetCurrentThermalMode(const char *flag="default")
{
	calling_interface_buffer in  = {};
	calling_interface_buffer out = {};
	size_t outsize = sizeof(out);
	in.cmd_class  = 17;
	in.cmd_select = 19;
	
	kern_return_t ret = IOConnectCallStructMethod(connect, actionEvaluate, &in, sizeof(in), &out, &outsize);
	if (ret != KERN_SUCCESS)
		printf("Can't connect to StructMethod to send commands\n");
	else {
		if (out.output[0] != 0) {
			printf("Info: Unable to Get Thermal Information on this system\n");
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

void SetCurrentThermalMode(const char *thermal_mode)
{
	uint32_t var_thermal_mode = 0;
	if   (!strcmp(thermal_mode, "balanced")) 		var_thermal_mode = 1;
	else if (!strcmp(thermal_mode, "cool-bottom")) 	var_thermal_mode = 1 << 1;
	else if (!strcmp(thermal_mode, "quiet")) 		var_thermal_mode = 1 << 2;
	else if (!strcmp(thermal_mode, "performance")) 	var_thermal_mode = 1 << 3;
	else {
		printf("Invalid thermal mode option specified: %s,  \
				\nSupported modes : balanced, cool-bottom, quiet, performance\n", thermal_mode);
		return;
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
	if (ret != KERN_SUCCESS)
		printf("Can't connect to StructMethod to send commands\n");
	else {
		if (out.output[0] != 0) {
			printf("Info: Unable to Get Thermal Information on this system\n");
		} else {
			printf("Thermal Information Set successfully to: %s\n", thermal_mode);
		}
	}
}

int main(int argc, const char * argv[]) {

	char * parameter;
	if (argc >= 2)
	{
		parameter = (char *)argv[1];
	} else {
		usage(argv[0]);
		return 1;
	}

	
	service = getService();

	int count = 0;
	while (!service)
	{
		service = getService();
		count++;
		// Try load 10 times, otherwise error return
		if (count > 10) {
			printf("Couldn't get path to IO service\n");
			return 1;
		}
	}
		
	kern_return_t ret;
	ret = IOServiceOpen(service, mach_task_self(), 0, &connect);
	if (ret != KERN_SUCCESS)
	{
		printf("Couldn't open IO service\n");
		return 1;
	}
	
    if (!strncmp(parameter, "wmi", 3)) {
		if (argc < 4)
		{
			usage(argv[0]);
			return 1;
		}

		calling_interface_buffer in  = {};
		calling_interface_buffer out = {};
		size_t outsize = sizeof(out);

		in.cmd_class  = str2int(argv[2]);
		in.cmd_select = str2int(argv[3]);
		for (int a=4, i=0; a<argc; ++a)
			in.input[i++] = str2int(argv[a]);

		ret = IOConnectCallStructMethod(connect, actionEvaluate, &in, sizeof(in), &out, &outsize);
		if (ret != KERN_SUCCESS)
			printf("Can't connect to StructMethod to send commands\n");
		else
			printf("actionEvaluate returns res[0] = %d, res[1] = %d, res[2] = %d, res[3] = %d\n", out.output[0], out.output[1], out.output[2], out.output[3]);
	}
	else if (!strncmp(parameter, "-g", 2) || !strncmp(parameter, "--get-thermal-info", 18)) {
		GetCurrentThermalMode();
	}
	else if (!strncmp(parameter, "--set-thermal-mode", 18)) {
		const char *thermal_mode = argv[2];
		if (argc == 2)
		{
			if (strlen(parameter) <= 22 || parameter[18] != '=') {
				usage(argv[0]);
				return 1;
			}
			thermal_mode = &parameter[19];
		}
		else if (argc < 3)
		{
			usage(argv[0]);
			return 1;
		}
		SetCurrentThermalMode(thermal_mode);
	}

		
	if (connect)
		IOServiceClose(connect);
	
	if (service)
		IOObjectRelease(service);

	return 0;
}
