//
//  main.mm
//  wmitool
//
//  Created by lvs1974 on 18/10/2022.
//  Copyright © 2022 lvs1974. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <sstream>
#import <vector>
#import <string>

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


int main(int argc, const char * argv[]) {

	service = getService();

	int count = 0;
	while (!service)
	{
		service = getService();
		count++;
		// Try load 10 times, otherwise error return
		if (count > 10) {
			printf("Couldn't get path to IO service\n");
			return (1);
		}
	}
	
	printf("path to IO service was acquired\n");
	
	kern_return_t ret;
	//io_connect_t connect = 0;
	ret = IOServiceOpen(service, mach_task_self(), 0, &connect);
	if (ret != KERN_SUCCESS)
	{
		printf("Couldn't open IO service\n");
	}
	else
	{
		printf("IO service was opened\n");
	}

	// set thermal status
//	{
//		calling_interface_buffer in  = {};
//		calling_interface_buffer out = {};
//		size_t outsize = sizeof(out);
//
//		in.cmd_class = 17;
//		in.cmd_select = 19;
//		in.input[0] = 1;
//		in.input[1] = 1;	// cool
//
//		ret = IOConnectCallStructMethod(connect,
//										actionEvaluate,
//										&in,
//										sizeof(in),
//										&out,
//										&outsize
//										);
//
//		if (ret != KERN_SUCCESS)
//			printf("Can't connect to StructMethod to send commands\n");
//		else
//			printf("actionEvaluate %x returns result %d, value %d, value %d\n", in.input[0], out.output[0], out.output[1], out.output[2]);
//	}
	
	// get thermal status
	{
		calling_interface_buffer in  = {};
		calling_interface_buffer out = {};
		size_t outsize = sizeof(out);
		
		in.cmd_class = 17;
		in.cmd_select = 19;
		in.input[0] = 0;
		//in.input[0] = 0xF610;
		
		ret = IOConnectCallStructMethod(connect,
										actionEvaluate,
										&in,
										sizeof(in),
										&out,
										&outsize
										);
		
		if (ret != KERN_SUCCESS)
			printf("Can't connect to StructMethod to send commands\n");
		else
			printf("actionEvaluate %x returns result %d, value %d, value %d\n", in.input[0], out.output[0], out.output[1], out.output[2]);
	}
	
	if (connect)
		IOServiceClose(connect);
	
	if (service)
		IOObjectRelease(service);

	return 0;
}
