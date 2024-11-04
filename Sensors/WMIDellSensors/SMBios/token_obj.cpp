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
#include "smbios.hpp"
#include "token.hpp"
#include "types.hpp"

// private
#include "token_impl.hpp"
#include "token.hpp"

#include "Headers/kern_util.hpp"
#define kern_os_calloc(num, size) kern_os_malloc(num * size) // malloc bzeroes the buffer


// forward declarations
static int init_token_table(struct token_table *);
static void _token_table_free_tokens(struct token_table *table);

// static vars
static struct token_table singleton = {}; // auto-init to 0
static char *module_error_buf = nullptr; // auto-init to 0

static void return_mem(void)
{
    kern_os_free(module_error_buf);
    module_error_buf = 0;
}

char *token_get_module_error_buf()
{
    if (!module_error_buf)
	{
		module_error_buf = (char*)kern_os_calloc(1, ERROR_BUFSIZE);
		memset(module_error_buf, 0, ERROR_BUFSIZE);
	}
    return module_error_buf;
}

static void clear_err(const struct token_table *table)
{
    if(table && table->errstring)
        memset(table->errstring, 0, ERROR_BUFSIZE);
    if(module_error_buf)
        memset(module_error_buf, 0, ERROR_BUFSIZE);
}

struct token_table *token_table_factory(int flags, ...)
{
    struct token_table *toReturn = 0;
    int ret;

    if (flags==TOKEN_DEFAULTS)
        flags = TOKEN_GET_SINGLETON;

    if (flags & TOKEN_GET_SINGLETON)
        toReturn = &singleton;
    else
	{
		toReturn = (struct token_table*)kern_os_calloc(1, sizeof(struct token_table));
		memset(toReturn, 0, sizeof(struct token_table));
	}

    if (toReturn->initialized)
        goto out;

    ret = init_token_table(toReturn);
    if(ret == 0)
        goto out;

    // failed
    memset(toReturn, 0, sizeof(struct token_table));
    token_table_free(toReturn);
    toReturn = 0;

out:
    if (toReturn && ! (flags & TOKEN_NO_ERR_CLEAR))
        clear_err(toReturn);
    return toReturn;
}

void token_table_free(struct token_table *m)
{
    // can do special cleanup for singleton, but none necessary atm
    if (!m || m == &singleton)
        return;

    _token_table_free_tokens(m);

    kern_os_free(m->errstring);
    m->errstring = 0;

    kern_os_free(m);
}

const char * token_table_strerror(const struct token_table *table)
{
    const char * retval = 0;
    if (table)
        retval = table->errstring;
    else
        retval = module_error_buf;

    return retval;
}

const char * token_obj_strerror(const struct token_obj *tok)
{
    const char *ret = 0;
    if (tok)
        ret = tok->errstring;
    return ret;
}

const struct token_obj *token_table_get_next(const struct token_table *t, const struct token_obj *cur)
{
    if (!t)
        return 0;

    if (!cur)
        return t->list_head;

    return cur->next;
}

const struct token_obj *token_table_get_next_by_id(const struct token_table *t, const struct token_obj *cur, u16 id)
{
    do {
        cur = token_table_get_next(t, cur);
		//DBGLOG("sdell", "SMBIOS token_table_get_next_by_id: look for %d, got %d", id, token_obj_get_id(cur));
        if (cur && token_obj_get_id(cur) == id)
            break;
    } while ( cur );
    return cur;
}

int token_obj_get_type (const struct token_obj *t)
{
	int retval = 0;
	if (t && t->get_type) retval = t->get_type(t);
	return retval;
}

u16 token_obj_get_id (const struct token_obj *t)
{
	u16 retval = 0;
	if (t && t->get_id) retval = t->get_id(t);
	return retval;
}

int token_obj_is_active (const struct token_obj *t)
{
	int retval = -1;
	if (t && t->is_active) retval = t->is_active(t);
	return retval;
}

