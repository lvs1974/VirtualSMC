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
#include "cmos.hpp"
#include "smbios.hpp"
#include "types.hpp"

// private
#include "token_impl.hpp"
#include "smi.hpp"

#include <Headers/kern_util.hpp>
#define kern_os_calloc(num, size) kern_os_malloc(num * size) // malloc bzeroes the buffer

// helpers so we dont get line lengths from heck.
#define cast_token(t)  ((struct calling_interface_token *)(t->token_ptr))
#define cast_struct(t) ((struct calling_interface_structure *)token_obj_get_smbios_struct(t))

static int _da_get_type(const struct token_obj *t)
{
    return 0xDA;
}

static int _da_get_id(const struct token_obj *t)
{
    return cast_token(t)->tokenId;
}

// pretend DA tokens are bool.
// if user wants to get fancy, use the raw smi write_nv_data functions...

static int _da_is_bool(const struct token_obj *t)
{
    return true;
}

static int _da_is_string(const struct token_obj *t)
{
    return true;
}

static int _da_is_active(const struct token_obj *t)
{
	SYSLOG("sdell", "SMBIOS _da_is_active: token 0x%04x  location: 0x%04x  value 0x%04x", cast_token(t)->tokenId, cast_token(t)->location, cast_token(t)->value);
    int retval = 0;
    u32 curVal=0;
    int ret = dell_smi_read_nv_storage(cast_token(t)->location, &curVal, 0, 0);
    if (ret) {
        retval = ret;
		lilu_os_strlcpy( t->errstring, "Low level SMI call failed.\n", ERROR_BUFSIZE);
		lilu_os_strlcat( t->errstring, dell_smi_strerror() ? dell_smi_strerror() : "SMI could not initialized", ERROR_BUFSIZE );
        goto out;
    }

    if (cast_token(t)->value == curVal)
        retval = 1;

out:
    return retval;
}

// wonky to get around GCC error: "cast from pointer to integer of different size"
// as well as "warning: dereferencing type-punned pointer will break strict-aliasing rules"
// should be just security_key = (u16)t->private_data;
union void_u16 {
    void *ptr;
    u16   val;
};

static int _da_activate(const struct token_obj *t)
{
	SYSLOG("sdell", "SMBIOS _da_activate: token 0x%04x  location: 0x%04x  value 0x%04x", cast_token(t)->tokenId, cast_token(t)->location, cast_token(t)->value);
    // security key in private_data
    union void_u16 *indirect = (union void_u16*) &(t->private_data);
    int retval = dell_smi_write_nv_storage(indirect->val, cast_token(t)->location, cast_token(t)->value, 0);

    if (retval) {
		lilu_os_strlcpy( t->errstring, "Low level SMI call failed.\n", ERROR_BUFSIZE);
		lilu_os_strlcat( t->errstring, dell_smi_strerror() ? dell_smi_strerror() : "SMI could not initialized", ERROR_BUFSIZE );
    }

    return retval;
}

static char * _da_get_string(const struct token_obj *t, size_t *len)
{
    char *retval = 0;
    u32 toRead = 0;
	SYSLOG("sdell", "SMBIOS _da_get_string: token 0x%04x  location: 0x%04x  value 0x%04x", cast_token(t)->tokenId, cast_token(t)->location, cast_token(t)->value);
    int ret = dell_smi_read_nv_storage(cast_token(t)->location, &toRead, 0, 0);

    if (ret) {
		lilu_os_strlcpy( t->errstring, "Low level SMI call failed.\n", ERROR_BUFSIZE);
		lilu_os_strlcat( t->errstring, dell_smi_strerror() ? dell_smi_strerror() : "SMI could not initialized", ERROR_BUFSIZE );
        goto out;
    }

    if (len)
        *len = 2;

    retval = (char *)kern_os_calloc(1, sizeof(u16));
    memcpy(retval, &toRead, sizeof(u16));

out:
    return retval;
}

