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
#include "types.hpp"
#include "smbios.hpp"

// private
#include "smbios_impl.hpp"

#include <Headers/kern_util.hpp>
#define kern_os_calloc(num, size) kern_os_malloc(num * size) // malloc bzeroes the buffer

// forward declarations

// static vars
static struct smbios_table singleton = {};
static char *module_error_buf = nullptr;

static void return_mem(void)
{
    kern_os_free(module_error_buf);
    module_error_buf = 0;
}

char *smbios_get_module_error_buf()
{
    if (!module_error_buf)
        module_error_buf = (char *)kern_os_calloc(1, ERROR_BUFSIZE);
    return module_error_buf;
}

static void clear_err(const struct smbios_table *table)
{
    if (table && table->errstring)
        memset(table->errstring, 0, ERROR_BUFSIZE);
    if(module_error_buf)
        memset(module_error_buf, 0, ERROR_BUFSIZE);
}

smbios_table *smbios_table_factory(int flags, ...)
{
    struct smbios_table *toReturn = 0;
    int ret;

    if (flags==SMBIOS_DEFAULTS)
        flags = SMBIOS_GET_SINGLETON;

    if (flags & SMBIOS_GET_SINGLETON)
        toReturn = &singleton;
    else
        toReturn = (struct smbios_table *)kern_os_calloc(1, sizeof(struct smbios_table));

    if (toReturn->initialized)
        goto out;

    ret = init_smbios_struct(toReturn);
    if (ret)
        goto out_init_fail;

    //if (!(flags & SMBIOS_NO_FIXUPS))
    //    do_smbios_fixups(toReturn);

    goto out;

out_init_fail:
    // fail. init_smbios_* functions are responsible for free-ing memory if they
    // return failure.
	kern_os_free(toReturn);
    toReturn = 0;

out:
    if (toReturn && ! (flags & SMBIOS_NO_ERR_CLEAR))
        clear_err(toReturn);
    return toReturn;
}

void smbios_table_free(struct smbios_table *table)
{
    if (!table || table == &singleton)
        return;
    kern_os_free(table->table_path);

	kern_os_free(table->errstring);
	table->errstring = 0;

	kern_os_free(table->table);
	table->table = 0;

	table->initialized=0;

    memset(table, 0, sizeof(*table)); // big hammer
	kern_os_free(table);
}

const char *smbios_table_strerror(const struct smbios_table *m)
{
    const char * retval = 0;
    if (m)
        retval = m->errstring;
    else
        retval = module_error_buf;

    return retval;
}

struct smbios_struct *smbios_table_get_next_struct(const struct smbios_table *table, const struct smbios_struct *cur)
{
    clear_err(table);
    const u8 *data = 0;

    //If we are called on an uninitialized smbiosBuffer, return 0;
    if (!table || 0 == table->table || (cur && 0x7f == cur->type))
        goto out1;

    data = (u8*)table->table;

    // cur == 0, that means we return the first struct
    if (0 == cur)
        goto out1;

    // start out at the end of the cur structure.
    // The only things that sits between us and the next struct
    // are the strings for the cur structure.
    data = (const u8 *)(cur) + smbios_struct_get_length(cur);

    // skip past strings at the end of the formatted structure,
    // go until we hit double NULL "\0"
    // add a check to make sure we don't walk off the buffer end
    // for broken BIOSen.
    // The (3) is to take into account the deref at the end "data[0] ||
    // data[1]", and to take into account the "data += 2" on the next line.
    while (((data - (u8*)table->table) < (table->table_length - 3)) && (*data || data[1]))
        data++;

    // ok, skip past the actual double null.
    data += 2;

    // add code specifically to work around crap bios implementations
    // that do not have the _required_ 0x7f end-of-table entry
    //   note: (4) == sizeof a std header.
    if ( (data - (u8*)table->table) > (table->table_length - 4))
    {
        // really should output some nasty message here... This is very
        // broken
        data = 0;
        goto out1;
    }

out1:
    return (struct smbios_struct *)data;
}

struct smbios_struct *smbios_table_get_next_struct_by_type(const struct smbios_table *table, const struct smbios_struct *cur, u8 type)
{
    do {
        cur = smbios_table_get_next_struct(table, cur);
        if (cur && cur->type == type)
            break;
    } while ( cur );
    return (struct smbios_struct *)cur;
}

struct smbios_struct *smbios_table_get_next_struct_by_handle(const struct smbios_table *table, const struct smbios_struct *cur, u16 handle)
{
    do {
        cur = smbios_table_get_next_struct(table, cur);
        if (cur && cur->handle == handle)
            break;
    } while ( cur );
    return (struct smbios_struct *)cur;
}


