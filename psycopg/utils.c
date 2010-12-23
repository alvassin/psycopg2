/* utils.c - miscellaneous utility functions
 *
 * Copyright (C) 2008-2010 Federico Di Gregorio <fog@debian.org>
 *
 * This file is part of psycopg.
 *
 * psycopg2 is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link this program with the OpenSSL library (or with
 * modified versions of OpenSSL that use the same license as OpenSSL),
 * and distribute linked combinations including the two.
 *
 * You must obey the GNU Lesser General Public License in all respects for
 * all of the code used other than OpenSSL.
 *
 * psycopg2 is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 */

#define PSYCOPG_MODULE
#include "psycopg/psycopg.h"

#include "psycopg/connection.h"
#include "psycopg/pgtypes.h"

#include <string.h>
#include <stdlib.h>

char *
psycopg_escape_string(PyObject *obj, const char *from, Py_ssize_t len,
                       char *to, Py_ssize_t *tolen)
{
    Py_ssize_t ql;
    connectionObject *conn = (connectionObject*)obj;
    int eq = (conn && (conn->equote)) ? 1 : 0;   

    if (len == 0)
        len = strlen(from);
    
    if (to == NULL) {
        to = (char *)PyMem_Malloc((len * 2 + 4) * sizeof(char));
        if (to == NULL)
            return NULL;
    }

    {
        #if PG_VERSION_HEX >= 0x080104
            int err;
            if (conn && conn->pgconn)
                ql = PQescapeStringConn(conn->pgconn, to+eq+1, from, len, &err);
            else
        #endif
                ql = PQescapeString(to+eq+1, from, len);
    }

    if (eq)
        to[0] = 'E';
    to[eq] = '\''; 
    to[ql+eq+1] = '\'';
    to[ql+eq+2] = '\0';

    if (tolen)
        *tolen = ql+eq+2;
        
    return to;
}

/* Duplicate a string.
 *
 * Allocate a new buffer on the Python heap containing the new string.
 * 'len' is optional: if 0 the length is calculated.
 *
 * Return NULL and set an exception in case of error.
 */
char *
psycopg_strdup(const char *from, Py_ssize_t len)
{
    char *rv;

    if (!len) { len = strlen(from); }
    if (!(rv = PyMem_Malloc(len + 1))) {
        PyErr_NoMemory();
        return NULL;
    }
    strcpy(rv, from);
    return rv;
}

/* Ensure a Python object is a bytes string.
 *
 * Useful when a char * is required out of it.
 *
 * Return a new reference. NULL on error.
 */
PyObject *
psycopg_ensure_bytes(PyObject *obj)
{
    PyObject *rv = NULL;

    if (PyUnicode_CheckExact(obj)) {
        rv = PyUnicode_AsUTF8String(obj);
    }
    else if (Bytes_CheckExact(obj)) {
        Py_INCREF(obj);
        rv = obj;
    }
    else {
        PyErr_Format(PyExc_TypeError, "I'm not into ensuring %s as bytes",
            obj ? Py_TYPE(obj)->tp_name : "NULL");
    }

    return rv;
}

