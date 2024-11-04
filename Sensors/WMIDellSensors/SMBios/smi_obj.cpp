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
#include "smi.hpp"
#include "smbios.hpp"

// private
#include "smi_impl.hpp"

#include <Headers/kern_util.hpp>
#define kern_os_calloc(num, size) kern_os_malloc(num * size) // malloc bzeroes the buffer

#include "../WMIDellDevice.hpp"


// static vars
static struct dell_smi_obj singleton = {}; // auto-init to 0
typedef int (*init_fn)(struct dell_smi_obj *);
static char *module_error_buf = nullptr; // auto-init to 0

static void return_mem(void)
{
    kern_os_free(module_error_buf);
    module_error_buf = 0;
}

/**************************************************
 *
 * Internal functions
 *
 **************************************************/

void _smi_free(struct dell_smi_obj *obj)
{
	obj->initialized=0;
	for (int i=0;i<4;++i)
	{
		kern_os_free(obj->physical_buffer[i]);
		obj->physical_buffer[i]=0;
		obj->physical_buffer_size[i] = 0;
	}
	kern_os_free(obj->errstring);
	obj->errstring = 0;
	kern_os_free(obj);
}


static char *smi_get_module_error_buf()
{
    if (!module_error_buf)
        module_error_buf = (char*)kern_os_calloc(1, ERROR_BUFSIZE);
    return module_error_buf;
}

static void clear_err(const struct dell_smi_obj *obj)
{
    if (obj && obj->errstring)
        memset(obj->errstring, 0, ERROR_BUFSIZE);
    if(module_error_buf)
        memset(module_error_buf, 0, ERROR_BUFSIZE);
}

struct dell_smi_obj *dell_smi_factory(int flags, ...)
{
    struct dell_smi_obj *toReturn = 0;
    int ret;

    if (flags==DELL_SMI_DEFAULTS)
        flags = DELL_SMI_GET_SINGLETON;

    if (flags & DELL_SMI_GET_SINGLETON)
        toReturn = &singleton;
    else
        toReturn = (struct dell_smi_obj *)kern_os_calloc(1, sizeof(struct dell_smi_obj));

    if (toReturn->initialized)
        goto out;

	SYSLOG("sdell", "SMBIOS dell_smi_factory: default init");
	ret = init_dell_smi_obj(toReturn);

    if (ret == 0)
        goto out;

    // failed
	SYSLOG("sdell", "SMBIOS dell_smi_factory: failed");
    toReturn->initialized = 0;
    toReturn = 0;

out:
    if (toReturn && ! (flags & DELL_SMI_NO_ERR_CLEAR))
        clear_err(toReturn);
    return toReturn;
}


void dell_smi_obj_free(struct dell_smi_obj *m)
{
    if (m && m != &singleton)
        _smi_free(m);
}

const char *dell_smi_obj_strerror(struct dell_smi_obj *s)
{
    const char * retval = 0;
    if (s)
        retval = s->errstring;
    else
        retval = module_error_buf;

	SYSLOG("sdell", "SMBIOS dell_smi_obj_strerror: error string: %s", retval);
    return retval;
}

void dell_smi_obj_set_class(struct dell_smi_obj *obj, u16 smi_class)
{
	SYSLOG("sdell", "SMBIOS dell_smi_obj_set_select: %d", smi_class);
    clear_err(obj);
    if(obj)
        obj->smi_buf.smi_class = smi_class;
}

void dell_smi_obj_set_select(struct dell_smi_obj *obj, u16 smi_select)
{
	SYSLOG("sdell", "SMBIOS dell_smi_obj_set_select: %d", smi_select);
    clear_err(obj);
    if(obj)
        obj->smi_buf.smi_select = smi_select;
}

void dell_smi_obj_set_arg(struct dell_smi_obj *obj, u8 argno, u32 value)
{
	SYSLOG("sdell", "SMBIOS dell_smi_obj_set_arg:%d -> 0x%x", argno, value);
    clear_err(obj);
    if(!obj) goto out;
    kern_os_free(obj->physical_buffer[argno]);
    obj->physical_buffer[argno] = 0;
    obj->physical_buffer_size[argno] = 0;

    obj->smi_buf.arg[argno] = value;
out:
    return;
}

u32  dell_smi_obj_get_res(struct dell_smi_obj *obj, u8 argno)
{
    clear_err(obj);
    u32 retval = 0;
    if (obj)
        retval = obj->smi_buf.res[argno];
	SYSLOG("sdell", "SMBIOS dell_smi_obj_get_res: %d = 0x%x", argno, retval);
    return retval;
}

static u8 * dell_smi_obj_make_buffer_X(struct dell_smi_obj *obj, u8 argno, size_t size)
{
    u8 *retval = 0;
    clear_err(obj);
    if (argno>3 || !obj)
        goto out;

    obj->smi_buf.arg[argno] = 0;
    kern_os_free(obj->physical_buffer[argno]);
    obj->physical_buffer[argno] = (unsigned char*)kern_os_calloc(1, size);
    obj->physical_buffer_size[argno] = size;
    retval = obj->physical_buffer[argno];
out:
    return retval;
}

