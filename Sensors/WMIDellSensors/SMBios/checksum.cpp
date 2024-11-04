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

// system
//#include <stdlib.h>
//#include <string.h>

// public
#include "cmos.hpp"
#include "types.hpp"

// private
#include "token_impl.hpp"
#include "Headers/kern_util.hpp"
#define kern_os_calloc(num, size) kern_os_malloc(num * size) // malloc bzeroes the buffer


int update_checksum(const struct cmos_access_obj *c, bool do_update, void *userdata)
{
    int retval = -1;
    struct checksum_details *data = (struct checksum_details *)userdata;

	SYSLOG("sdell", "SMBIOS update_checksum: BEGIN: start 0x%x end 0x%x location 0x%x indexPort 0x%x", data->start, data->end, data->csumloc,  data->indexPort);

    u16 wordRetval = data->csum_fn(c, data->start, data->end, data->indexPort, data->dataPort);
    const u8 *csum = (const u8 *)(&wordRetval);

	SYSLOG("sdell", "SMBIOS update_checksum: calculated 0x%x", wordRetval);

    u32 actualcsum = 0;
    for( unsigned int i=0; i<data->csumlen; ++i )
    {
        u8 byte;
        int ret = cmos_obj_read_byte(c, &byte, data->indexPort, data->dataPort, data->csumloc+i);
        if (ret)
            goto out;

        actualcsum = (actualcsum << 8) | byte;
    }

	SYSLOG("sdell", "SMBIOS update_checksum: actual 0x%x (len %d)", actualcsum, data->csumlen);

#if 0
    u8 byteC = byteChecksum(c, data->start, data->end, data->indexPort, data->dataPort);
    u16 C = wordChecksum(c, data->start, data->end, data->indexPort, data->dataPort);
    u16 Cn = wordChecksum_n(c, data->start, data->end, data->indexPort, data->dataPort);
    u16 Crc = wordCrc(c, data->start, data->end, data->indexPort, data->dataPort);
    fnprintf(" byte(%x) wordcsum(%x) wordcsum_n(%x) wordCrc(%x)\n", byteC, C, Cn, Crc);
#endif

    if(do_update && actualcsum != wordRetval)
    {
        // write new checksum
		SYSLOG("sdell", "SMBIOS update_checksum: REWRITE CSUM");
        for( unsigned int i=0; i<data->csumlen; ++i )
        {
            int ret = cmos_obj_write_byte(c, data->indexPort, data->dataPort, data->csumloc+i, csum[data->csumlen -i -1]);
            if (ret)
                goto out;
        }
        // re-run callbacks since we may have written checksum in middle of another checksumed area
        cmos_obj_run_callbacks(c, do_update);
    }

    retval = 1;
    if (actualcsum != wordRetval)
        goto out;

    retval = 0;

out:
	SYSLOG("sdell", "SMBIOS update_checksum: END");
    return retval;
}

u16 byteChecksum(const struct cmos_access_obj *c, u32 start, u32 end, u32 indexPort, u32 dataPort )
{
    u8 running_checksum=0;
    u8 byte;
    for( u32 i = start; i <= end; i++) {
        if(cmos_obj_read_byte(c, &byte, indexPort, dataPort, i ))
            goto out;
        running_checksum += byte;
    }
out:
    return running_checksum;
}

u16 wordChecksum(const struct cmos_access_obj *c, u32 start, u32 end, u32 indexPort, u32 dataPort)
{
    u16 running_checksum=0;
    u8 byte;
    for( u32 i = start; i <= end; i++) {
        if(cmos_obj_read_byte(c, &byte, indexPort, dataPort, i ))
            goto out;
        running_checksum += byte;
    }
out:
    return running_checksum;
}

u16 wordChecksum_n(const struct cmos_access_obj *c, u32 start, u32 end, u32 indexPort, u32 dataPort)
{
    return (~wordChecksum(c, start, end, indexPort, dataPort)) + 1;
}

u16 wordCrc(const struct cmos_access_obj *c, u32 start, u32 end, u32 indexPort, u32 dataPort )
{
    u16 running_crc=0;
    u8 byte;

    for( u32 i = start; i <= end; i++)
    {
        if(cmos_obj_read_byte(c, &byte, indexPort, dataPort, i ))
            goto out;
        running_crc ^= byte;

        for( int j=0; j<7; j++ )
        {
            u16 temp = running_crc & 0x0001;
            running_crc >>= 1;
            if( temp != 0 )
            {
                running_crc |= 0x8000;
                running_crc ^= 0xA001;
            }
        }
    }
out:
    return running_crc;
}

