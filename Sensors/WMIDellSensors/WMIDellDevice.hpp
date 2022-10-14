//
//  WMIDellDevice.hpp
//  WMIDellSensors
//
//  Created by Sergey Lvov on 13/10/2022.
//  Copyright © 2022 vit9696. All rights reserved.
//

#ifndef WMIDellDevice_hpp
#define WMIDellDevice_hpp

#include <Headers/kern_util.hpp>
#include <Headers/kern_compat.hpp>

#include <VirtualSMCSDK/kern_vsmcapi.hpp>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <mach/mach_types.h>

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
	WMIDellDevice(IOACPIPlatformDevice* device, const guid_block *gblock);
	~WMIDellDevice();

	static bool parse_wdg(IOACPIPlatformDevice* device);
	static const struct guid_block* find_guid(const uid_arr &guid);

	
private:
	IOACPIPlatformDevice   *m_device {nullptr};
	const guid_block	   *m_gblock {nullptr};
	dell_wmi_smbios_buffer *m_buffer {nullptr};
	
	static constexpr const char *PnpDeviceIdAMW = "PNP0C14";
	static constexpr const size_t buffer_size = 32*1024;
	static constexpr const size_t max_guids {200};
	static evector<guid_block&> guid_list;
};

#endif /* WMIDellDevice_hpp */
