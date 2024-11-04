// vim:expandtab:autoindent:tabstop=4:shiftwidth=4:filetype=c:
/*
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

#pragma once

#include "types.hpp"

#define ERROR_BUFSIZE 1024

#define E_BLOCK_START 0xE0000UL
#define F_BLOCK_START 0xF0000UL
#define F_BLOCK_END   0xFFFFFUL

#pragma pack(push)
#pragma pack(1)
struct smbios_struct
{
    u8 type;
    u8 length;
    u16 handle;
};

struct dmi_table_entry_point
{
    u8 anchor[5];
    u8 checksum;
    u16 table_length;
    u32 table_address;
    u16 table_num_structs;
    u8 smbios_bcd_revision;
};

struct smbios_table_entry_point
{
    u8 anchor[4];
    u8 checksum;
    u8 eps_length;
    u8 major_ver;
    u8 minor_ver;
    u16 max_struct_size;
    u8 revision;
    u8 formatted_area[5];
    struct dmi_table_entry_point dmi;
    u8 padding_for_Intel_BIOS_bugs;
};

struct smbios_table_entry_point_64
{
    u8 anchor[5];
    u8 checksum;
    u8 eps_length;
    u8 major_ver;
    u8 minor_ver;
    u8 smbios_docrev;
    u8 entry_point_rev;
    u8 reserved;
    u32 structure_table_length;
    u64 structure_table_address;
};

struct smbios_table
{
    int initialized;
    char *table_path;
    struct table *table;
    long table_length;
    int last_errno;
    char *errstring;
};

#pragma pack(pop)

int  init_smbios_struct(struct smbios_table *m);
void  smbios_table_free(struct smbios_table *table);
bool  validate_dmi_tep(const struct dmi_table_entry_point *dmiTEP);
bool  smbios_verify_smbios(const char *buf, long length, long *dmi_length_out, u64 *dmi_table_address);
bool  smbios_verify_smbios3(const char *buf, long length, long *dmi_length_out, u64 *dmi_table_address);
int  smbios_get_table_firm_tables(struct smbios_table *m);
//int  smbios_get_table_memory(struct smbios_table *m);
const char *smbios_struct_get_string_number(const struct smbios_struct *s, u8 which);
u8 smbios_struct_get_length(const struct smbios_struct *s);
