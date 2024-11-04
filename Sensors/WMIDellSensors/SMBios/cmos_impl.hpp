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

#include "cmos.hpp"
#include "types.hpp"


#define ERROR_BUFSIZE 1024

struct callback
{
    cmos_write_callback cb_fn;
    void *userdata;
    void (*destructor)(void *);
    struct callback *next;
};

struct cmos_access_obj
{
    int initialized;
    int (*read_fn)(const struct cmos_access_obj *m, u8 *byte, u32 indexPort, u32 dataPort, u32 offset);
    int (*write_fn)(const struct cmos_access_obj *m, u8 byte, u32 indexPort, u32 dataPort, u32 offset);
    void (*free)(struct cmos_access_obj *obj);
    void (*cleanup)(struct cmos_access_obj *obj); // called instead of ->free for singleton
    char *errstring;
    struct callback *cb_list_head;
    void *private_data;
    int write_lock;
};

// regular one
int init_cmos_struct(struct cmos_access_obj *m);
int _init_cmos_std_stuff(struct cmos_access_obj *m);  // base class constructor

// unit test one
int init_cmos_struct_filename(struct cmos_access_obj *m, const char *fn);

// other funcs
char *cmos_get_module_error_buf();

