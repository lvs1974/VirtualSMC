//
//  WMIDellDevice.cpp
//  WMIDellSensors
//
//  Created by lvs1974 on 13/10/2022.
//  Copyright © 2022 lvs1974. All rights reserved.
//

#include "WMIDellDevice.hpp"


evector<extended_guid_block&> WMIDellDevice::guid_list;
size_t WMIDellDevice::buffer_size = 0;

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
	if (guid_list.size() == 0) {
		PANIC_COND(!guid_list.reserve(max_guids), "sdell", "evector::push_back has failed");
	}
		
	if (iterator) {
		while (auto entry = iterator->getNextObject()) {
			if (entry->compareName(pnp)) {
				auto device = OSDynamicCast(IOACPIPlatformDevice, entry);
				if (device != nullptr) {
					if (!parse_wdg(device))
						continue;
				}
			}
		}

		auto *gblock = find_guid(dell_wmi_desctiptor_guid);
		if (gblock != nullptr)
		{
			if (buffer_size == 0 && !dell_wmi_descriptor_probe(gblock)) {
				SYSLOG("sdell", "WMIDellDevice failed to retrieve wmi descriptor");
			}
			else if (buffer_size != 0) {
				auto *gblock = find_guid(dell_wmi_smbios_guid);
				if (gblock != nullptr)
				{
					if (gblock->flags & ACPI_WMI_METHOD)
					{
						DBGLOG("sdell", "found ACPI PNP %s", safeString(gblock->device->getName()));
						result = new WMIDellDevice(gblock);
					}
					else
					{
						SYSLOG("sdell", "WMIDellDevice failed to find correct DELL_WMI_SMBIOS_GUID");
					}
				}
				else
					SYSLOG("sdell", "DELL_WMI_SMBIOS_GUID could not be found");
			}
		}
		else
			SYSLOG("sdell", "DELL_WMI_DESCRIPTOR_GUID could not be found");

		iterator->release();
	} else {
		SYSLOG("sdell", "WMI device find failed to iterate over acpi");
	}

	pnp->release();
	return result;
}

WMIDellDevice::WMIDellDevice(const extended_guid_block *gblock) : m_device(gblock->device), m_gblock(gblock)
{
	m_buffer = reinterpret_cast<dell_wmi_smbios_buffer*>(Buffer::create<uint8_t>(buffer_size));
}

WMIDellDevice::~WMIDellDevice()
{
	Buffer::deleter(m_buffer);
}

bool WMIDellDevice::evaluate(WMI_CLASS smi_class, WMI_SELECTOR select, const int_array args, int_array &res)
{
	bool result = false;
	char method[5] = "WM";
	strncat(method, m_gblock->object_id, 2);
	OSObject *out = nullptr;
	OSObject *params[3];
	memset(m_buffer, 0, sizeof(dell_wmi_smbios_buffer));
	m_buffer->std.cmd_class  = smi_class;
	m_buffer->std.cmd_select = select;
	memcpy(m_buffer->std.input, args, sizeof(int_array));
	params[0] = OSNumber::withNumber(static_cast<unsigned long long>(0), 32);
	params[1] = OSNumber::withNumber(1UL, 32);
	params[2] = OSData::withBytes(&m_buffer->std, static_cast<unsigned int>(buffer_size - sizeof(uint64_t)));
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
				output->release();
			}
		}
	}
	for (auto *param : params)
		param->release();
		
	return result;
}

bool WMIDellDevice::parse_wdg(IOACPIPlatformDevice* device)
{
	bool result = false;
	OSObject *out = nullptr;
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
					extended_guid_block gb;
					memcpy(&gb, &gblock[i], sizeof(struct guid_block));
					gb.device = device;
					PANIC_COND(!guid_list.push_back(gb), "sdell", "evector::push_back has failed");
				}
				output->release();
			}
		}
	}
	
	return result;
}


/*
 * Descriptor buffer is 128 byte long and contains:
 *
 *       Name             Offset  Length  Value
 * Vendor Signature          0       4    "DELL"
 * Object Signature          4       4    " WMI"
 * WMI Interface Version     8       4    <version>
 * WMI buffer length        12       4    <length>
 * WMI hotfix number        16       4    <hotfix>
 */
