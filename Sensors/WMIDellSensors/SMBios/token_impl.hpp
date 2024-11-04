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

#include "token.hpp"
#include "types.hpp"

#include <stddef.h>

#define ERROR_BUFSIZE 1024

enum  // Smbios Structure types
{
    DellIndexedIoTokenType = 0xD4,
    DellProtectedAreaType1 = 0xD5,
    DellProtectedAreaType2 = 0xD6,
    DellCallingInterface   = 0xDA,
};

enum // Token types
{
    TokenTypeUnused = 0x0000,
    TokenTypeEOT = 0xffff,
};

struct token_obj
{
    int (*get_type)(const struct token_obj*);
    int (*get_id)(const struct token_obj*);
    int (*is_bool)(const struct token_obj*);
    int (*is_string)(const struct token_obj*);

    int (*is_active)(const struct token_obj*);
    int (*activate)(const struct token_obj*);

    char* (*get_string)(const struct token_obj*, size_t *len);
    int (*set_string)(const struct token_obj*, const char *, size_t size);

    int (*try_password)(const struct token_obj *, const char *ascii, const char *scancode);

    const char *(*strerror)(const struct token_obj*);

    const struct smbios_struct *smbios_structure;
    void *token_ptr;
    struct token_obj *next;
    char *errstring;
    void *private_data;
};

struct token_table
{
    int initialized;
    struct smbios_table *smbios_table;
    struct token_obj *list_head;
    char *errstring;
};

void add_token(struct token_table *t, struct token_obj *o);
int add_d4_tokens(struct token_table *t);
int add_da_tokens(struct token_table *t);


#pragma pack(push,1)

struct calling_interface_token
{
    u16 tokenId;
    u16 location;  // 0 for string tokens
    union {
        u16 value;
        u16 stringLength;
    };
};

struct calling_interface_structure
{ /* 0xDA structure */
    u8	     type;
    u8	     length;
    u16	     handle;

    u16      cmdIOAddress;
    u8       cmdIOCode;
    u32      supportedCmds;
    struct   calling_interface_token  tokens[];
};

enum  // protected value format types
{
    pvFmtAlphaNumericScanCode = 0,
    pvFmtAlphaNumericAscii    = 1,
    pvFmtAlphaNumericScanCodeNS = 2,
    pvFmtAlphaNumericAsciiNS   = 3,
};

#pragma pack(pop)

struct cmos_access_obj; // forward declare so we dont have to include cmos.h

u16 byteChecksum(const struct cmos_access_obj *c, u32 start, u32 end, u32 indexPort, u32 dataPort );
u16 wordChecksum(const struct cmos_access_obj *c, u32 start, u32 end, u32 indexPort, u32 dataPort);
u16 wordChecksum_n(const struct cmos_access_obj *c, u32 start, u32 end, u32 indexPort, u32 dataPort);
u16 wordCrc(const struct cmos_access_obj *c, u32 start, u32 end, u32 indexPort, u32 dataPort );
int update_checksum(const struct cmos_access_obj *c, bool do_update, void *userdata);

struct checksum_details
{
    u32 csumloc;
    u32 csumlen;
    u32 start;
    u32 end;
    u32 indexPort;
    u32 dataPort;
    u32 checkType;
    u16 (*csum_fn)(const struct cmos_access_obj *c, u32 start, u32 end, u32 indexPort, u32 dataPort );
};
