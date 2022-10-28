//
//  wmi.h
//  wmitool
//
//  Ported from libsmbios by lvs1974 on 18/10/2022.
//  Copyright © 2022 lvs1974. All rights reserved.
//

#define kWMIDellSensorsClassName "WMIDellSensors"

extern io_connect_t connect;
extern io_service_t service;

enum { actionEvaluate = 0 };



using int_array = uint32_t[4];
typedef struct
{
	uint16_t     cmd_class;
	uint16_t     cmd_select;
	int_array    input;
	int_array    output;
} calling_interface_buffer;

extern unsigned long long hex2int(const char *s);
extern uint32_t str2int(const char *s);
extern bool 	is_set(uint32_t num, uint8_t pos);
extern uint32_t get_byte(uint32_t num, uint8_t byte_pos);
extern int  	PrintSupportedThermModes();
extern int      GetCurrentThermalMode(const char *flag="default");
extern int  	SetCurrentThermalMode(const char *thermal_mode);
extern int      PrintBatteryChargingState();