bool WMIDellDevice::dell_wmi_descriptor_probe(const extended_guid_block *gblock)
{
	if (gblock->flags & (ACPI_WMI_EVENT | ACPI_WMI_METHOD)) {
		SYSLOG("sdell", "dell_wmi_get_size: block DELL_WMI_DESCRIPTOR_GUID has wrong flags");
		return false;
	}
	
	if (gblock->instance_count <= 0) {
		SYSLOG("sdell", "dell_wmi_get_size: block DELL_WMI_DESCRIPTOR_GUID has wrong instance");
		return false;
	}

	char method[5] = "WQ";
	char wc_method[5] = "WC";
	acpi_status wc_status = AE_ERROR;

	/*
	 * If ACPI_WMI_EXPENSIVE, call the relevant WCxx method first to
	 * enable collection.
	 */
	if (gblock->flags & ACPI_WMI_EXPENSIVE) {
		strncat(wc_method, gblock->object_id, 2);
		OSObject *param = OSNumber::withNumber(static_cast<unsigned long long>(0), 1);
		IOReturn ret = gblock->device->evaluateInteger(wc_method, &wc_status, &param, 1);
		param->release();
		if (ret != kIOReturnSuccess) {
			SYSLOG("sdell", "dell_wmi_get_size: evaluateInteger for method %s has failed with code 0x%x", wc_method, ret);
			return false;
		}
	}
	
	bool result = false;
	strncat(method, gblock->object_id, 2);
	OSObject *out = nullptr;
	OSObject *param = OSNumber::withNumber(static_cast<unsigned long long>(0), 0);
	IOReturn ret = gblock->device->evaluateObject(method, &out, &param, 1);
	if (ret != kIOReturnSuccess) {
		SYSLOG("sdell", "dell_wmi_get_size: evaluateObject %s has failed with code 0x%x", method, ret);
	}
	else if (out == nullptr) {
		SYSLOG("sdell", "dell_wmi_get_size: evaluateObject %s has failed since output buffer is null", method);
	}
	else {
		auto output = OSDynamicCast(OSData, out);
		if (output == nullptr) {
			SYSLOG("sdell", "dell_wmi_get_size: OSDynamicCast for out is null");
		} else {
			if (output->getLength() != 128) {
				SYSLOG("sdell", "dell_wmi_get_size: %s returned the wrong length: %d", method, output->getLength());
			}
			else {
				DBGLOG("sdell", "dell_wmi_get_size: buffer length is OK");
				if (strncmp(reinterpret_cast<const char*>(output->getBytesNoCopy()), "DELL WMI", 8) != 0) {
					SYSLOG("sdell", "dell_wmi_get_size: Dell descriptor buffer has invalid signature");
				}
				else {
					DBGLOG("sdell", "dell_wmi_get_size: Dell buffer signature is OK");
					const uint32_t *buffer = reinterpret_cast<const uint32_t*>(output->getBytesNoCopy());
					if (buffer[2] != 0 && buffer[2] != 1) {
						SYSLOG("sdell", "dell_wmi_get_size: Dell descriptor buffer has unknown version (%lu)\n", (unsigned long) buffer[2]);
					} else {
						result = true;
						buffer_size = buffer[3];
						DBGLOG("sdell", "Detected Dell WMI interface version %u, buffer size %lu, hotfix %u\n", buffer[2], buffer_size, buffer[4]);
						if (!buffer[4]) {
							SYSLOG("sdell","WMI SMBIOS userspace interface not supported(%u), try upgrading to a newer BIOS", buffer[4]);
						}
					}
				}
			}
			output->release();
		}
	}
	param->release();
	
	/*
	 * If ACPI_WMI_EXPENSIVE, call the relevant WCxx method, even if
	 * the WQxx method failed - we should disable collection anyway.
	 */
	if ((gblock->flags & ACPI_WMI_EXPENSIVE) && ACPI_SUCCESS(wc_status)) {
		OSObject *param = OSNumber::withNumber(static_cast<unsigned long long>(0), 0);
		IOReturn ret = gblock->device->evaluateInteger(wc_method, &wc_status, &param, 1);
		if (ret != kIOReturnSuccess) {
			SYSLOG("sdell", "dell_wmi_get_size: evaluateInteger %s has failed with code 0x%x", wc_method, ret);
		}
		param->release();
	}
	
	return result;
}

const struct extended_guid_block* WMIDellDevice::find_guid(const uid_arr &guid)
{
	for (size_t i = 0; i < guid_list.size(); i++) {
		auto &gblock = guid_list[i];
		if (memcmp(gblock.guid, guid, UUID_SIZE) == 0)
			return &gblock;
	}
	return nullptr;
}
