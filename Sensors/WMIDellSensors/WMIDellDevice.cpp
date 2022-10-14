//
//  WMIDellDevice.cpp
//  WMIDellSensors
//
//  Created by Sergey Lvov on 13/10/2022.
//  Copyright © 2022 vit9696. All rights reserved.
//

#include "WMIDellDevice.hpp"


evector<guid_block&> WMIDellDevice::guid_list;

//
// Lame implementation just for use by strcasecmp/strncasecmp
//
static int tolower(unsigned char ch)
{
	if (ch >= 'A' && ch <= 'Z') {
		ch = 'a' + (ch - 'A');
	}
	return ch;
}

static int hex_to_bin(char ch)
{
	if ((ch >= '0') && (ch <= '9'))
		return ch - '0';
	ch = tolower(ch);
	if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	return -1;
}

static void guid_parse(const char *uuid, uid_arr u)
{
	static const uint8_t guid_index[16] = {3,2,1,0,5,4,7,6,8,9,10,11,12,13,14,15};
	static const uint8_t si[16] = {0,2,4,6,9,11,14,16,19,21,24,26,28,30,32,34};
	unsigned int i;

	for (i = 0; i < 16; i++) {
		int hi = hex_to_bin(uuid[si[i] + 0]);
		int lo = hex_to_bin(uuid[si[i] + 1]);
		u[guid_index[i]] = (hi << 4) | lo;
	}
}

WMIDellDevice* WMIDellDevice::probe()
{
	WMIDellDevice *result = nullptr;
	auto iterator = IORegistryIterator::iterateOver(gIOACPIPlane, kIORegistryIterateRecursively);
	auto pnp = OSString::withCString(PnpDeviceIdAMW);

	uid_arr dell_wmi_smbios_guid, dell_wmi_desctiptor_guid;
	guid_parse(DELL_WMI_SMBIOS_GUID, dell_wmi_smbios_guid);
	guid_parse(DELL_WMI_DESCRIPTOR_GUID, dell_wmi_desctiptor_guid);

	if (iterator) {
		while (auto entry = iterator->getNextObject()) {
			if (entry->compareName(pnp)) {
				auto device = OSDynamicCast(IOACPIPlatformDevice, entry);
				if (device != nullptr) {
					if (!parse_wdg(device))
						continue;
					if (result != nullptr)
						continue;
					auto *gblock = find_guid(dell_wmi_smbios_guid);
					if (gblock != nullptr)
					{
						if (gblock->flags & ACPI_WMI_METHOD)
						{
							DBGLOG("sdell", "found ACPI PNP AMW %s", safeString(entry->getName()));
							result = new WMIDellDevice(device, gblock);
						}
						else
						{
							SYSLOG("sdell", "WMIDellDevice failed to find correct DELL_WMI_SMBIOS_GUID");
						}
					}
				}
			}
		}
		
		if (result != nullptr && find_guid(dell_wmi_desctiptor_guid) == nullptr)
			SYSLOG("sdell", "WMIDellDevice failed find DELL_WMI_DESCRIPTOR_GUID (%s)", DELL_WMI_DESCRIPTOR_GUID);

		iterator->release();
	} else {
		SYSLOG("sdell", "AMW find failed to iterate over acpi");
	}

	pnp->release();
	return result;
}

WMIDellDevice::WMIDellDevice(IOACPIPlatformDevice* device, const guid_block *gblock) : m_device(device), m_gblock(gblock)
{
	m_buffer = reinterpret_cast<dell_wmi_smbios_buffer*>(Buffer::create<uint8_t>(buffer_size));
}

WMIDellDevice::~WMIDellDevice()
{
	Buffer::deleter(m_buffer);
}

bool WMIDellDevice::evaluate(uint16_t smi_class, uint16_t select, const int_array args, int_array &res)
{
	bool result = false;
	char method[5] = "WM";
	strncat(method, m_gblock->object_id, 2);
	OSObject *out = nullptr;
	OSObject *params[3];
	memset(m_buffer, 0, sizeof(dell_wmi_smbios_buffer));
	*m_buffer = {buffer_size, {smi_class, select, {}, {}}, {}};
	memcpy(m_buffer->std.input, args, sizeof(int_array));
	params[0] = OSNumber::withNumber(static_cast<unsigned long long>(0), 32);
	params[1] = OSNumber::withNumber(1UL, 32);
	params[2] = OSData::withBytes(&m_buffer->std, buffer_size - sizeof(uint64_t));
	IOReturn ret = m_device->evaluateObject(method, &out, params, 3);
	if (ret != kIOReturnSuccess)
		SYSLOG("sdell", "evaluate: evaluateObject for method %s has failed with code 0x%x", method, ret);
	else {
		if (out == nullptr) {
			SYSLOG("sdell", "evaluate: evaluateObject for method %s has failed since output buffer is null", method);
		}
		else {
			auto output = OSDynamicCast(OSData, out);
			if (output == nullptr) {
				SYSLOG("sdell", "evaluate: OSDynamicCast for out is null");
			} else {
				memcpy(&m_buffer->std, output->getBytesNoCopy(), output->getLength());
				memcpy(res, m_buffer->std.output, sizeof(int_array));
				result = true;
			}
			out->release();
		}
	}
	
	return result;
}

bool WMIDellDevice::parse_wdg(IOACPIPlatformDevice* device)
{
	bool result = false;
	OSObject *out = nullptr;
	PANIC_COND(!guid_list.reserve(max_guids), "sdell", "evector::push_back has failed");
	IOReturn ret = device->evaluateObject("_WDG", &out);
	if (ret != kIOReturnSuccess)
		SYSLOG("sdell", "parse_wdg: evaluateObject for method _WDG has failed with code 0x%x", ret);
	else {
		if (out == nullptr) {
			SYSLOG("sdell", "parse_wdg: evaluateObject _WDG has failed since output buffer is null");
		}
		else {
			auto output = OSDynamicCast(OSData, out);
			if (output == nullptr) {
				SYSLOG("sdell", "parse_wdg: OSDynamicCast for out is null");
			} else {
				auto total = output->getLength() / sizeof(struct guid_block);
				const struct guid_block *gblock = reinterpret_cast<const struct guid_block *>(output->getBytesNoCopy());
				result = (total != 0);
				for (auto i = 0; i < total; i++)
				{
					if (find_guid(gblock[i].guid) != nullptr)
						continue;
					guid_block gb;
					memcpy(&gb, &gblock[i], sizeof(struct guid_block));
					PANIC_COND(!guid_list.push_back(gb), "sdell", "evector::push_back has failed");
				}
			}
			out->release();
		}
	}
	
	return result;
}

const struct guid_block* WMIDellDevice::find_guid(const uid_arr &guid)
{
	for (size_t i = 0; i < guid_list.size(); i++) {
		auto &gblock = guid_list[i];
		if (memcmp(gblock.guid, guid, UUID_SIZE) == 0)
			return &gblock;
	}
	return nullptr;
}