/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim:expandtab:autoindent:tabstop=4:shiftwidth=4:filetype=c:cindent:textwidth=0:
 *
 * Copyright (C) 2005 Dell Inc.
 *  by Michael Brown <Michael_E_Brown@dell.com>
 * Licensed under the Open Software License version 2.1
 *
 * Alternatively, you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.

 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

// public
#include "types.hpp"
#include "smbios_impl.hpp"


#include <pexpert/i386/efi.h>

#include <Headers/kern_util.hpp>
#define kern_os_calloc(num, size) kern_os_malloc(num * size) // malloc bzeroes the buffer

extern void *gPEEFISystemTable;
extern uint64_t physmap_base, physmap_max;

static bool efi_compare_guids(const EFI_GUID &guid1, const EFI_GUID &guid2)
{
	return (bcmp(&guid1, &guid2, sizeof(EFI_GUID)) == 0) ? true : false;
}

#define VM_MIN_KERNEL_ADDRESS  ((vm_offset_t) 0xFFFFFF8000000000UL)
static inline uint64_t efi_efiboot_virtual_to_physical(uint64_t addr)
{
	if (addr >= VM_MIN_KERNEL_ADDRESS) {
		return addr & (0x40000000ULL - 1);
	} else {
		return addr;
	}
}

static  inline void *PHYSMAP_PTOV_check(void *paddr)
{
	uint64_t pvaddr = (uint64_t)paddr + physmap_base;

	if (__improbable(pvaddr >= physmap_max)) {
		panic("PHYSMAP_PTOV bounds exceeded, 0x%qx, 0x%qx, 0x%qx", pvaddr, physmap_base, physmap_max);
	}

	return (void *)pvaddr;
}
#define PHYSMAP_PTOV(x) (PHYSMAP_PTOV_check((void*) (x)))

static void * efi_efiboot_virtual_to_physmap_virtual(uint64_t addr)
{
	return PHYSMAP_PTOV(efi_efiboot_virtual_to_physical(addr));
}

#define SMBIOS_TABLE_GUID \
  {0xeb9d2d31,0x2d88,0x11d3,\
	{0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}}

#define SMBIOS3_TABLE_GUID \
  {0xf2fd1544, 0x9794, 0x4a2c,\
	{0x99,0x2e,0xe5,0xbb,0xcf,0x20,0xe3,0x94}}

