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

#include <stddef.h>

#include "types.hpp"
#include "smbios_impl.hpp"


#define DELL_SMI_DEFAULTS       0x0000
#define DELL_SMI_GET_SINGLETON  0x0001
#define DELL_SMI_GET_NEW        0x0002
#define DELL_SMI_UNIT_TEST_MODE 0x0004
#define DELL_SMI_NO_ERR_CLEAR   0x0008

struct dell_smi_obj;

// construct
struct dell_smi_obj *dell_smi_factory(int flags, ...);

// destruct
void dell_smi_obj_free(struct dell_smi_obj *);

const char *dell_smi_obj_strerror(struct dell_smi_obj *);

void dell_smi_obj_set_class(struct dell_smi_obj *, u16 );
void dell_smi_obj_set_select(struct dell_smi_obj *, u16 );
void dell_smi_obj_set_arg(struct dell_smi_obj *, u8 argno, u32 value);
u32  dell_smi_obj_get_res(struct dell_smi_obj *, u8 argno);
u8  *dell_smi_obj_make_buffer_frombios_auto(struct dell_smi_obj *, u8 argno, size_t size);
u8  *dell_smi_obj_make_buffer_frombios_withheader(struct dell_smi_obj *, u8 argno, size_t size);
u8  *dell_smi_obj_make_buffer_frombios_withoutheader(struct dell_smi_obj *, u8 argno, size_t size);
u8  *dell_smi_obj_make_buffer_tobios(struct dell_smi_obj *, u8 argno, size_t size);
int  dell_smi_obj_execute(struct dell_smi_obj *);


enum {
	cbARG1 = 0,
	cbARG2 = 1,
	cbARG3 = 2,
	cbARG4 = 3,
	cbRES1 = 0,
	cbRES2 = 1,
	cbRES3 = 2,
	cbRES4 = 3,
};

const char *dell_smi_strerror();

int dell_simple_ci_smi(u16 smiClass, u16 select, const u32 args[4], u32 res[4]);

// not yet implemented
int dell_adv_ci_smi(u16 smiClass, u16 select, const u32 args[4], u32 res[4], const u8 *buffer[4], const size_t buffer_size[4]);

int dell_smi_read_nv_storage         (u32 location, u32 *curValue, u32 *minValue, u32 *maxValue);
int dell_smi_read_battery_mode_setting(u32 location, u32 *curValue, u32 *minValue, u32 *maxValue);
int dell_smi_read_ac_mode_setting     (u32 location, u32 *curValue, u32 *minValue, u32 *maxValue);

int dell_smi_write_nv_storage         (u16 security_key, u32 location, u32 value, u32 *smiret);
int dell_smi_write_battery_mode_setting(u16 security_key, u32 location, u32 value, u32 *smiret);
int dell_smi_write_ac_mode_setting     (u16 security_key, u32 location, u32 value, u32 *smiret);

// password related functions
enum { DELL_SMI_PASSWORD_ANY = 0, DELL_SMI_PASSWORD_USER = 9, DELL_SMI_PASSWORD_ADMIN = 10, DELL_SMI_PASSWORD_OWNER = 12 };
enum { DELL_SMI_PASSWORD_FMT_SCANCODE = 0, DELL_SMI_PASSWORD_FMT_ASCII = 1 };
int dell_smi_password_format(int which);
int dell_smi_get_security_key(const char *password, u16 *security_key);
bool dell_smi_is_password_present(int which);
int dell_smi_password_verify(int which, const char *password);
int dell_smi_password_max_len(int which);
int dell_smi_password_min_len(int which);
int dell_smi_password_change(int which, const char *oldpass, const char *newpass);