static int _da_set_string(const struct token_obj *t, const char *str, size_t size)
{
    // security key in private_data
    union void_u16 *indirect = (union void_u16*) &(t->private_data);
	SYSLOG("sdell", "SMBIOS _da_set_string: token 0x%04x  location: 0x%04x  value 0x%04x", cast_token(t)->tokenId, cast_token(t)->location, cast_token(t)->value);
 
    u16 toWrite = 0;
    if (str && size >= 2)
        toWrite = *(u16 *)str;
    else if (str && size >= 1)
        toWrite = *(u8 *)str;

	SYSLOG("sdell", "SMBIOS _da_set_string: setting string: 0x%04x", toWrite);


    int retval = dell_smi_write_nv_storage(indirect->val, cast_token(t)->location, toWrite, 0);

    if (retval) {
		lilu_os_strlcpy( t->errstring,  "Low level SMI call failed.\n", ERROR_BUFSIZE);
		lilu_os_strlcat( t->errstring, dell_smi_strerror() ? dell_smi_strerror() : "SMI could not initialized", ERROR_BUFSIZE );
    }

	SYSLOG("sdell", "SMBIOS _da_set_string: retval %d", retval);
    return retval;
}

static int _da_try_password(const struct token_obj *t, const char *pass_ascii, const char *pass_scan)
{
    union void_u16 *indirect = (union void_u16*) &(t->private_data);

    const char *whichpw = pass_scan;
    if (dell_smi_password_format(DELL_SMI_PASSWORD_ADMIN) == DELL_SMI_PASSWORD_FMT_ASCII)
        whichpw=pass_ascii;

	SYSLOG("sdell", "SMBIOS _da_try_password: current security key: %d", indirect->val);
    int ret = dell_smi_get_security_key(whichpw, &(indirect->val));
	SYSLOG("sdell", "SMBIOS _da_try_password:new security key: %d", indirect->val);
    return ret;
}

void init_da_token(struct token_table *table, struct token_obj *t)
{
    t->get_type = _da_get_type;
    t->get_id = _da_get_id;
    t->is_bool = _da_is_bool;
    t->is_string = _da_is_string;
    t->is_active = _da_is_active;
    t->activate = _da_activate;
    t->get_string = _da_get_string;
    t->set_string = _da_set_string;
    t->try_password = _da_try_password;
    t->private_data = 0;
    t->errstring = table->errstring;
}

int add_da_tokens(struct token_table *table)
{
    const char *error=0;
    int retval = 0;
    smbios_table_for_each_struct_type(table->smbios_table, s, 0xDA) {
        struct calling_interface_structure *da_struct = (struct calling_interface_structure*)s;
        struct calling_interface_token *token = da_struct->tokens;


        while (token->tokenId != TokenTypeEOT) {
            if (token->tokenId == TokenTypeUnused)
            {
                token++;
                continue;
            }

            if ( (void *)(token + 1) > (void *)(da_struct + da_struct->length))
            {
//                fnprintf("\n");
//                fnprintf("\n");
//                fnprintf("\n");
//                fnprintf("BIOS BUG ============================= BIOS BUG\n");
//                fnprintf("BIOS BUG ============================= BIOS BUG\n");
//                fnprintf("\n");
//                fnprintf("Ran off the end of the token table! %p  >  %p \n\n", token, da_struct + da_struct->length);
//                fnprintf("\n");
//                fnprintf("BIOS BUG ============================= BIOS BUG\n");
//                fnprintf("BIOS BUG ============================= BIOS BUG\n\n\n\n");
//                fnprintf("\n");
//                fnprintf("\n");
//                fnprintf("\n");
				SYSLOG("sdell", "SMBIOS add_da_tokens: BIOS BUG ============================= BIOS BUG");
                break;
            }
            error = "Allocation failure while trying to create token object.";
            struct token_obj *n = (struct token_obj *)kern_os_calloc(1, sizeof(struct token_obj));
            if (!n)
                goto out_err;

            n->token_ptr = token;
            n->smbios_structure = s;
            init_da_token(table, n);
            add_token(table, n);
            token++;
        }
    }
    goto out;
out_err:
    lilu_os_strlcat(table->errstring, error, ERROR_BUFSIZE);
    retval = -1;
out:
    return retval;
}