int smbios_get_table_firm_tables(struct smbios_table *m)
{
    int retval = -1; //fail
    const char *error = "Could not open Table Entry Point.";
    char *entry_buffer = nullptr;
    long entry_length = 0;
	u64 table_address = 0;
	
	auto efiSystemTable = static_cast<EFI_SYSTEM_TABLE_64 *>(gPEEFISystemTable);
	if (efiSystemTable == nullptr)
		panic("efiSystemTable cannot be nullptr");
	
	SYSLOG("sdell", "SMBIOD smbios_get_table_firm_tables: detected %lld system tables", efiSystemTable->NumberOfTableEntries);
	auto cfgTable = static_cast<EFI_CONFIGURATION_TABLE_64 *>((void *)efiSystemTable->ConfigurationTable);
	SYSLOG("sdell", "SMBIOD smbios_get_table_firm_tables: ConfigurationTable = %p", (void *)(uintptr_t)efiSystemTable->ConfigurationTable);
	for (int i = 0; i < efiSystemTable->NumberOfTableEntries; i++)
	{
		auto cfg_table_entp = static_cast<EFI_CONFIGURATION_TABLE_64 *>(&cfgTable[i]);
		if (efi_compare_guids(cfg_table_entp->VendorGuid, SMBIOS3_TABLE_GUID) == true)
		{
			SYSLOG("sdell", "SMBIOD smbios_get_table_firm_tables: table %d is SMBIOS3 table", i);
			SYSLOG("sdell", "SMBIOD smbios_get_table_firm_tables: VendorTable = %p", (void *)(uintptr_t)cfg_table_entp->VendorTable);
			SYSLOG("sdell", "SMBIOD smbios_get_table_firm_tables: converted VendorTable = %p", (void *)(uintptr_t)efi_efiboot_virtual_to_physmap_virtual(cfg_table_entp->VendorTable));
			auto entry = reinterpret_cast<void *>(efi_efiboot_virtual_to_physmap_virtual(cfg_table_entp->VendorTable));
			entry_length = sizeof(smbios_table_entry_point_64);
			entry_buffer = reinterpret_cast<char*>(kern_os_malloc(entry_length));
			memcpy(entry_buffer, entry, entry_length);
			break;
		}
	}
	
	if (entry_buffer == nullptr)
	{
		for (int i = 0; i < efiSystemTable->NumberOfTableEntries; i++)
		{
			auto cfg_table_entp = static_cast<EFI_CONFIGURATION_TABLE_64 *>(&cfgTable[i]);
			if (efi_compare_guids(cfg_table_entp->VendorGuid, SMBIOS_TABLE_GUID) == true)
			{
				SYSLOG("sdell", "SMBIOD smbios_get_table_firm_tables: table %d is SMBIOS table", i);
				SYSLOG("sdell", "SMBIOD smbios_get_table_firm_tables: VendorTable = %p", (void *)(uintptr_t)cfg_table_entp->VendorTable);
				SYSLOG("sdell", "SMBIOD smbios_get_table_firm_tables: converted VendorTable = %p", (void *)(uintptr_t)efi_efiboot_virtual_to_physmap_virtual(cfg_table_entp->VendorTable));
				auto entry = reinterpret_cast<void *>(efi_efiboot_virtual_to_physmap_virtual(cfg_table_entp->VendorTable));
				entry_length = sizeof(smbios_table_entry_point);
				entry_buffer = reinterpret_cast<char*>(kern_os_malloc(entry_length));
				memcpy(entry_buffer, entry, entry_length);
				break;
			}
		}
	}

    if (entry_buffer == nullptr || entry_length ==0)
        goto out_err;

    error = "Invalid SMBIOS table signature";
    /* parse SMBIOS structure */
    if (memcmp (entry_buffer, "_SM_", 4) == 0) {
        if (!smbios_verify_smbios (entry_buffer, entry_length, &m->table_length, &table_address))
            goto out_free_entry_buffer;
    /* parse SMBIOS 3.0 structure */
    } else if (memcmp (entry_buffer, "_SM3_", 5) == 0) {
    if (!smbios_verify_smbios3 (entry_buffer, entry_length, &m->table_length, &table_address))
        goto out_free_entry_buffer;
    } else
        goto out_free_entry_buffer;

    error = "Could not read table from memory. ";
    m->table = (struct table*)kern_os_calloc(1, m->table_length);
    if (!m->table)
        goto out_free_entry_buffer;
	
	SYSLOG("sdell", "SMBIOD smbios_get_table_firm_tables: dmi table address = %p", (void *)(uintptr_t)table_address);
	SYSLOG("sdell", "SMBIOS smbios_get_table_firm_tables: converted dmi table address = %p", (void *)(uintptr_t)efi_efiboot_virtual_to_physmap_virtual(table_address));
	memcpy(m->table, reinterpret_cast<void *>(efi_efiboot_virtual_to_physmap_virtual(table_address)), m->table_length);
	retval = 0;
    goto out;

out_free_table:
	SYSLOG("sdell", "SMBIOS smbios_get_table_firm_tables: out_free_table");
	kern_os_free(m->table);
    m->table = 0;

out_free_entry_buffer:
	kern_os_free(entry_buffer);

out_err:
	SYSLOG("sdell", "SMBIOS smbios_get_table_firm_tables: out_err");
    if (strlen(m->errstring))
		lilu_os_strlcat(m->errstring, "\n", ERROR_BUFSIZE);
	lilu_os_strlcat (m->errstring, error, ERROR_BUFSIZE);
    return retval;

out:
	kern_os_free(entry_buffer);
	SYSLOG("sdell", "SMBIOS smbios_get_table_firm_tables: out: %d", retval);
    return retval;
}