u8 smbios_struct_get_type(const struct smbios_struct *s)
{
    if (s)
        return s->type;
    return 0;
}

u8 smbios_struct_get_length(const struct smbios_struct *s)
{
    if (s)
        return s->length;
    return 0;
}

u16 smbios_struct_get_handle(const struct smbios_struct *s)
{
    if (s)
        return s->handle;
    return 0;
}

int smbios_struct_get_data(const struct smbios_struct *s, void *dest, u8 offset, size_t len)
{
    int retval = -1;

	DBGLOG("sdell", "SMBIOS smbios_struct_get_data: (%p, %p, %d, %zd", s, dest, offset, len);

    if (!s)
        goto out;

    if (offset > smbios_struct_get_length(s))
        goto out;

    if( offset + len < offset ) // attempt to wraparound... :(
        goto out;

    if( offset + len > smbios_struct_get_length(s) )
        goto out;

    retval = 0;
    memcpy(dest, (const u8 *)(s)+offset, len);

out:
    return retval;
}


const char *smbios_struct_get_string_from_offset(const struct smbios_struct *s, u8 offset)
{
    u8 strnum = 0;
    const char *retval = 0;

	DBGLOG("sdell", "SMBIOS smbios_struct_get_string_from_offset()");

    if (!s)
        goto out;

    if (smbios_struct_get_data(s, &strnum, offset, sizeof(strnum)) >= 0)
    {
		DBGLOG("sdell", "SMBIOS smbios_struct_get_string_from_offset: string offset: %d  which: %d", offset, strnum);
        retval = smbios_struct_get_string_number(s, strnum);
    }

out:
	DBGLOG("sdell", "SMBIOS smbios_struct_get_string_from_offset: string %s", retval);
    return retval;
}

const char *smbios_struct_get_string_number(const struct smbios_struct *s, u8 which)
{
    const char *string_pointer = 0;
    const char *retval = 0;

	DBGLOG("sdell", "SMBIOS smbios_struct_get_string_number(%p, %d)", s, which);
	

    //strings are numbered beginning with 1
    // bail if passed null smbios_struct, or user tries to get string 0
    if (!which || !s)
        goto out;

    string_pointer = (const char *)(s);

    // start out at the end of the header. This is where
    // the first string starts
    string_pointer += smbios_struct_get_length(s);

    for (; which > 1; which--)
    {
        string_pointer += strlen (string_pointer) + 1;

        // if it is still '\0', that means we are
        // at the end of this item and should stop.
        // user gave us a bad index
        if( ! *string_pointer )
            goto out;
    }

    retval = string_pointer;


out:
    return retval;
}

// visitor pattern
void smbios_table_walk(struct smbios_table *table, void (*fn)(const struct smbios_struct *, void *userdata), void *userdata)
{
    clear_err(table);
    const struct smbios_struct *s = smbios_table_get_next_struct(table, 0);
    while(s) {
        fn(s, userdata);
        s = smbios_table_get_next_struct(table, s);
    };
}

int init_smbios_struct(struct smbios_table *m)
{
    char *errbuf = nullptr;
    const char *error = "Allocation error trying to allocate memory for error string. (ironic, yes?)";
    m->initialized = 1;
    m->errstring = (char *)kern_os_calloc(1, ERROR_BUFSIZE);
    if (!m->errstring)
        goto out_fail;
	memset(m->errstring, 0, ERROR_BUFSIZE);

    error = "Could not instantiate SMBIOS table. The errors from the low-level modules were:";

    // smbios firmware tables strategy
    if (smbios_get_table_firm_tables(m) >= 0)
        return 0;

    // smbios memory strategy
    //if (smbios_get_table_memory(m) >= 0)
    //    return 0;

    // fall through to failure...

out_fail:
    errbuf = smbios_get_module_error_buf();
     if (errbuf){
 		lilu_os_strlcpy(errbuf, error, ERROR_BUFSIZE);
         if (m->errstring)
 			lilu_os_strlcat(errbuf, m->errstring, ERROR_BUFSIZE);
     }
    smbios_table_free(m);
    return -1;
}


