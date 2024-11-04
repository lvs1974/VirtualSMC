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
#include <string.h>

// public
#include "cmos.hpp"
#include "types.hpp"

// private
#include "cmos_impl.hpp"

#include "Headers/kern_util.hpp"
#define kern_os_calloc(num, size) kern_os_malloc(num * size) // malloc bzeroes the buffer

struct cmos_access_obj singleton = {}; // auto-init to 0
static char *module_error_buf = nullptr; // auto-init to 0

static void return_mem(void)
{
    kern_os_free(module_error_buf);
    module_error_buf = 0;
}

char *cmos_get_module_error_buf()
{
    if (!module_error_buf)
        module_error_buf = (char*)kern_os_calloc(1, ERROR_BUFSIZE);
    return module_error_buf;
}

static void clear_err(const struct cmos_access_obj *obj)
{
    if (obj && obj->errstring)
        memset(obj->errstring, 0, ERROR_BUFSIZE);
    if(module_error_buf)
        memset(module_error_buf, 0, ERROR_BUFSIZE);
}

struct cmos_access_obj *cmos_obj_factory(int flags, ...)
{
    struct cmos_access_obj *toReturn = 0;
    int ret;

    if (flags==CMOS_DEFAULTS)
        flags = CMOS_GET_SINGLETON;

    if (flags & CMOS_GET_SINGLETON)
        toReturn = &singleton;
    else
        toReturn = (struct cmos_access_obj *)kern_os_calloc(1, sizeof(struct cmos_access_obj));

    if (toReturn->initialized)
        goto out;

	ret = init_cmos_struct(toReturn);

    if (ret==0)
        goto out;

    toReturn->initialized = 0;
    if (toReturn != &singleton)
        kern_os_free(toReturn);
    toReturn = 0;

out:
    if (toReturn  && ! (flags & CMOS_NO_ERR_CLEAR))
        clear_err(toReturn);
    return toReturn;
}

const char *cmos_obj_strerror(const struct cmos_access_obj *m)
{
    const char * retval = 0;
    if (m)
        retval = m->errstring;
    else
        retval = module_error_buf;
    return retval;
}

int  cmos_obj_read_byte(const struct cmos_access_obj *m, u8 *byte, u32 indexPort, u32 dataPort, u32 offset)
{
    clear_err(m);
    int retval = -6;  // bad *buffer ptr
    if (!byte)
        goto out;

    retval = -5; // bad memory_access_obj
    if (!m)
        goto out;

    retval = -7; // not implemented
    if (!m->read_fn)
        goto out;

    retval = m->read_fn(m, byte, indexPort, dataPort, offset);

out:
    return retval;
}

int  cmos_obj_write_byte(const struct cmos_access_obj *m, u8 byte, u32 indexPort, u32 dataPort, u32 offset)
{
    clear_err(m);

    int retval = -5; // bad memory_access_obj
    if (!m)
        goto out;

    retval = -7; // not implemented
    if (!m->write_fn)
        goto out;

    ((struct cmos_access_obj *)m)->write_lock++;
    retval = m->write_fn(m, byte, indexPort, dataPort, offset);
    if (m->write_lock == 1)
        cmos_obj_run_callbacks(m, true);
    ((struct cmos_access_obj *)m)->write_lock--;

out:
    return retval;
}

void cmos_obj_free(struct cmos_access_obj *m)
{
    struct callback *ptr = 0;
    struct callback *next = 0;

    if (!m)
        return;

    if(m->cleanup)
        m->cleanup(m);

    if (m == &singleton)
        return;

    ptr = m->cb_list_head;
    // free callback list
    while(ptr)
    {
        next = 0;
        if (ptr->next)
            next = ptr->next;

        if (ptr->destructor)
            ptr->destructor(ptr->userdata);
        kern_os_free(ptr);
        ptr = next;
    }

    m->cb_list_head = 0;

    kern_os_free(m->errstring);
    m->errstring=0;
    m->initialized=0;

    if(m->free)
        m->free(m);

    memset(m, 0, sizeof(*m)); // big hammer
    kern_os_free(m);
}

void cmos_obj_register_write_callback(struct cmos_access_obj *m, cmos_write_callback cb_fn, void *userdata, void (*destructor)(void *))
{
    clear_err(m);
    struct callback *ptr = 0;
    struct callback *new_cb = 0;

    if(!m || !cb_fn)
        goto out;

	SYSLOG("sdell", "SMBIOS cmos_obj_register_write_callback: loop");

    ptr = m->cb_list_head;
    while(ptr && ptr->next)
    {
        // dont add duplicates
        if (ptr->cb_fn == cb_fn && ptr->userdata == userdata)
            goto out;

        ptr = ptr->next;
    }

	SYSLOG("sdell", "SMBIOS cmos_obj_register_write_callback: allocate");
	new_cb = (struct callback *)kern_os_calloc(1, sizeof(struct callback));
	new_cb->cb_fn = cb_fn;
	new_cb->userdata = userdata;
	new_cb->destructor = destructor;
	new_cb->next = 0;

	SYSLOG("sdell", "SMBIOS cmos_obj_register_write_callback: join %p", ptr);
    if (ptr)
        ptr->next = new_cb;
    else
        m->cb_list_head = new_cb;

out:
    return;
}

int cmos_obj_run_callbacks(const struct cmos_access_obj *m, bool do_update)
{
    clear_err(m);
    int retval = -1;
    const struct callback *ptr = 0;

    if (!m)
        goto out;

    retval = 0;
    ptr = m->cb_list_head;
    if(!ptr)
        goto out;

    do{
		SYSLOG("sdell", "SMBIOS cmos_obj_run_callbacks: ptr->cb_fn %p", ptr->cb_fn);
        retval |= ptr->cb_fn(m, do_update, ptr->userdata);
        ptr = ptr->next;
    } while (ptr);

out:
    return retval;
}

int _init_cmos_std_stuff(struct cmos_access_obj *m)
{
    int retval = 0;
    m->initialized = 1;
    m->cb_list_head = 0;
    char * errbuf;

    // allocate space for error buffer now. Can optimize this later once api
    // settles. possibly only allocate on error (which would be problematic in
    // the case of error==out-of-mem)
    m->errstring = (char*)kern_os_calloc(1, ERROR_BUFSIZE);
    if (!m->errstring)
        goto out_allocfail;
	memset(m->errstring, 0, ERROR_BUFSIZE);
    goto out;

out_allocfail:
    // if any allocations failed, roll everything back. This should be safe.
	SYSLOG("sdell", "SMBIOS _init_cmos_std_stuff: out_allocfail");
    errbuf = cmos_get_module_error_buf();
    if (errbuf)
        strlcpy(errbuf, "There was an allocation failure while trying to construct the cmos object.", ERROR_BUFSIZE);
    retval = -1;

out:
    return retval;
}