/* this method is only designed to work with SMBIOS 2.0
   - it is also what pythong unit tests will use.
   - when the unit tests are changed over to use sysfs files
     then this method should also be dropped
*/
/*int smbios_get_tep_memory(struct smbios_table *table, u64 *address, long *length)
{
    int retval = 0;
	u64 table_address = 0;
    unsigned long fp = E_BLOCK_START;
    const char *errstring;
    u8 *block = reinterpret_cast<u8*>(kern_os_malloc(sizeof(struct smbios_table_entry_point)));
    if (!block)
        goto out_block;

    // tell the memory subsystem that it can optimize here and
    // keep memory open while we scan rather than open/close/open/close/...
    // for each fillBuffer() call
    memory_suggest_leave_open();
    errstring = "Could not read physical memory. Lowlevel error was:\n";
    while ( (fp + sizeof(struct smbios_table_entry_point)) < F_BLOCK_END)
    {
        int ret = memory_read(block, fp, sizeof(struct smbios_table_entry_point));
        if (ret)
            goto out_memerr;

        // look for SMBIOS 2.x style header
        if ((memcmp (block, "_SM_", 4) == 0))
        {
            struct smbios_table_entry_point *tempTEP = (struct smbios_table_entry_point *) block;
            errstring = "Found _SM_ anchor but could not parse SMBIOS structure.";
			DBGLOG("sdell", "Found _SM_ anchor. Trying to parse SMBIOS structure.");
            if(smbios_verify_smbios((char*) tempTEP, tempTEP->eps_length, length, &table_address)) {
                *address = tempTEP->dmi.table_address;
                break;
            }
        }

        fp += 16;
    }

    // dont need memory optimization anymore
    //memory_suggest_close();

    // bad stuff happened if we got to here and fp > 0xFFFFFL
    errstring = "Did not find smbios table entry point in memory.";
    if ((fp + sizeof(struct smbios_table_entry_point)) >= F_BLOCK_END)
        goto out_notfound;

    retval = 1;
    goto out;

out_memerr:
	SYSLOG("sdell", "SMBIOS: out_memerr: %s", errstring);
    strlcat (table->errstring, errstring, ERROR_BUFSIZE);
    strlcat (table->errstring, memory_strerror(), ERROR_BUFSIZE);
    goto out;

out_notfound:
	SYSLOG("sdell", "SMBIOS: out_notfound");
    strlcat (table->errstring, errstring, ERROR_BUFSIZE);
    goto out;

out_block:
	SYSLOG("sdell", "SMBIOS: out_block");
    return retval;

out:
	kern_os_free(block);
	SYSLOG("sdell", "SMBIOS: out");
    return retval;
}

int smbios_get_table_memory(struct smbios_table *m)
{
    int retval = -1; //fail
	const char *error = "Could not find Table Entry Point.";
    u64 address;

    if (!smbios_get_tep_memory(m, &address, &m->table_length))
        goto out_err;

    error = "Found table entry point but could not read table from memory. ";
    m->table = (struct table*)kern_os_calloc(1, m->table_length);
    retval = memory_read(m->table, address, m->table_length);
    if (retval != 0)
        goto out_free_table;

    goto out;
out_free_table:
	SYSLOG("sdell", "SMBIOS smbios_get_table_memory: out_free_table");
	kern_os_free(m->table);
    m->table = 0;
out_err:
	SYSLOG("sdell", "SMBIOS smbios_get_table_memory: out_err");
    if (strlen(m->errstring))
        strlcat(m->errstring, "\n", ERROR_BUFSIZE);
    strlcat (m->errstring, error, ERROR_BUFSIZE);
out:
	SYSLOG("sdell", "SMBIOS smbios_get_table_memory: out");
    return retval;
}*/