// validate the smbios table entry point
bool validate_dmi_tep(const struct dmi_table_entry_point *dmiTEP)
{
    // This code checks for the following:
    //       entry point structure checksum : As per the specs
    //       anchor string : As per the specs
    bool retval = true;

    u8 checksum = 0;
    const u8 *ptr = (const u8*)(dmiTEP);
    // don't overrun dmiTEP if BIOS is buggy... (note sizeof() test here)
    //      added especially to deal with buggy Intel BIOS.
    for( unsigned int i = 0; i < sizeof(*dmiTEP); ++i )
        // stupid stuff to avoid MVC++ .NET runtime exception check for cast to different size
        checksum = (checksum + ptr[i]) & 0xFF;

    if(memcmp(dmiTEP->anchor,"_DMI_",5)!=0) // Checking intermediate anchor string
        retval = false;  // validation failed

	SYSLOG("sdell", "SMBIOS validate_dmi_tep: TEP csum %d.", (int)checksum);
    if(checksum) // Checking entry point structure checksum
        retval = false;  // validation failed

    return retval;
}



// validate the smbios table entry point
bool smbios_verify_smbios(const char *buf, long length, long *dmi_length_out, u64 *dmi_table_address)
{
    struct smbios_table_entry_point *ep;
    bool retval = true;

    u8 checksum = 0;
    for(unsigned int i = 0; i < length ; ++i )
        checksum = (checksum + buf[i]) & 0xFF;

	SYSLOG("sdell", "SMBIOS smbios_verify_smbios: TEP csum %d.", (int)checksum);
    if(checksum) // Checking entry point structure checksum
        return false;  // validation failed

    ep = (struct smbios_table_entry_point*) buf;
    retval = validate_dmi_tep( &(ep->dmi));

    *dmi_length_out = ep->dmi.table_length;
	*dmi_table_address = ep->dmi.table_address;
	SYSLOG("sdell", "SMBIOS smbios_verify_smbios: Major version: %d Minor version: %d",  ep->major_ver, ep->minor_ver);

    return retval;
}

// validate the smbios3 table entry point
bool smbios_verify_smbios3(const char *buf, long length, long *dmi_length_out, u64 *dmi_table_address)
{
    struct smbios_table_entry_point_64 *ep;
    u8 checksum = 0;
    for(unsigned int i = 0; i < length ; ++i )
        checksum = (checksum + buf[i]) & 0xFF;

	SYSLOG("sdell", "SMBIOS smbios_verify_smbios3: TEP csum %d.", (int)checksum);
    if(checksum) // Checking entry point structure checksum
        return false;  // validation failed

    ep = (struct smbios_table_entry_point_64*) buf;
    *dmi_length_out = ep->structure_table_length;
	*dmi_table_address  = ep->structure_table_address;
	SYSLOG("sdell", "SMBIOS smbios_verify_smbios3: Major version: %d Minor version: %d",  ep->major_ver, ep->minor_ver);

    return true;
}

// visitor pattern
void smbios_walk(void (*smbios_walk_fn)(const struct smbios_struct *, void *userdata), void *userdata)
{
    struct smbios_table *table = smbios_table_factory(SMBIOS_DEFAULTS);
    smbios_table_walk(table, smbios_walk_fn, userdata);
    smbios_table_free(table);
}

// for looping/searching
struct smbios_struct *smbios_get_next_struct(const struct smbios_struct *cur)
{
    struct smbios_table *table = smbios_table_factory(SMBIOS_DEFAULTS);
    struct smbios_struct *ret = smbios_table_get_next_struct(table, cur);
    smbios_table_free(table);
    return ret;
}

struct smbios_struct *smbios_get_next_struct_by_type(const struct smbios_struct *cur, u8 type)
{
    struct smbios_table *table = smbios_table_factory(SMBIOS_DEFAULTS);
    struct smbios_struct *ret = smbios_table_get_next_struct_by_type(table, cur, type);
    smbios_table_free(table);
    return ret;
}

struct smbios_struct *smbios_get_next_struct_by_handle(const struct smbios_struct *cur, u16 handle)
{
    struct smbios_table *table = smbios_table_factory(SMBIOS_DEFAULTS);
    struct smbios_struct *ret = smbios_table_get_next_struct_by_handle(table, cur, handle);
    smbios_table_free(table);
    return ret;
}

static char *strdup(const char * string)
{
	char * result = NULL;
	size_t size;

	if (!string) {
		goto finish;
	}

	size = 1 + strlen(string);
	result = (char *)kern_os_malloc(size);
	if (!result) {
		goto finish;
	}

	memcpy(result, string, size);

finish:
	return result;
}

char *smbios_strerror()
{
    char *ret;
    struct smbios_table *table = smbios_table_factory(SMBIOS_DEFAULTS | SMBIOS_NO_ERR_CLEAR);
    if (table) {
        ret = strdup(smbios_table_strerror(table));
        smbios_table_free(table);
    } else {
        ret = NULL;
    }
    return ret;
}
