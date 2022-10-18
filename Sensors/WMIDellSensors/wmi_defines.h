//
//  wmi_defines.h
//  WMIDellSensors
//
//  Created by lvs1974 on 18/10/2022.
//  Copyright © 2022 lvs1974. All rights reserved.
//

#ifndef wmi_defines_h
#define wmi_defines_h

using acpi_status = uint32_t;	/* All ACPI Exceptions */

/*
 * If the GUID data block is marked as expensive, we must enable and
 * explicitily disable data collection.
 */
#define ACPI_WMI_EXPENSIVE   0x1
#define ACPI_WMI_METHOD      0x2        /* GUID is a method */
#define ACPI_WMI_STRING      0x4        /* GUID takes & returns a string */
#define ACPI_WMI_EVENT       0x8        /* GUID is an event */

#define DELL_WMI_DESCRIPTOR_GUID 		"8D9DDCBC-A997-11DA-B012-B622A1EF5492"
#define DELL_WMI_SMBIOS_GUID 			"A80593CE-A997-11DA-B012-B622A1EF5492"
/*
 * Exception code classes
 */
#define AE_CODE_ENVIRONMENTAL           0x0000	/* General ACPICA environment */
#define AE_CODE_PROGRAMMER              0x1000	/* External ACPICA interface caller */
#define AE_CODE_ACPI_TABLES             0x2000	/* ACPI tables */
#define AE_CODE_AML                     0x3000	/* From executing AML code */
#define AE_CODE_CONTROL                 0x4000	/* Internal control codes */

#define AE_CODE_MAX                     0x4000
#define AE_CODE_MASK                    0xF000

#define EXCEP_ENV(code)                 ((acpi_status) (code | AE_CODE_ENVIRONMENTAL))
#define AE_ERROR                        EXCEP_ENV (0x0001)
/*
 * Success is always zero, failure is non-zero
 */
#define ACPI_SUCCESS(a)                 (!(a))
#define ACPI_FAILURE(a)                 (a)

#define AE_OK                           (acpi_status) 0x0000

#define UUID_SIZE 16

using uid_arr = uint8_t[UUID_SIZE];

struct PACKED guid_block
{
	uid_arr guid;
	union {
			char object_id[2];
			struct {
					unsigned char notify_id;
					unsigned char reserved;
			};
	};
	uint8_t instance_count;
	uint8_t flags;
};

using int_array = uint32_t[4];

enum class WMI_CLASS : uint16_t
{
	TokenRead = 0,			// Read a token.
	TokenWrite = 1,			// Write a token.
	KeyboardBacklight = 4,	// For dealing with the keyboard backlight.
	Info = 17				// For getting or setting various information.
};

enum class WMI_SELECTOR : uint16_t
{
	Standard = 0,			// "Standard" tokens apply regardless of battery setting (?).
	Battery = 1,			// "Battery" token applies when the system is on battery power.
	VerifyPasswordOld = 1,	// Used to verify password, "old" method.
	AC = 2,					// "AC" token applies when the system is on AC power.
	PasswordProperties = 3,	// Used to get BIOS password information.
	VerifyPasswordNew = 4,	// Used to verify a password, "new" method.
	RfKill = 11,			// Used to enable or disable wireless connectivity.
	KeyboardBacklight = 11,	// Used to get or set keyboard backlight status.
	ThermalMode = 19		// For getting or setting "Thermal Mode".
};

// full list of tokens is located here: https://github.com/dell/libsmbios/blob/master/doc/token_list.csv
enum class WMI_TOKEN : uint32_t
{
	LcdBrightness = 0x007D,			 	// LCD brightness value.
	DiskControllerModeAhci = 0x0138, 	// Disk controller set to "AHCI" mode.
	DiskControllerModeRaid = 0x0139, 	// Disk controller set to "RAID" mode.
	KeyboardIlluminationOff = 0x01E1,	// Keyboard illumunation always off.
	KeyboardIlluminationOn = 0x01E2, 	// Keyboard illumination always on.
	KeyboardIlluminationAuto = 0x01E3,	// Keyboard illumination depending on ambient light level.
	KeyboardIlluminationAuto25 = 0x02EA,// Keyboard illumination level set to 25%.
	KeyboardIlluminationAuto50 = 0x02EB,// Keyboard illumination level set to 50%.
	KeyboardIlluminationAuto75 = 0x02EC,// Keyboard illumination level set to 75%.
	FanControlOverrideEnable = 0x02FD,	// Fans run at max speed.
	FanControlOverrideDisable = 0x02FE,	// Fans run at speed based on environment.
	KeyboardIlluminationAuto100 = 0x02F6,// Keyboard illumination level set to 100%.
	FanSpeedAuto = 0x0332,				// Fans run at speed based on environment.
	FanSpeedHigh = 0x0333,				// Fans run at a "high" level.
	FanSpeedMedium = 0x0334,			// Fans run at a "medium" level.
	FanSpeedLow = 0x0335,				// Fans run at a "low" level.
	FanSpeedMediumHigh = 0x0405,		// Fans run at a "medium high" level.
	FanSpeedMediumLow = 0x0406,			// Fans run at a "medium low" level.
	Fan1Rpm = 0xF600,					// Fetch the value of the first fan sensor.
	Fan2Rpm = 0xF610					// Fetch the value of the second fan sensor.
};

struct PACKED calling_interface_buffer
{
	WMI_CLASS    cmd_class;
	WMI_SELECTOR cmd_select;
	int_array    input;
	int_array    output;
};

static_assert(sizeof(calling_interface_buffer) == 36,  "Invalid calling_interface_buffer size, must be 36");

struct PACKED dell_wmi_extensions {
	uint32_t argattrib;
	uint32_t blength;
	uint8_t data[];
};

struct PACKED dell_wmi_smbios_buffer {
	uint64_t length;
	struct calling_interface_buffer std;
	struct dell_wmi_extensions ext;
};

static_assert((sizeof(dell_wmi_smbios_buffer) - 8) == 44,  "Invalid dell_wmi_smbios_buffer size, must be 41");




#endif /* wmi_defines_h */
