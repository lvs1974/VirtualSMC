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


#define CMOS_DEFAULTS       0x0000
#define CMOS_GET_SINGLETON  0x0001
#define CMOS_GET_NEW        0x0002
#define CMOS_UNIT_TEST_MODE 0x0004
#define CMOS_NO_ERR_CLEAR   0x0008

// forward declaration to reduce header file deps
struct cmos_access_obj;

struct cmos_access_obj *cmos_obj_factory(int flags, ...);
void   cmos_obj_free(struct cmos_access_obj *);

int    cmos_obj_read_byte(const struct cmos_access_obj *, u8 *byte, u32 indexPort, u32 dataPort, u32 offset);
int    cmos_obj_write_byte(const struct cmos_access_obj *, u8 byte,  u32 indexPort, u32 dataPort, u32 offset);

// format error string
const char *cmos_obj_strerror(const struct cmos_access_obj *m);

// useful for checksums, etc
typedef int (*cmos_write_callback)(const struct cmos_access_obj *, bool, void *);
void cmos_obj_register_write_callback(struct cmos_access_obj *, cmos_write_callback, void *, void (*destruct)(void *));
int cmos_obj_run_callbacks(const struct cmos_access_obj *m, bool do_update);


/** Read byte from CMOS.
 *  @param byte  pointer to buffer were byte will be stored
 *  @param indexPort  the io port where we write the offset
 *  @param dataPort  the io port where we will read the resulting value
 *  @param offset  the offset into cmos (usually cmos is 256 byte banks)
 *  @return  0 on success, < 0 on failure
 */
int cmos_read_byte (u8 *byte, u32 indexPort, u32 dataPort, u32 offset);

/** Write byte to CMOS.
 *  @param byte  byte to write
 *  @param indexPort  the io port where we write the offset
 *  @param dataPort  the io port where we will write the byte
 *  @param offset  the offset into cmos (usually cmos is 256 byte banks)
 *  @return  0 on success, < 0 on failure
 */
int cmos_write_byte(u8 byte,  u32 indexPort, u32 dataPort, u32 offset);

/** Run all registered CMOS callbacks.
 * Higher layers can register callbacks that are run when any byte in CMOS is
 * changed. Presently, all these callbacks are used to update checksums in
 * CMOS.  If do_update is false, return code indicates if checksums are
 * currently correct.
 * @param do_update  should callback update checksum if it is wrong
 * @return The return value of all callbacks is 'or'-ed together. Checksum
 * callbacks return 0 if checksum is good and do_update is false. (otherwise
 * they just write the correct checksum)
 */
int cmos_run_callbacks(bool do_update);

/** Returns string describing the last error condition.
 * Can return 0. The buffer used is guaranteed to be valid until the next call
 * to any cmos_* function. Copy the contents if you need it longer.
 */
const char *cmos_strerror();

