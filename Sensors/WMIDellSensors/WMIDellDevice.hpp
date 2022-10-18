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

#include "wmi_defines.h"

struct extended_guid_block : public guid_block
{
	IOACPIPlatformDevice* device;
};

class WMIDellDevice
{
public:
	static WMIDellDevice* probe();
	bool evaluate(WMI_CLASS smi_class, WMI_SELECTOR select, const int_array args, int_array &res);
	
private:
	WMIDellDevice(const extended_guid_block *gblock);
	~WMIDellDevice();

	static bool parse_wdg(IOACPIPlatformDevice* device);
	static bool dell_wmi_descriptor_probe(const extended_guid_block *gblock);
	static const struct extended_guid_block* find_guid(const uid_arr &guid);

	
private:
	IOACPIPlatformDevice      *m_device {nullptr};
	const extended_guid_block *m_gblock {nullptr};
	dell_wmi_smbios_buffer    *m_buffer {nullptr};
	
	static constexpr const char*		 PnpDeviceIdAMW = "PNP0C14";
	static constexpr const size_t 		 max_guids {200};
	static evector<extended_guid_block&> guid_list;
	static size_t   					 buffer_size;
};

#endif /* WMIDellDevice_hpp */