int token_obj_activate (const struct token_obj *t)
{
	int retval = -1;
	if (t && t->activate) retval = t->activate(t);
	return retval;
}

bool token_obj_is_bool (const struct token_obj *t)
{
	bool retval = 0;
	if (t && t->is_bool) retval = t->is_bool(t);
	return retval;
}

bool token_obj_is_string (const struct token_obj *t)
{
	bool retval = 0;
	if (t && t->is_string) retval = t->is_string(t);
	return retval;
}

char * token_obj_get_string (const struct token_obj *t, size_t *len)
{
    if (t && t->get_string && token_obj_is_string(t))
        return t-> get_string (t, len);
    return 0;
}

int token_obj_set_string(const struct token_obj *t, const char *newstr, size_t size)
{
    if (t && t->set_string && token_obj_is_string(t))
        return t->set_string (t, newstr, size);
    return 0;
}

int token_obj_try_password(const struct token_obj *t, const char *pass_ascii, const char *pass_scan)
{
    if (t && t->try_password)
        return t->try_password (t, pass_ascii, pass_scan);
    return 0;
}

const struct smbios_struct *token_obj_get_smbios_struct(const struct token_obj *t)
{
    if (t)
        return t->smbios_structure;
    return 0;
}

const void *token_obj_get_ptr(const struct token_obj *t)
{
    if (t)
        return t->token_ptr;
    return 0;
}


/**************************************************
 *
 * Internal functions
 *
 **************************************************/
static void _token_table_free_tokens(struct token_table *table)
{
    struct token_obj *ptr = table->list_head;
    struct token_obj *next = 0;

    while(ptr)
    {
        next = 0;
        if (ptr->next)
            next = ptr->next;

        // token_obj errstring generally points to token_table
        // errstring. dont free if this is the case
        if (ptr->errstring != table->errstring)
            kern_os_free(ptr->errstring);
        kern_os_free(ptr);
        ptr = next;
    }

	table->list_head = 0;
}

void add_token(struct token_table *t, struct token_obj *o)
{
    struct token_obj *ptr = t->list_head;

    while(ptr && ptr->next)
        ptr = ptr->next;

    if(ptr)
        ptr->next = o;
    else
        t->list_head = o;
}

int init_token_table(struct token_table *t)
{
    int retval = -1, ret;
    const char *error = "Failed to obtain smbios table.\n";
    struct smbios_table *table = smbios_table_factory(SMBIOS_GET_SINGLETON);
    char *errbuf;

    if(!table)
        goto out_tablefail;

    t->smbios_table = table;

    error = "Memory allocation failure allocating error string.\n";
    t->errstring = (char *)kern_os_calloc(1, ERROR_BUFSIZE);
    if (!t->errstring)
        goto out_allocfail;
	memset(t->errstring, 0, ERROR_BUFSIZE);

    error = "Error while trying to add 0xD4 tokens.\n";
    ret = add_d4_tokens(t);
    if (ret)
        goto out_tokenfail;

    ret = add_da_tokens(t);
    if (ret)
        goto out_tokenfail;

    t->initialized = 1;
    retval = 0;
    goto out;

out_tokenfail:
	SYSLOG("sdell", "SMBIOS init_token_table: out_tokenfail");
    _token_table_free_tokens(t);

out_allocfail:
	SYSLOG("sdell", "SMBIOS init_token_table: out_allocfail");
    smbios_table_free(table);

out_tablefail:
	SYSLOG("sdell", "SMBIOS init_token_table: out_tablefail");
    errbuf = token_get_module_error_buf();
    if (errbuf){
        lilu_os_strlcpy(errbuf, error, ERROR_BUFSIZE);
        if (t->errstring)
            lilu_os_strlcat(errbuf, t->errstring, ERROR_BUFSIZE);
        if (!table)
            lilu_os_strlcat(errbuf, smbios_table_strerror(0), ERROR_BUFSIZE); // yes, it is null
    }

out:
	SYSLOG("sdell", "SMBIOS init_token_table: out");
    return retval;
}


