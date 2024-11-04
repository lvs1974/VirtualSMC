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
#include <errno.h>

// public
#include "cmos.hpp"
#include "types.hpp"

// private
#include "cmos_impl.hpp"

#include "Headers/kern_util.hpp"
#include <architecture/i386/pio.h>

#define kern_os_calloc(num, size) kern_os_malloc(num * size) // malloc bzeroes the buffer

static int linux_read_fn(const struct cmos_access_obj *obj, u8 *byte, u32 indexPort, u32 dataPort, u32 offset)
{
	outb (offset, indexPort);
    *byte = (inb (dataPort));
	DBGLOG("sdell", "SMBIOS linux_read_fn: cmos read offset 0x%x = 0x%x", offset, *byte);
    return 0;
}

static int linux_write_fn(const struct cmos_access_obj *obj, u8 byte, u32 indexPort, u32 dataPort, u32 offset)
{
	DBGLOG("sdell", "SMBIOS linux_write_fn: cmos write: offset 0x%x = 0x%x", offset, byte);
	outb (offset, indexPort);
	outb (byte, dataPort);
    return 0;
}

int init_cmos_struct(struct cmos_access_obj *m)
{
    char * errbuf;
    int retval = 0;

    //if(iopl(3) < 0)
    //    goto out_noprivs;

    m->read_fn = linux_read_fn;
    m->write_fn = linux_write_fn;

    retval = _init_cmos_std_stuff(m);
    goto out;

out_noprivs:
	DBGLOG("sdell", "SMBIOS init_cmos_struct: out_noprivs");
    retval = -1;
    errbuf = cmos_get_module_error_buf();
    if (errbuf)
    {
        strlcpy(errbuf, "Error trying to raise IO Privilege level.\n", ERROR_BUFSIZE);
        strlcat(errbuf, "The OS Error string was: ", ERROR_BUFSIZE);
        //fixed_strerror(errno, errbuf, ERROR_BUFSIZE);
        strlcat(errbuf, "\n", ERROR_BUFSIZE);
    }
    // nothing left to free
    goto out;

out:
    return retval;
}
