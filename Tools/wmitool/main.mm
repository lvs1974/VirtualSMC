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
#import "wmi.h"

io_connect_t connect = NULL;
io_service_t service = NULL;

io_service_t getService()
{
	io_service_t 	service = 0;
	mach_port_t 	masterPort;
	io_iterator_t 	iter;
	kern_return_t 	ret;
	io_string_t 	path;
	
	ret = IOMainPort(MACH_PORT_NULL, &masterPort);
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
	printf("get information : \n %s --info -i,  This will Display the Supported Features of USTT and AAC\n\n", name);
	printf("get current thermal info :\n %s  --get-thermal-info -g,  This will display the thermal information of a system \n\n", name);
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

int PrintRaw(int argc, const char * argv[])
{
	calling_interface_buffer in  = {};
	calling_interface_buffer out = {};
	size_t outsize = sizeof(out);

	in.cmd_class  = str2int(argv[2]);
	in.cmd_select = str2int(argv[3]);
	for (int a=4, i=0; a<argc; ++a)
		in.input[i++] = str2int(argv[a]);

	kern_return_t ret = IOConnectCallStructMethod(connect, actionEvaluate, &in, sizeof(in), &out, &outsize);
	if (ret != KERN_SUCCESS) {
		printf("Can't connect to StructMethod to send commands\n");
		return 1;
	}
	
	printf("actionEvaluate returns res[0] = %d, res[1] = %d, res[2] = %d, res[3] = %d\n", out.output[0], out.output[1], out.output[2], out.output[3]);
	return 0;
}


int main(int argc, const char * argv[]) {
		
	char * parameter = (argc >= 2) ? (char *)argv[1] : NULL;
	if (parameter == NULL || !strncmp(parameter, "--help", 6)) {
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
	
	int result = 1;
		
	kern_return_t ret;
	ret = IOServiceOpen(service, mach_task_self(), 0, &connect);
	if (ret != KERN_SUCCESS)
	{
		printf("Couldn't open IO service\n");
		goto exit;
	}
	
    if (!strncmp(parameter, "wmi", 3)) {
		if (argc < 4)
		{
			usage(argv[0]);
			goto exit;
		}

		result = PrintRaw(argc, argv);
	}
	else if (!strncmp(parameter, "-i", 2) || !strncmp(parameter, "--info", 6)) {
		result = PrintSupportedThermModes();
	}
	else if (!strncmp(parameter, "-g", 2) || !strncmp(parameter, "--get-thermal-info", 18)) {
		result = GetCurrentThermalMode();
	}
	else if (!strncmp(parameter, "--set-thermal-mode", 18)) {
		const char *thermal_mode = argv[2];
		if (argc == 2)
		{
			if (strlen(parameter) <= 22 || parameter[18] != '=') {
				usage(argv[0]);
				goto exit;
			}
			thermal_mode = &parameter[19];
		}
		else if (argc < 3)
		{
			usage(argv[0]);
			goto exit;
		}
		result = SetCurrentThermalMode(thermal_mode);
	} else {
		usage(argv[0]);
	}
	
exit:
	if (connect)
		IOServiceClose(connect);
	
	if (service)
		IOObjectRelease(service);

	return result;
}
