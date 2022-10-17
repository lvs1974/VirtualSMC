//
//  WMIDellDevice.hpp
//  WMIDellSensors
//
//  Created by lvs1974 on 13/10/2022.
//  Copyright © 2022 lvs1974. All rights reserved.
//

#ifndef WMIDellDevice_hpp
#define WMIDellDevice_hpp

#include <Headers/kern_util.hpp>
#include <Headers/kern_compat.hpp>

#include <VirtualSMCSDK/kern_vsmcapi.hpp>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <mach/mach_types.h>

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

struct wrapped_guid_block : public guid_block
{
	IOACPIPlatformDevice* device;
};

using int_array = uint32_t[4];


struct PACKED calling_interface_buffer
{
	uint16_t cmd_class;
	uint16_t cmd_select;
	int_array input;
	int_array output;
};

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


class WMIDellDevice
{
public:
	static WMIDellDevice* probe();
	bool evaluate(uint16_t smi_class, uint16_t select, const int_array args, int_array &res);
	
private:
	WMIDellDevice(const wrapped_guid_block *gblock);
	~WMIDellDevice();

	static bool parse_wdg(IOACPIPlatformDevice* device);
	static bool dell_wmi_descriptor_probe(const wrapped_guid_block *gblock);
	static const struct wrapped_guid_block* find_guid(const uid_arr &guid);

	
private:
	IOACPIPlatformDevice     *m_device {nullptr};
	const wrapped_guid_block *m_gblock {nullptr};
	dell_wmi_smbios_buffer   *m_buffer {nullptr};
	
	static constexpr const char*		PnpDeviceIdAMW = "PNP0C14";
	static constexpr const size_t 		max_guids {200};
	static evector<wrapped_guid_block&> guid_list;
	static size_t   					buffer_size;
};

#endif /* WMIDellDevice_hpp */