const char *bufpat = "DSCI";
u8 * dell_smi_obj_make_buffer_frombios_withheader(struct dell_smi_obj *obj, u8 argno, size_t size)
{
    // allocate 4 extra bytes to hold size marker at the beginning
    u8 *buf = dell_smi_obj_make_buffer_X(obj, argno, size + sizeof(u32));
    if(buf)
    {
        // write buffer pattern
        for (unsigned int i=0; i<size+4; i++)
            buf[i] = bufpat[i%4];

        // write size of remaining bytes
        memcpy(buf, &size, sizeof(u32));
        buf += sizeof(u32);
    }
    return buf;
}

u8 * dell_smi_obj_make_buffer_frombios_withoutheader(struct dell_smi_obj *obj, u8 argno, size_t size)
{
    return dell_smi_obj_make_buffer_X(obj, argno, size);
}

u8 * dell_smi_obj_make_buffer_frombios_auto(struct dell_smi_obj *obj, u8 argno, size_t size)
{
    clear_err(obj);
    u8 smbios_ver = 1;
    u8 *retval = 0;
    // look in smbios struct 0xD0 (Revisions and IDs) to find the Dell SMBIOS implementation version
    //  offset 4 of the struct == dell major version
    struct smbios_struct *s = smbios_get_next_struct_by_type(0, 0xd0);
    smbios_struct_get_data(s, &(smbios_ver), 0x04, sizeof(u8));

	SYSLOG("sdell", "SMBIOS dell_smi_obj_make_buffer_frombios_auto: dell smbios ver: %d", smbios_ver);

    if (smbios_ver >= 2)
        retval = dell_smi_obj_make_buffer_frombios_withheader(obj, argno, size);
    else
        retval = dell_smi_obj_make_buffer_frombios_withoutheader(obj, argno, size);
    return retval;
}

u8 * dell_smi_obj_make_buffer_tobios(struct dell_smi_obj *obj, u8 argno, size_t size)
{
    return dell_smi_obj_make_buffer_X(obj, argno, size);
}


int dell_smi_obj_execute(struct dell_smi_obj *obj)
{
    clear_err(obj);
    int retval = -1;
    if(!obj)
        goto out;
    obj->smi_buf.res[0] = -3; //default to 'not handled'
    if (obj->execute)
        retval = obj->execute(obj);
out:
    return retval;
}

int init_dell_smi_obj_std(struct dell_smi_obj *obj)
{
    int retval = 0;
    char *errbuf = 0;

    const char *error = "Failed to find appropriate SMBIOS 0xD4 structure.\n";
    struct smbios_struct *s = smbios_get_next_struct_by_type(0, 0xda);
    if (s) {
        smbios_struct_get_data(s, &(obj->command_address), 4, sizeof(u16));
        smbios_struct_get_data(s, &(obj->command_code), 6, sizeof(u8));
    }
    else
        goto out_fail;

    error = "Failed to allocate memory for error string.\n";
    obj->errstring = (char*)kern_os_calloc(1, ERROR_BUFSIZE);
    if (!obj->errstring)
        goto out_fail;
	memset(obj->errstring, 0, ERROR_BUFSIZE);

    obj->initialized = 1;
    goto out;

out_fail:
	SYSLOG("sdell", "SMBIOS init_dell_smi_obj_std: out_fail");
    retval = -1;
    errbuf = smi_get_module_error_buf();
    if (errbuf) {
        char *smberr = smbios_strerror();
		SYSLOG("sdell", "SMBIOS init_dell_smi_obj_std: error: %s", error);
        lilu_os_strlcpy(errbuf, error, ERROR_BUFSIZE);
        if (smberr)
        {
			SYSLOG("sdell", "SMBIOS init_dell_smi_obj_std: smbios_strerror: %s", smberr);
            lilu_os_strlcat(errbuf, smberr, ERROR_BUFSIZE);
			kern_os_free(smberr);
        }
    }

out:
    return retval;
}

int LINUX_dell_wmi_obj_execute(struct dell_smi_obj *obj)
{
	if (obj->wmiDevice == nullptr)
	{
		SYSLOG("sdell", "SMBIOS LINUX_dell_wmi_obj_execute: there is no wmiDevice");
		return -1;
	}
	
	if (obj->wmiDevice->evaluate((WMI_CLASS)obj->smi_buf.smi_class,
							 (WMI_SELECTOR)obj->smi_buf.smi_select,
							 obj->smi_buf.arg, obj->smi_buf.res))
	{
		return 0;
	}
	return -1;
}

int init_dell_smi_obj(struct dell_smi_obj *obj)
{
	obj->execute = LINUX_dell_wmi_obj_execute;
	return init_dell_smi_obj_std(obj);
}


