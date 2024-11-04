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

#include "smi.hpp"
#include "types.hpp"


#define KERNEL_SMI_MAGIC_NUMBER (0x534D4931)   /* "SMI1" */
#define DELL_CALLINTF_SMI_MAGIC_NUMBER   (0x42534931)  /* "BSI1" */

class WMIDellDevice;

enum {
    class_user_password =  9,
    class_admin_password = 10,
};

#pragma pack(push,1)
/* cut and paste from kernel sources */
struct callintf_cmd
{
    u32 magic;
    u32 ebx;
    u32 ecx;
    u16 command_address;
    u8  command_code;
    u8  reserved;
    u8  command_buffer_start[];
};


struct smi_cmd_buffer
{
    u16 smi_class;
    u16 smi_select;
    union {  /* to match BIOS docs, can use exact arg names specified in doc */
        u32	     arg[4];
        struct
        {
            u32 cbARG1;
            u32 cbARG2;
            u32 cbARG3;
            u32 cbARG4;
        };
    };
    union {  /* to match BIOS docs, can use exact res names specified in doc */
        u32      res[4];
        struct
        {
            s32 cbRES1;
            s32 cbRES2;
            s32 cbRES3;
            s32 cbRES4;
        };
    };
};

#pragma pack(pop)

#define ERROR_BUFSIZE 1024

struct dell_smi_obj
{
    int initialized;
    u16 command_address;
    u8  command_code;
    int (*execute)(struct dell_smi_obj *);
    struct smi_cmd_buffer smi_buf;
    u8 *physical_buffer[4];
    size_t physical_buffer_size[4];
    char *errstring;
	WMIDellDevice* wmiDevice;
};

int init_dell_smi_obj(struct dell_smi_obj *);
int init_dell_smi_obj_std(struct dell_smi_obj *);

