//-< DATABASE.CPP >--------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 14-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Database memory management, query execution, scheme evaluation
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "fastdb.h"
#include "compiler.h"
#include "symtab.h"
#include "hashtab.h"
#include "ttree.h"
#include "rtree.h"
#include <ctype.h>
#include <math.h>
#ifndef _WINCE
#include <sys/stat.h>
#endif
#include "os/OsSysLog.h"

BEGIN_FASTDB_NAMESPACE

dbNullReference null;

char const* const dbMetaTableName = "Metatable";

size_t dbDatabase::internalObjectSize[] = {
    0,
    dbPageSize,
    sizeof(dbTtree),
    sizeof(dbTtreeNode),
    sizeof(dbHashTable),
    sizeof(dbHashTableItem),
    sizeof(dbRtree),
    sizeof(dbRtreePage)
};

int dbDatabase::getVersion()
{
    return header->getVersion();
}

FixedSizeAllocator::FixedSizeAllocator()
{
    minSize = 0;
    maxSize = 0;
    bufSize = 0;
    quantum = 0;
    nChains = 0;
    chains = NULL;
    holes = NULL;
    vacant = NULL;
}

FixedSizeAllocator::~FixedSizeAllocator()
{
    TRACE_MSG(("hits=%ld, faults=%ld, retries=%ld\n", (long)hits, (long)faults, (long)retries));
    delete[] chains;
    delete[] holes;
}

void FixedSizeAllocator::reset()
{
    memset(chains, 0, sizeof(Hole*)*nChains);
    if (bufSize > 0) {
        for (size_t i = 1; i < bufSize; i++) {
            holes[i-1].next = &holes[i];
        }
        holes[bufSize-1].next = NULL;
    }
    vacant = holes;

    hits = 0;
    faults = 0;
    retries = 0;
}

void FixedSizeAllocator::init(size_t minSize, size_t maxSize, size_t quantum, size_t bufSize)
{
    delete[] chains;
    delete[] holes;
    this->minSize = minSize;
    this->maxSize = maxSize;
    this->quantum = quantum;
    this->bufSize = bufSize;
    nChains = (maxSize - minSize + quantum - 1) / quantum + 1;
    chains = new Hole*[nChains];
    holes = new Hole[bufSize];
    reset();
}


coord_t FASTDB_DLL_ENTRY distance(rectangle const& r, rectangle const& q)
{
    if (r & q) {
        return 0;
    }
    coord_t d = 0;;
    for (int i = 0; i < rectangle::dim; i++) {
        if (r.boundary[i] > q.boundary[rectangle::dim+i]) {
            coord_t di = r.boundary[i] - q.boundary[rectangle::dim+i];
            d += di*di;
        } else if (q.boundary[i] > r.boundary[rectangle::dim+i]) {
            coord_t di = q.boundary[i] - r.boundary[rectangle::dim+i];
            d += di*di;
        }
    }
    return (coord_t)sqrt((double)d);
}

inline void convertIntToString(dbInheritedAttribute&   iattr,
                               dbSynthesizedAttribute& sattr)
{
    char buf[32];
    sattr.array.size = sprintf(buf, INT8_FORMAT, sattr.ivalue) + 1;
    sattr.array.base = dbStringValue::create(buf, iattr);
}

inline void convertRealToString(dbInheritedAttribute&   iattr,
                                dbSynthesizedAttribute& sattr)
{
    char buf[32];
    sattr.array.size = sprintf(buf, "%f", sattr.fvalue) + 1;
    sattr.array.base = dbStringValue::create(buf, iattr);
}

inline void concatenateStrings(dbInheritedAttribute&   iattr,
                               dbSynthesizedAttribute& sattr,
                               dbSynthesizedAttribute& sattr2)
{
    char* str =
        dbStringValue::create(sattr.array.size + sattr.array.size - 1, iattr);
    memcpy(str, sattr.array.base, sattr.array.size-1);
    memcpy(str + sattr.array.size - 1, sattr2.array.base, sattr2.array.size);
    sattr.array.base = str;
    sattr.array.size += sattr2.array.size-1;
}

inline int compareStringsForEquality(dbSynthesizedAttribute& sattr1,
                                     dbSynthesizedAttribute& sattr2)
{
#ifdef IGNORE_CASE
    return stricmp(sattr1.array.base, sattr2.array.base);
#else
    return strcmp(sattr1.array.base, sattr2.array.base);
#endif
}

inline int compareStrings(dbSynthesizedAttribute& sattr1,
                          dbSynthesizedAttribute& sattr2)
{
#ifdef USE_LOCALE_SETTINGS
#ifdef IGNORE_CASE
    return stricoll(sattr1.array.base, sattr2.array.base);
#else
    return strcoll(sattr1.array.base, sattr2.array.base);
#endif
#else
#ifdef IGNORE_CASE
    return stricmp(sattr1.array.base, sattr2.array.base);
#else
    return strcmp(sattr1.array.base, sattr2.array.base);
#endif
#endif
}

#ifdef IGNORE_CASE
#define GET_CHAR(c) toupper((byte)(c))
#else
#define GET_CHAR(c) (c)
#endif

dbException::dbException(int p_err_code, char const* p_msg, int p_arg)
: err_code (p_err_code),
  msg (NULL),
  arg (p_arg)
{
    if (p_msg != NULL) {
        msg = new char[strlen(p_msg)+1];
        strcpy(msg, p_msg);
    }
}

dbException::dbException(dbException const& ex)
{
    err_code = ex.err_code;
    arg = ex.arg;
    if (ex.msg != NULL) {
        msg = new char[strlen(ex.msg)+1];
        strcpy(msg, ex.msg);
    } else {
        msg = NULL;
    }
}

dbException::~dbException() throw()
{
    delete[] msg;
}

const char* dbException::what() const throw()
{
    return getMsg();
}



inline bool matchStrings(dbSynthesizedAttribute& sattr1,
                         dbSynthesizedAttribute& sattr2,
                         char escapeChar)
{
    char *str = sattr1.array.base;
    char *pattern = sattr2.array.base;
    char *wildcard = NULL;
    char *strpos = NULL;
    while (true) {
        int ch = GET_CHAR(*str);
        if (*pattern == dbMatchAnySubstring) {
            wildcard = ++pattern;
            strpos = str;
        } else if (ch == '\0') {
            return (*pattern == '\0');
        } else if (*pattern == escapeChar && GET_CHAR(pattern[1]) == ch) {
            str += 1;
            pattern += 2;
        } else if (*pattern != escapeChar
                   && (ch == GET_CHAR(*pattern)
                       || *pattern == dbMatchAnyOneChar))
        {
            str += 1;
            pattern += 1;
        } else if (wildcard) {
            str = ++strpos;
            pattern = wildcard;
        } else {
            return false;
        }
    }
}

inline bool matchStrings(dbSynthesizedAttribute& sattr1,
                         dbSynthesizedAttribute& sattr2)
{
    char *str = sattr1.array.base;
    char *pattern = sattr2.array.base;
    char *wildcard = NULL;
    char *strpos = NULL;
    while (true) {
        int ch = GET_CHAR(*str);
        if (*pattern == dbMatchAnySubstring) {
            wildcard = ++pattern;
            strpos = str;
        } else if (ch == '\0') {
            return (*pattern == '\0');
        } else if (ch == GET_CHAR(*pattern) || *pattern == dbMatchAnyOneChar) {
            str += 1;
            pattern += 1;
        } else if (wildcard) {
            str = ++strpos;
            pattern = wildcard;
        } else {
            return false;
        }
    }
}


inline void lowercaseString(dbInheritedAttribute&   iattr,
                            dbSynthesizedAttribute& sattr)
{
    char *dst = dbStringValue::create(sattr.array.size, iattr);
    char *src = sattr.array.base;
    sattr.array.base = dst;
    while ((*dst++ = tolower(byte(*src++))) != '\0');
}

inline void uppercaseString(dbInheritedAttribute&   iattr,
                            dbSynthesizedAttribute& sattr)
{
    char *dst = dbStringValue::create(sattr.array.size, iattr);
    char *src = sattr.array.base;
    sattr.array.base = dst;
    while ((*dst++ = toupper(byte(*src++))) != '\0');
}

inline void copyString(dbInheritedAttribute&   iattr,
                       dbSynthesizedAttribute& sattr, char* str)
{
    sattr.array.base = dbStringValue::create(str, iattr);
    sattr.array.size = strlen(str) + 1;
    delete[] str;
}

inline void searchArrayOfBool(dbSynthesizedAttribute& sattr,
                              dbSynthesizedAttribute& sattr2)
{
    bool *p = (bool*)sattr2.array.base;
    int   n = sattr2.array.size;
    bool  v = (bool)sattr.bvalue;
    while (--n >= 0) {
        if (v == *p++) {
            sattr.bvalue = true;
            return;
        }
    }
    sattr.bvalue = false;
}

inline void searchArrayOfInt1(dbSynthesizedAttribute& sattr,
                              dbSynthesizedAttribute& sattr2)
{
    int1 *p = (int1*)sattr2.array.base;
    int   n = sattr2.array.size;
    int1  v = (int1)sattr.ivalue;
    while (--n >= 0) {
        if (v == *p++) {
            sattr.bvalue = true;
            return;
        }
    }
    sattr.bvalue = false;
}

inline void searchArrayOfInt2(dbSynthesizedAttribute& sattr,
                              dbSynthesizedAttribute& sattr2)
{
    int2 *p = (int2*)sattr2.array.base;
    int   n = sattr2.array.size;
    int2  v = (int2)sattr.ivalue;
    while (--n >= 0) {
        if (v == *p++) {
            sattr.bvalue = true;
            return;
        }
    }
    sattr.bvalue = false;
}

inline void searchArrayOfInt4(dbSynthesizedAttribute& sattr,
                              dbSynthesizedAttribute& sattr2)
{
    int4 *p = (int4*)sattr2.array.base;
    int   n = sattr2.array.size;
    int4  v = (int4)sattr.ivalue;
    while (--n >= 0) {
        if (v == *p++) {
            sattr.bvalue = true;
            return;
        }
    }
    sattr.bvalue = false;
}

inline void searchArrayOfInt8(dbSynthesizedAttribute& sattr,
                              dbSynthesizedAttribute& sattr2)
{
    db_int8 *p = (db_int8*)sattr2.array.base;
    int   n = sattr2.array.size;
    db_int8  v = sattr.ivalue;
    while (--n >= 0) {
        if (v == *p) {
            sattr.bvalue = true;
            return;
        }
        p += 1;
    }
    sattr.bvalue = false;
}

inline void searchArrayOfReal4(dbSynthesizedAttribute& sattr,
                              dbSynthesizedAttribute& sattr2)
{
    real4* p = (real4*)sattr2.array.base;
    int    n = sattr2.array.size;
    real4  v = (real4)sattr.fvalue;
    while (--n >= 0) {
        if (v == *p++) {
            sattr.bvalue = true;
            return;
        }
    }
    sattr.bvalue = false;
}

inline void searchArrayOfReal8(dbSynthesizedAttribute& sattr,
                               dbSynthesizedAttribute& sattr2)
{
    real8 *p = (real8*)sattr2.array.base;
    int    n = sattr2.array.size;
    real8  v = sattr.fvalue;
    while (--n >= 0) {
        if (v == *p) {
            sattr.bvalue = true;
            return;
        }
        p += 1;
    }
    sattr.bvalue = false;
}

inline void searchArrayOfReference(dbSynthesizedAttribute& sattr,
                                   dbSynthesizedAttribute& sattr2)
{
    oid_t *p = (oid_t*)sattr2.array.base;
    int    n = sattr2.array.size;
    oid_t  v = sattr.oid;
    while (--n >= 0) {
        if (v == *p) {
            sattr.bvalue = true;
            return;
        }
        p += 1;
    }
    sattr.bvalue = false;
}

inline void searchArrayOfRectangle(dbInheritedAttribute&   iattr,
                                   dbSynthesizedAttribute& sattr,
                                   dbSynthesizedAttribute& sattr2)
{
    rectangle *p = (rectangle*)sattr2.array.base;
    int        n = sattr2.array.size;
    rectangle  v = sattr.rvalue;
    sattr.bvalue = false;
    while (--n >= 0) {
        if (v == *p) {
            sattr.bvalue = true;
            break;
        }
        p += 1;
    }
}

inline void searchArrayOfString(dbSynthesizedAttribute& sattr,
                                dbSynthesizedAttribute& sattr2)
{
    dbVarying *p = (dbVarying*)sattr2.array.base;
    int        n = sattr2.array.size;
    char*      str = sattr.array.base;
    char*      base = (char*)sattr2.base;
    while (--n >= 0) {
        if (strcmp(base + p->offs, str) == 0) {
            sattr.bvalue = true;
            return;
        }
        p += 1;
    }
    sattr.bvalue = false;
}

inline void searchInString(dbSynthesizedAttribute& sattr,
                           dbSynthesizedAttribute& sattr2)
{
    if (sattr.array.size > sattr2.array.size) {
        sattr.bvalue = false;
    } else if (sattr2.array.size > dbBMsearchThreshold) {
        int len = sattr.array.size - 2;
        int n = sattr2.array.size - 1;
        int i, j, k;
        int shift[256];
        byte* pattern = (byte*)sattr.array.base;
        byte* str = (byte*)sattr2.array.base;
        for (i = 0; i < (int)itemsof(shift); i++) {
            shift[i] = len+1;
        }
        for (i = 0; i < len; i++) {
            shift[pattern[i]] = len-i;
        }
        for (i = len; i < n; i += shift[str[i]]) {
            j = len;
            k = i;
            while (pattern[j] == str[k]) {
                k -= 1;
                if (--j < 0) {
                    sattr.bvalue = true;
                    return;
                }
            }
        }
        sattr.bvalue = false;
    } else {
        sattr.bvalue = strstr(sattr2.array.base, sattr.array.base) != NULL;
    }
}

inline db_int8 powerIntInt(db_int8 x, db_int8 y)
{
    db_int8 res = 1;

    if (y < 0) {
        x = 1/x;
        y = -y;
    }
    while (y != 0) {
        if (y & 1) {
            res *= x;
        }
        x *= x;
        y >>= 1;
    }
    return res;
}

inline real8 powerRealInt(real8 x, db_int8 y)
{
    real8 res = 1.0;

    if (y < 0) {
        x = 1/x;
        y = -y;
    }
    while (y != 0) {
        if (y & 1) {
            res *= x;
        }
        x *= x;
        y >>= 1;
    }
    return res;
}

bool dbDatabase::evaluate(dbExprNode* expr, oid_t oid, dbTable* table, dbAnyCursor* cursor)
{
    dbInheritedAttribute iattr;
    dbSynthesizedAttribute sattr;
    iattr.db = this;
    iattr.oid = oid;
    iattr.table = table;
    iattr.record = (byte*)getRow(oid);
    iattr.paramBase = (size_t)cursor->paramBase;
    execute(expr, iattr, sattr);
    return sattr.bvalue != 0;
}

void _fastcall dbDatabase::execute(dbExprNode*             expr,
                                   dbInheritedAttribute&   iattr,
                                   dbSynthesizedAttribute& sattr)
{
    dbSynthesizedAttribute sattr2, sattr3;

    switch (expr->cop) {
      case dbvmVoid:
        sattr.bvalue = true; // empty condition
        return;
      case dbvmCurrent:
        sattr.oid = iattr.oid;
        return;
      case dbvmFirst:
        sattr.oid = iattr.table->firstRow;
        return;
      case dbvmLast:
        sattr.oid = iattr.table->lastRow;
        return;
      case dbvmLoadBool:
        execute(expr->operand[0], iattr, sattr);
        sattr.bvalue = *(bool*)(sattr.base+expr->offs);
        return;
      case dbvmLoadInt1:
        execute(expr->operand[0], iattr, sattr);
        sattr.ivalue = *(int1*)(sattr.base+expr->offs);
        return;
      case dbvmLoadInt2:
        execute(expr->operand[0], iattr, sattr);
        sattr.ivalue = *(int2*)(sattr.base+expr->offs);
        return;
      case dbvmLoadInt4:
        execute(expr->operand[0], iattr, sattr);
        sattr.ivalue = *(int4*)(sattr.base+expr->offs);
        return;
      case dbvmLoadInt8:
        execute(expr->operand[0], iattr, sattr);
        sattr.ivalue = *(db_int8*)(sattr.base+expr->offs);
        return;
      case dbvmLoadReal4:
        execute(expr->operand[0], iattr, sattr);
        sattr.fvalue = *(real4*)(sattr.base+expr->offs);
        return;
      case dbvmLoadReal8:
        execute(expr->operand[0], iattr, sattr);
        sattr.fvalue = *(real8*)(sattr.base+expr->offs);
        return;
      case dbvmLoadReference:
        execute(expr->operand[0], iattr, sattr);
        sattr.oid = *(oid_t*)(sattr.base+expr->offs);
        return;
      case dbvmLoadRectangle:
        execute(expr->operand[0], iattr, sattr);
        sattr.rvalue = *(rectangle*)(sattr.base+expr->offs);
        return;
      case dbvmLoadArray:
      case dbvmLoadString:
        execute(expr->operand[0], iattr, sattr2);
        sattr.array.base = (char*)sattr2.base
            + ((dbVarying*)(sattr2.base + expr->offs))->offs;
        sattr.array.size = ((dbVarying*)(sattr2.base + expr->offs))->size;
        return;
      case dbvmLoadRawBinary:
        execute(expr->operand[0], iattr, sattr);
        sattr.raw = (void*)(sattr.base+expr->offs);
        return;

      case dbvmLoadSelfBool:
        sattr.bvalue = *(bool*)(iattr.record+expr->offs);
        return;
      case dbvmLoadSelfInt1:
        sattr.ivalue = *(int1*)(iattr.record+expr->offs);
        return;
      case dbvmLoadSelfInt2:
        sattr.ivalue = *(int2*)(iattr.record+expr->offs);
        return;
      case dbvmLoadSelfInt4:
        sattr.ivalue = *(int4*)(iattr.record+expr->offs);
        return;
      case dbvmLoadSelfInt8:
        sattr.ivalue = *(db_int8*)(iattr.record+expr->offs);
        return;
      case dbvmLoadSelfReal4:
        sattr.fvalue = *(real4*)(iattr.record+expr->offs);
        return;
      case dbvmLoadSelfReal8:
        sattr.fvalue = *(real8*)(iattr.record+expr->offs);
        return;
      case dbvmLoadSelfReference:
        sattr.oid = *(oid_t*)(iattr.record+expr->offs);
        return;
      case dbvmLoadSelfRectangle:
        sattr.rvalue = *(rectangle*)(iattr.record+expr->offs);
        return;
      case dbvmLoadSelfArray:
      case dbvmLoadSelfString:
        sattr.array.base = (char*)iattr.record +
            ((dbVarying*)(iattr.record + expr->offs))->offs;
        sattr.array.size = ((dbVarying*)(iattr.record + expr->offs))->size;
        return;
      case dbvmLoadSelfRawBinary:
        sattr.raw = (void*)(iattr.record+expr->offs);
        return;

      case dbvmInvokeMethodBool:
        execute(expr->ref.base, iattr, sattr);
        expr->ref.field->method->invoke(sattr.base, &sattr.bvalue);
        sattr.bvalue = *(bool*)&sattr.bvalue;
        return;
      case dbvmInvokeMethodInt1:
        execute(expr->ref.base, iattr, sattr);
        expr->ref.field->method->invoke(sattr.base, &sattr.ivalue);
        sattr.ivalue = *(int1*)&sattr.ivalue;
        return;
      case dbvmInvokeMethodInt2:
        execute(expr->ref.base, iattr, sattr);
        expr->ref.field->method->invoke(sattr.base, &sattr.ivalue);
        sattr.ivalue = *(int2*)&sattr.ivalue;
        return;
      case dbvmInvokeMethodInt4:
        execute(expr->ref.base, iattr, sattr);
        expr->ref.field->method->invoke(sattr.base, &sattr.ivalue);
        sattr.ivalue = *(int4*)&sattr.ivalue;
        return;
      case dbvmInvokeMethodInt8:
        execute(expr->ref.base, iattr, sattr);
        expr->ref.field->method->invoke(sattr.base, &sattr.ivalue);
        return;
      case dbvmInvokeMethodReal4:
        execute(expr->ref.base, iattr, sattr);
        expr->ref.field->method->invoke(sattr.base, &sattr.fvalue);
        sattr.fvalue = *(real4*)&sattr.fvalue;
        return;
      case dbvmInvokeMethodReal8:
        execute(expr->ref.base, iattr, sattr);
        expr->ref.field->method->invoke(sattr.base, &sattr.fvalue);
        return;
      case dbvmInvokeMethodReference:
        execute(expr->ref.base, iattr, sattr);
        expr->ref.field->method->invoke(sattr.base, &sattr.oid);
        return;
      case dbvmInvokeMethodString:
        execute(expr->ref.base, iattr, sattr);
        expr->ref.field->method->invoke(sattr.base, &sattr2.array.base);
        sattr.array.size = strlen(sattr2.array.base) + 1;
        sattr.array.base = dbStringValue::create(sattr2.array.base, iattr);
        delete[] sattr2.array.base;
        return;

      case dbvmInvokeSelfMethodBool:
        expr->ref.field->method->invoke(iattr.record, &sattr.bvalue);
        sattr.bvalue = *(bool*)&sattr.bvalue;
        return;
      case dbvmInvokeSelfMethodInt1:
        expr->ref.field->method->invoke(iattr.record, &sattr.ivalue);
        sattr.ivalue = *(int1*)&sattr.ivalue;
        return;
      case dbvmInvokeSelfMethodInt2:
        expr->ref.field->method->invoke(iattr.record, &sattr.ivalue);
        sattr.ivalue = *(int2*)&sattr.ivalue;
        return;
      case dbvmInvokeSelfMethodInt4:
        expr->ref.field->method->invoke(iattr.record, &sattr.ivalue);
        sattr.ivalue = *(int4*)&sattr.ivalue;
        return;
      case dbvmInvokeSelfMethodInt8:
        expr->ref.field->method->invoke(iattr.record, &sattr.ivalue);
        return;
      case dbvmInvokeSelfMethodReal4:
        expr->ref.field->method->invoke(iattr.record, &sattr.fvalue);
        sattr.fvalue = *(real4*)&sattr.fvalue;
        return;
      case dbvmInvokeSelfMethodReal8:
        expr->ref.field->method->invoke(iattr.record, &sattr.fvalue);
        return;
      case dbvmInvokeSelfMethodReference:
        expr->ref.field->method->invoke(iattr.record, &sattr.oid);
        return;
      case dbvmInvokeSelfMethodString:
        expr->ref.field->method->invoke(iattr.record, &sattr2.array.base);
        sattr.array.size = strlen(sattr2.array.base) + 1;
        sattr.array.base = dbStringValue::create(sattr2.array.base, iattr);
        delete[] sattr2.array.base;
        return;

      case dbvmLength:
        execute(expr->operand[0], iattr, sattr);
        sattr.ivalue = sattr.array.size;
        return;
      case dbvmStringLength:
        execute(expr->operand[0], iattr, sattr);
        sattr.ivalue = sattr.array.size - 1;
        return;

      case dbvmGetAt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if ((nat8)sattr2.ivalue >= (nat8)sattr.array.size) {
            if (expr->operand[1]->cop == dbvmVariable) {
                longjmp(iattr.exists_iterator[expr->operand[1]->offs].unwind, 1);
            }
            iattr.removeTemporaries();
            iattr.db->handleError(IndexOutOfRangeError, NULL,
                                  int(sattr2.ivalue));
        }
        sattr.base = (byte*)sattr.array.base + int(sattr2.ivalue)*expr->offs;
        return;
      case dbvmRectangleCoord:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if ((nat8)sattr2.ivalue >= rectangle::dim*2) {
            if (expr->operand[1]->cop == dbvmVariable) {
                longjmp(iattr.exists_iterator[expr->operand[1]->offs].unwind, 1);
            }
            iattr.removeTemporaries();
            iattr.db->handleError(IndexOutOfRangeError, NULL,
                                  int(sattr2.ivalue));
        }
        sattr.fvalue = sattr.rvalue.boundary[int(sattr2.ivalue)];
        return;
      case dbvmCharAt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if ((nat8)sattr2.ivalue >= (nat8)(sattr.array.size-1)) {
            if (expr->operand[1]->cop == dbvmVariable) {
                longjmp(iattr.exists_iterator[expr->operand[1]->offs].unwind, 1);
            }
            iattr.removeTemporaries();
            iattr.db->handleError(IndexOutOfRangeError, NULL,
                                  int(sattr2.ivalue));
        }
        sattr.ivalue = (byte)sattr.array.base[int(sattr2.ivalue)];
        return;

      case dbvmExists:
        iattr.exists_iterator[expr->offs].index = 0;
        if (setjmp(iattr.exists_iterator[expr->offs].unwind) == 0) {
            do {
                execute(expr->operand[0], iattr, sattr);
                iattr.exists_iterator[expr->offs].index += 1;
            } while (!sattr.bvalue);
        } else {
            sattr.bvalue = false;
        }
        return;

      case dbvmVariable:
        sattr.ivalue = iattr.exists_iterator[expr->offs].index;
        return;

      case dbvmLoadVarBool:
        sattr.bvalue = *(bool*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarInt1:
        sattr.ivalue = *(int1*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarInt2:
        sattr.ivalue = *(int2*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarInt4:
        sattr.ivalue = *(int4*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarInt8:
        sattr.ivalue = *(db_int8*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarReal4:
        sattr.fvalue = *(real4*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarReal8:
        sattr.fvalue = *(real8*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarReference:
        sattr.oid = *(oid_t*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarRectangle:
        sattr.rvalue = *(rectangle*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarString:
        sattr.array.base = (char*)((char*)expr->var + iattr.paramBase);
        sattr.array.size = strlen((char*)sattr.array.base) + 1;
        return;
      case dbvmLoadVarStringPtr:
        sattr.array.base = *(char**)((char*)expr->var + iattr.paramBase);
        sattr.array.size = strlen((char*)sattr.array.base) + 1;
        return;
      case dbvmLoadVarArrayOfOid:
      case dbvmLoadVarArrayOfInt4:
      case dbvmLoadVarArrayOfInt8:
        sattr.array.base = (char*)((dbAnyArray*)((char*)expr->var + iattr.paramBase))->base();
        sattr.array.size = ((dbAnyArray*)((char*)expr->var + iattr.paramBase))->length();
        return;
      case dbvmLoadVarArrayOfOidPtr:
      case dbvmLoadVarArrayOfInt4Ptr:
      case dbvmLoadVarArrayOfInt8Ptr:
        {
            dbAnyArray* arr = *(dbAnyArray**)((char*)expr->var + iattr.paramBase);
            sattr.array.base = (char*)arr->base();
            sattr.array.size = arr->length();
            return;
        }
      case dbvmLoadVarRawBinary:
        sattr.raw = (void*)((char*)expr->var + iattr.paramBase);
        return;

#ifdef USE_STD_STRING
      case dbvmLoadVarStdString:
        sattr.array.base = (char*)((std::string*)((char*)expr->var + iattr.paramBase))->c_str();
        sattr.array.size = ((std::string*)((char*)expr->var + iattr.paramBase))->length() + 1;
        return;
#endif

      case dbvmLoadTrue:
        sattr.bvalue = true;
        return;
      case dbvmLoadFalse:
        sattr.bvalue = false;
        return;
      case dbvmLoadNull:
        sattr.oid = 0;
        return;
      case dbvmLoadIntConstant:
        sattr.ivalue = expr->ivalue;
        return;
      case dbvmLoadRealConstant:
        sattr.fvalue = expr->fvalue;
        return;
      case dbvmLoadRectangleConstant:
        sattr.rvalue = expr->rvalue;
        return;
      case dbvmLoadStringConstant:
        sattr.array.base = expr->svalue.str;
        sattr.array.size = expr->svalue.len;
        return;

      case dbvmOrBool:
        execute(expr->operand[0], iattr, sattr);
        if (sattr.bvalue == 0) {
            execute(expr->operand[1], iattr, sattr);
        }
        return;
      case dbvmAndBool:
        execute(expr->operand[0], iattr, sattr);
        if (sattr.bvalue != 0) {
            execute(expr->operand[1], iattr, sattr);
        }
        return;
      case dbvmNotBool:
        execute(expr->operand[0], iattr, sattr);
        sattr.bvalue = !sattr.bvalue;
        return;

      case dbvmIsNull:
        execute(expr->operand[0], iattr, sattr);
        sattr.bvalue = sattr.oid == 0;
        return;

      case dbvmAddRectangle:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.rvalue += sattr2.rvalue;
        return;

      case dbvmNegInt:
        execute(expr->operand[0], iattr, sattr);
        sattr.ivalue = -sattr.ivalue;
        return;
      case dbvmAddInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.ivalue += sattr2.ivalue;
        return;
      case dbvmSubInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.ivalue -= sattr2.ivalue;
        return;
      case dbvmMulInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.ivalue *= sattr2.ivalue;
        return;
      case dbvmDivInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if (sattr2.ivalue == 0) {
            iattr.removeTemporaries();
            iattr.db->handleError(ArithmeticError, "Division by zero");
        } else {
            sattr.ivalue /= sattr2.ivalue;
        }
        return;
      case dbvmAndInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.ivalue &= sattr2.ivalue;
        return;
      case dbvmOrInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.ivalue |= sattr2.ivalue;
        return;
      case dbvmNotInt:
        execute(expr->operand[0], iattr, sattr);
        sattr.ivalue = ~sattr.ivalue;
        return;
      case dbvmAbsInt:
        execute(expr->operand[0], iattr, sattr);
        if (sattr.ivalue < 0) {
            sattr.ivalue = -sattr.ivalue;
        }
        return;
      case dbvmPowerInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if (sattr.ivalue == 2) {
            sattr.ivalue = sattr2.ivalue < 64
                ? (nat8)1 << (int)sattr2.ivalue : 0;
        } else if (sattr.ivalue == 0 && sattr2.ivalue < 0) {
            iattr.removeTemporaries();
            iattr.db->handleError(ArithmeticError,
                                  "Raise zero to negative power");
        } else {
            sattr.ivalue = powerIntInt(sattr.ivalue, sattr2.ivalue);
        }
        return;


      case dbvmEqInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.ivalue == sattr2.ivalue;
        return;
      case dbvmNeInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.ivalue != sattr2.ivalue;
        return;
      case dbvmGtInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.ivalue > sattr2.ivalue;
        return;
      case dbvmGeInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.ivalue >= sattr2.ivalue;
        return;
      case dbvmLtInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.ivalue < sattr2.ivalue;
        return;
      case dbvmLeInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.ivalue <= sattr2.ivalue;
        return;
      case dbvmBetweenInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if (sattr.ivalue < sattr2.ivalue) {
            sattr.bvalue = false;
        } else {
            execute(expr->operand[2], iattr, sattr2);
            sattr.bvalue = sattr.ivalue <= sattr2.ivalue;
        }
        return;

      case dbvmEqRectangle:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.rvalue == sattr2.rvalue;
        return;
      case dbvmNeRectangle:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.rvalue != sattr2.rvalue;
        return;
      case dbvmGtRectangle:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.rvalue > sattr2.rvalue;
        return;
      case dbvmGeRectangle:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.rvalue >= sattr2.rvalue;
        return;
      case dbvmLtRectangle:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.rvalue < sattr2.rvalue;
        return;
      case dbvmLeRectangle:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.rvalue <= sattr2.rvalue;
        return;

      case dbvmOverlapsRectangle:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.rvalue & sattr2.rvalue;
        return;

      case dbvmRectangleArea:
        execute(expr->operand[0], iattr, sattr);
        sattr.fvalue = (double)area(sattr.rvalue);
        return;

      case dbvmNegReal:
        execute(expr->operand[0], iattr, sattr);
        sattr.fvalue = -sattr.fvalue;
        return;
      case dbvmAddReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.fvalue += sattr2.fvalue;
        return;
      case dbvmSubReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.fvalue -= sattr2.fvalue;
        return;
      case dbvmMulReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.fvalue *= sattr2.fvalue;
        return;
      case dbvmDivReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if (sattr2.fvalue == 0.0) {
            iattr.removeTemporaries();
            iattr.db->handleError(ArithmeticError, "Division by zero");
        } else {
            sattr.fvalue /= sattr2.fvalue;
        }
        return;
      case dbvmAbsReal:
        execute(expr->operand[0], iattr, sattr);
        if (sattr.fvalue < 0) {
            sattr.fvalue = -sattr.fvalue;
        }
        return;
      case dbvmPowerReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if (sattr.fvalue < 0) {
            iattr.removeTemporaries();
            iattr.db->handleError(ArithmeticError,
                                  "Power operator returns complex result");
        } else if (sattr.fvalue == 0.0 && sattr2.fvalue < 0) {
            iattr.removeTemporaries();
            iattr.db->handleError(ArithmeticError,
                                  "Raise zero to negative power");
        } else {
            sattr.fvalue = pow(sattr.fvalue, sattr2.fvalue);
        }
        return;
      case dbvmPowerRealInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if (sattr.fvalue == 0.0 && sattr2.ivalue < 0) {
            iattr.removeTemporaries();
            iattr.db->handleError(ArithmeticError,
                                  "Raise zero to negative power");
        } else {
            sattr.fvalue = powerRealInt(sattr.fvalue, sattr2.ivalue);
        }
        return;

      case dbvmEqReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.fvalue == sattr2.fvalue;
        return;
      case dbvmNeReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.fvalue != sattr2.fvalue;
        return;
      case dbvmGtReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.fvalue > sattr2.fvalue;
        return;
      case dbvmGeReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.fvalue >= sattr2.fvalue;
        return;
      case dbvmLtReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.fvalue < sattr2.fvalue;
        return;
      case dbvmLeReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.fvalue <= sattr2.fvalue;
        return;
      case dbvmBetweenReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if (sattr.fvalue < sattr2.fvalue) {
            sattr.bvalue = false;
        } else {
            execute(expr->operand[2], iattr, sattr2);
            sattr.bvalue = sattr.fvalue <= sattr2.fvalue;
        }
        return;

     case dbvmEqBinary:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(sattr.raw, sattr2.raw, expr->offs) == 0;
        return;
      case dbvmNeBinary:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(sattr.raw, sattr2.raw, expr->offs) != 0;
        return;
      case dbvmGtBinary:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(sattr.raw, sattr2.raw, expr->offs) > 0;
        return;
      case dbvmGeBinary:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(sattr.raw, sattr2.raw, expr->offs) >= 0;
        return;
      case dbvmLtBinary:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(sattr.raw, sattr2.raw, expr->offs) < 0;
        return;
      case dbvmLeBinary:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(sattr.raw, sattr2.raw, expr->offs) <= 0;
        return;
      case dbvmBetweenBinary:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if ((*(dbUDTComparator)expr->func.fptr)(sattr.raw, sattr2.raw, expr->offs) < 0) {
            sattr.bvalue = false;
        } else {
            execute(expr->operand[2], iattr, sattr2);
            sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(sattr.raw, sattr2.raw, expr->offs) <= 0;
        }
        return;

      case dbvmIntToReference:
        execute(expr->operand[0], iattr, sattr);
        sattr.oid = (oid_t)sattr.ivalue;
        return;

       case dbvmIntToReal:
        execute(expr->operand[0], iattr, sattr);
        sattr.fvalue = (real8)sattr.ivalue;
        return;
      case dbvmRealToInt:
        execute(expr->operand[0], iattr, sattr);
        sattr.ivalue = (db_int8)sattr.fvalue;
        return;

      case dbvmIntToString:
        execute(expr->operand[0], iattr, sattr);
        convertIntToString(iattr, sattr);
        return;
      case dbvmRealToString:
        execute(expr->operand[0], iattr, sattr);
        convertRealToString(iattr, sattr);
        return;
      case dbvmStringConcat:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        concatenateStrings(iattr, sattr, sattr2);
        return;
      case dbvmUpperString:
        execute(expr->operand[0], iattr, sattr);
        uppercaseString(iattr, sattr);
        return;
      case dbvmLowerString:
        execute(expr->operand[0], iattr, sattr);
        lowercaseString(iattr, sattr);
        return;

      case dbvmEqString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = compareStrings(sattr, sattr2) == 0;
        return;
      case dbvmNeString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = compareStrings(sattr, sattr2) != 0;
        return;
      case dbvmGtString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = compareStrings(sattr, sattr2) > 0;
        return;
      case dbvmGeString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = compareStrings(sattr, sattr2) >= 0;
        return;
      case dbvmLtString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = compareStrings(sattr, sattr2) < 0;
        return;
      case dbvmLeString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = compareStrings(sattr, sattr2) <= 0;
        return;
#ifdef USE_REGEX
      case dbvmMatchString:
        execute(expr->regex.opd, iattr, sattr);
        sattr.bvalue = regexec(&expr->regex.re, (char*)sattr.array.base, 0, NULL, 0) == 0;
        return;
#endif
      case dbvmLikeString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = matchStrings(sattr, sattr2);
        return;
      case dbvmLikeEscapeString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        execute(expr->operand[2], iattr, sattr3);
        sattr.bvalue = matchStrings(sattr, sattr2, *sattr3.array.base);
        return;
      case dbvmBetweenString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if (compareStrings(sattr, sattr2) < 0) {
            sattr.bvalue = false;
        } else {
            execute(expr->operand[2], iattr, sattr2);
            sattr.bvalue = compareStrings(sattr, sattr2) <= 0;
        }
        return;

      case dbvmEqBool:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.bvalue == sattr2.bvalue;
        return;
      case dbvmNeBool:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.bvalue != sattr2.bvalue;
        return;

      case dbvmEqReference:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.oid == sattr2.oid;
        return;
      case dbvmNeReference:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.oid != sattr2.oid;
        return;

      case dbvmDeref:
        execute(expr->operand[0], iattr, sattr);
        if (sattr.oid == 0) {
            iattr.removeTemporaries();
            iattr.db->handleError(NullReferenceError);
        }
        assert(!(iattr.db->currIndex[sattr.oid]
                 & (dbInternalObjectMarker|dbFreeHandleMarker)));
        sattr.base = iattr.db->baseAddr + iattr.db->currIndex[sattr.oid];
        return;

      case dbvmFuncArg2Bool:
        sattr.bvalue = (*(bool(*)(dbUserFunctionArgument const&))expr->func.fptr)
            (dbUserFunctionArgument(expr, iattr, sattr, 0));
        return;
      case dbvmFuncArg2Int:
        sattr.ivalue = (*(db_int8(*)(dbUserFunctionArgument const&))expr->func.fptr)
            (dbUserFunctionArgument(expr, iattr, sattr, 0));
        return;
      case dbvmFuncArg2Real:
        sattr.fvalue = (*(real8(*)(dbUserFunctionArgument const&))expr->func.fptr)
            (dbUserFunctionArgument(expr, iattr, sattr, 0));
        return;
      case dbvmFuncArg2Str:
        copyString(iattr, sattr,
                   (*(char*(*)(dbUserFunctionArgument const&))expr->func.fptr)
                   (dbUserFunctionArgument(expr, iattr, sattr, 0)));
        return;
      case dbvmFuncArgArg2Bool:
        sattr.bvalue = (*(bool(*)(dbUserFunctionArgument const&, dbUserFunctionArgument const&))expr->func.fptr)
            (dbUserFunctionArgument(expr, iattr, sattr, 0),
             dbUserFunctionArgument(expr, iattr, sattr, 1));
        return;
      case dbvmFuncArgArg2Int:
        sattr.ivalue = (*(db_int8(*)(dbUserFunctionArgument const&, dbUserFunctionArgument const&))expr->func.fptr)
            (dbUserFunctionArgument(expr, iattr, sattr, 0),
             dbUserFunctionArgument(expr, iattr, sattr, 1));
        return;
      case dbvmFuncArgArg2Real:
        sattr.fvalue = (*(real8(*)(dbUserFunctionArgument const&, dbUserFunctionArgument const&))expr->func.fptr)
            (dbUserFunctionArgument(expr, iattr, sattr, 0),
             dbUserFunctionArgument(expr, iattr, sattr, 1));
        return;
      case dbvmFuncArgArg2Str:
        copyString(iattr, sattr,
                   (*(char*(*)(dbUserFunctionArgument const&, dbUserFunctionArgument const&))expr->func.fptr)
                   (dbUserFunctionArgument(expr, iattr, sattr, 0),
                    dbUserFunctionArgument(expr, iattr, sattr, 1)));
        return;
      case dbvmFuncArgArgArg2Bool:
        sattr.bvalue = (*(bool(*)(dbUserFunctionArgument const&, dbUserFunctionArgument const&, dbUserFunctionArgument const&))expr->func.fptr)
            (dbUserFunctionArgument(expr, iattr, sattr, 0),
             dbUserFunctionArgument(expr, iattr, sattr, 1),
             dbUserFunctionArgument(expr, iattr, sattr, 2));
        return;
      case dbvmFuncArgArgArg2Int:
        sattr.ivalue = (*(db_int8(*)(dbUserFunctionArgument const&, dbUserFunctionArgument const&, dbUserFunctionArgument const&))expr->func.fptr)
            (dbUserFunctionArgument(expr, iattr, sattr, 0),
             dbUserFunctionArgument(expr, iattr, sattr, 1),
             dbUserFunctionArgument(expr, iattr, sattr, 2));
        return;
      case dbvmFuncArgArgArg2Real:
        sattr.fvalue = (*(real8(*)(dbUserFunctionArgument const&, dbUserFunctionArgument const&, dbUserFunctionArgument const&))expr->func.fptr)
            (dbUserFunctionArgument(expr, iattr, sattr, 0),
             dbUserFunctionArgument(expr, iattr, sattr, 1),
             dbUserFunctionArgument(expr, iattr, sattr, 2));
        return;
      case dbvmFuncArgArgArg2Str:
        copyString(iattr, sattr,
                   (*(char*(*)(dbUserFunctionArgument const&, dbUserFunctionArgument const&, dbUserFunctionArgument const&))expr->func.fptr)
                   (dbUserFunctionArgument(expr, iattr, sattr, 0),
                    dbUserFunctionArgument(expr, iattr, sattr, 1),
                    dbUserFunctionArgument(expr, iattr, sattr, 2)));
        return;



      case dbvmFuncInt2Bool:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.bvalue = (*(bool(*)(db_int8))expr->func.fptr)(sattr.ivalue);
        return;
      case dbvmFuncReal2Bool:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.bvalue = (*(bool(*)(real8))expr->func.fptr)(sattr.fvalue);
        return;
      case dbvmFuncStr2Bool:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.bvalue =
            (*(bool(*)(char const*))expr->func.fptr)(sattr.array.base);
        return;
      case dbvmFuncInt2Int:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.ivalue = (*(db_int8(*)(db_int8))expr->func.fptr)(sattr.ivalue);
        return;
      case dbvmFuncReal2Int:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.ivalue = (*(db_int8(*)(real8))expr->func.fptr)(sattr.fvalue);
        return;
      case dbvmFuncStr2Int:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.ivalue =
            (*(db_int8(*)(char const*))expr->func.fptr)(sattr.array.base);
        return;
      case dbvmFuncInt2Real:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.fvalue = (*(real8(*)(db_int8))expr->func.fptr)(sattr.ivalue);
        return;
      case dbvmFuncReal2Real:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.fvalue = (*(real8(*)(real8))expr->func.fptr)(sattr.fvalue);
        return;
      case dbvmFuncStr2Real:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.fvalue =
            (*(real8(*)(char const*))expr->func.fptr)(sattr.array.base);
        return;
      case dbvmFuncInt2Str:
        execute(expr->func.arg[0], iattr, sattr);
        copyString(iattr, sattr,
                   (*(char*(*)(db_int8))expr->func.fptr)(sattr.ivalue));
        return;
      case dbvmFuncReal2Str:
        execute(expr->func.arg[0], iattr, sattr);
        copyString(iattr, sattr,
                   (*(char*(*)(real8))expr->func.fptr)(sattr.fvalue));
        return;
      case dbvmFuncStr2Str:
        execute(expr->func.arg[0], iattr, sattr);
        copyString(iattr, sattr,
                  (*(char*(*)(char const*))expr->func.fptr)(sattr.array.base));
        return;

      case dbvmInArrayBool:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfBool(sattr, sattr2);
        return;
      case dbvmInArrayInt1:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfInt1(sattr, sattr2);
        return;
      case dbvmInArrayInt2:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfInt2(sattr, sattr2);
        return;
      case dbvmInArrayInt4:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfInt4(sattr, sattr2);
        return;
      case dbvmInArrayInt8:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfInt8(sattr, sattr2);
        return;
      case dbvmInArrayReal4:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfReal4(sattr, sattr2);
        return;
      case dbvmInArrayReal8:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfReal8(sattr, sattr2);
        return;
      case dbvmInArrayString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfString(sattr, sattr2);
        return;
      case dbvmInArrayReference:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfReference(sattr, sattr2);
        return;
      case dbvmInArrayRectangle:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfRectangle(iattr, sattr, sattr2);
        return;
      case dbvmInString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchInString(sattr, sattr2);
        return;

      default:
        assert(false);
    }
}

char const* const dbDatabase::errorMessage[] =
{
    "No error",
    "Query syntax error",
    "Arithmetic exception",
    "Index out of range",
    "Database open error",
    "File access error",
    "Out of memory",
    "Deadlock",
    "Null reference",
    "Lock revoked",
    "File limit exeeded",
    "Inconsistent inverse reference",
    "Attempt to modify read-only database"
};

void dbDatabase::handleError(dbErrorClass error, char const* msg, int arg)
{
    if (errorHandler != NULL) {
        (*errorHandler)(error, msg, arg, errorHandlerContext);
    }
#ifdef THROW_EXCEPTION_ON_ERROR
    if (error != NoError) {
        if (msg == NULL) {
            msg = errorMessage[error];
        }
        if (error == DatabaseOpenError) {
            OsSysLog::add(FAC_DB, PRI_ERR, "%s", msg);
        } else {
            throw dbException(error, msg, arg);
        }
    }
#else
    char buf[256];
    switch (error) {
      case QueryError:
        OsSysLog::add(FAC_DB, PRI_ERR, "%s in position %d", msg, arg);
        return;
      case ArithmeticError:
        OsSysLog::add(FAC_DB, PRI_ERR, "%s", msg);
        break;
      case IndexOutOfRangeError:
        OsSysLog::add(FAC_DB, PRI_ERR, "Index %d is out of range", arg);
        break;
      case DatabaseOpenError:
        OsSysLog::add(FAC_DB, PRI_ERR, "%s", msg);
        return;
      case FileError:
        OsSysLog::add(FAC_DB, PRI_ERR, "%s: %s", msg,
                dbFile::errorText(arg, buf, sizeof(buf)));
        break;
      case OutOfMemoryError:
        OsSysLog::add(FAC_DB, PRI_ERR,"Not enough memory: failed to allocate %d bytes",arg);
        break;
      case NullReferenceError:
        OsSysLog::add(FAC_DB, PRI_ERR, "Null object reference is accessed");
        break;
      case Deadlock:
        OsSysLog::add(FAC_DB, PRI_ERR, "Deadlock is caused by upgrading shared locks to exclusive");
        break;
      case LockRevoked:
        OsSysLog::add(FAC_DB, PRI_ERR, "Lock is revoked by some other client");
        break;
      case InconsistentInverseReference:
        OsSysLog::add(FAC_DB, PRI_ERR, "%s", msg);
        return;
      case DatabaseReadOnly:
        OsSysLog::add(FAC_DB, PRI_ERR, "Attempt to modify readonly database");
        break;
      default:
        return;
    }
    abort();
#endif
}

bool dbDatabase::isReplicated()
{
    return false;
}

void dbDatabase::initializeMetaTable()
{
    static struct {
        char const* name;
        int         type;
        int         size;
        int         offs;
    } metaTableFields[] = {
        { "name", dbField::tpString, sizeof(dbVarying),
          offsetof(dbTable, name)},
        { "fields", dbField::tpArray, sizeof(dbVarying),
          offsetof(dbTable, fields)},
        { "fields[]", dbField::tpStructure, sizeof(dbField), 0},
        { "fields[].name", dbField::tpString, sizeof(dbVarying),
          offsetof(dbField, name)},
        { "fields[].tableName",dbField::tpString,sizeof(dbVarying),
          offsetof(dbField, tableName)},
        { "fields[].inverse", dbField::tpString, sizeof(dbVarying),
          offsetof(dbField, inverse)},
//        { "fields[].type", dbField::tpInt4, 4, offsetof(dbField, type)},
        { "fields[].type", dbField::tpInt4, 4, offsetof(dbField, offset)-4},
        { "fields[].offset", dbField::tpInt4, 4, offsetof(dbField, offset)},
        { "fields[].size", dbField::tpInt4, 4, offsetof(dbField, size)},
        { "fields[].hashTable", dbField::tpReference, sizeof(oid_t),
          offsetof(dbField, hashTable)},
        { "fields[].tTree", dbField::tpReference, sizeof(oid_t),
          offsetof(dbField, tTree)},
        { "fixedSize", dbField::tpInt4, 4, offsetof(dbTable, fixedSize)},
        { "nRows", dbField::tpInt4, 4, offsetof(dbTable, nRows)},
        { "nColumns", dbField::tpInt4, 4, offsetof(dbTable, nColumns)},
        { "firstRow", dbField::tpReference, sizeof(oid_t), offsetof(dbTable, firstRow)},
        { "lastRow", dbField::tpReference, sizeof(oid_t), offsetof(dbTable, lastRow)}
 #ifdef AUTOINCREMENT_SUPPORT
       ,{ "count", dbField::tpInt4, 4, offsetof(dbTable, count)}
 #endif
    };

    unsigned i;
    size_t varyingSize = strlen(dbMetaTableName)+1;
    for (i = 0; i < itemsof(metaTableFields); i++) {
        varyingSize += strlen(metaTableFields[i].name) + 3;

    }
    offs_t metaTableOffs = allocate(sizeof(dbTable)
                                    + sizeof(dbField)*itemsof(metaTableFields)
                                    + varyingSize);
    index[0][dbMetaTableId] = metaTableOffs;
    dbTable* table = (dbTable*)(baseAddr + metaTableOffs);
    table->size = sizeof(dbTable) + sizeof(dbField)*itemsof(metaTableFields)
                + varyingSize;
    table->next = table->prev = 0;
    int offs = sizeof(dbTable) + sizeof(dbField)*itemsof(metaTableFields);
    table->name.offs = offs;
    table->name.size = strlen(dbMetaTableName)+1;
    strcpy((char*)table + offs, dbMetaTableName);
    offs += table->name.size;
    table->fields.offs = sizeof(dbTable);
    table->fields.size = itemsof(metaTableFields);
    table->fixedSize = sizeof(dbTable);
    table->nRows = 0;
    table->nColumns = 5;
    table->firstRow = 0;
    table->lastRow = 0;
#ifdef AUTOINCREMENT_SUPPORT
    table->count = 0;
#endif

    dbField* field = (dbField*)((char*)table + table->fields.offs);
    offs -= sizeof(dbTable);
    for (i = 0; i < itemsof(metaTableFields); i++) {
        field->name.offs = offs;
        field->name.size = strlen(metaTableFields[i].name) + 1;
        strcpy((char*)field + offs, metaTableFields[i].name);
        offs += field->name.size;

        field->tableName.offs = offs;
        field->tableName.size = 1;
        *((char*)field + offs++) = '\0';

        field->inverse.offs = offs;
        field->inverse.size = 1;
        *((char*)field + offs++) = '\0';

        field->flags = 0;
        field->type = metaTableFields[i].type;
        field->size = metaTableFields[i].size;
        field->offset = metaTableFields[i].offs;
        field->hashTable = 0;
        field->tTree = 0;
        field += 1;
        offs -= sizeof(dbField);
    }
}

void dbDatabase::cleanup(dbInitializationMutex::initializationStatus status, int step)
{
    switch (step) {
      case 9:
        if (status == dbInitializationMutex::NotYetInitialized) {
            file.close();
        }
        // no break
      case 8:
        if (accessType == dbConcurrentUpdate || accessType == dbConcurrentRead) {
            mutatorCS.close();
        }
        // no break
      case 7:
        if (delayedCommitEventsOpened) {
            delayedCommitStopTimerEvent.close();
            delayedCommitStartTimerEvent.close();
            commitThreadSyncEvent.close();
            delayedCommitEventsOpened = false;
        }
        // no break
      case 6:
        cs.close();
        // no break
      case 5:
        backupCompletedEvent.close();
        // no break
      case 4:
        upgradeSem.close();
        // no break
      case 3:
        readSem.close();
        // no break
      case 2:
        writeSem.close();
        // no break
      case 1:
        shm.close();
        // no break
      default:
        if (status == dbInitializationMutex::NotYetInitialized) {
            initMutex.done();
        }
        initMutex.close();
    }
}

bool dbDatabase::open(OpenParameters& params)
{
    accessType = params.accessType;
    extensionQuantum = params.extensionQuantum;
    initIndexSize = params.initIndexSize;
    initSize = params.initSize;
    freeSpaceReuseThreshold = params.freeSpaceReuseThreshold;
    parallelScanThreshold = params.parallelScanThreshold;
    setConcurrency(params.nThreads);
    return open(params.databaseName, params.databaseFilePath, params.waitLockTimeoutMsec, params.transactionCommitDelay);
}

bool dbDatabase::open(char const* dbName, char const* fiName,
                      time_t waitLockTimeoutMsec, time_t commitDelaySec)
{
    waitLockTimeout = (unsigned)waitLockTimeoutMsec;
    delete[] databaseName;
    delete[] fileName;
    commitDelay = 0;
    commitTimeout = 0;
    commitTimerStarted = 0;
    delayedCommitEventsOpened = false;
    backupFileName = NULL;
    backupPeriod = 0;
    opened = false;
    stopDelayedCommitThread = false;
    databaseNameLen = strlen(dbName);
    char* name = new char[databaseNameLen+16];
    sprintf(name, "%s.in", dbName);
    databaseName = name;
    if (fiName == NULL) {
        fileName = new char[databaseNameLen + 5];
        sprintf(fileName, "%s.fdb", dbName);
    } else {
        fileName = new char[strlen(fiName)+1];
        strcpy(fileName, fiName);
    }

    dbInitializationMutex::initializationStatus status = initMutex.initialize(name);
    if (status == dbInitializationMutex::InitializationError) {
        handleError(DatabaseOpenError,
                    "Failed to start database initialization");
        return false;
    }
    OsSysLog::add(FAC_DB, PRI_INFO, "initMutex.initialize returns %s\n",
        status == dbInitializationMutex::NotYetInitialized ?
            "dbInitializationMutex::NotYetInitialized" : "dbInitializationMutex::AlreadyInitialized");

    sprintf(name, "%s.dm", dbName);
    int shmRes = shm.open(name) ;
    if (shmRes < 0) {
        handleError(DatabaseOpenError, "Failed to open database monitor");
        cleanup(status, 0);
        return false;
    }
    // If the shared memory segment was created, it needs to be initialized
    if (shmRes == 1)
    {
        status = dbInitializationMutex::NotYetInitialized ;
    }
#if defined(USE_POSIX_MMAP) && USE_POSIX_MMAP
    if (status == dbInitializationMutex::AlreadyInitialized)
    {
       // Still Holding initMutex here in POSIX case (not so for SysV)
       // Safe to clear initMutex now
       initMutex.done() ;
    }
#endif
    monitor = shm.get();

    sprintf(name, "%s.ws", dbName);
    if (!writeSem.open(name)) {
        handleError(DatabaseOpenError,
                    "Failed to initialize database writers semaphore");
        cleanup(status, 1);
        return false;
    }
    sprintf(name, "%s.rs", dbName);
    if (!readSem.open(name)) {
        handleError(DatabaseOpenError,
                    "Failed to initialize database readers semaphore");
        cleanup(status, 2);
        return false;
    }
    sprintf(name, "%s.us", dbName);
    if (!upgradeSem.open(name)) {
        handleError(DatabaseOpenError,
                    "Failed to initialize database upgrade semaphore");
        cleanup(status, 3);
        return false;
    }
    sprintf(name, "%s.bce", dbName);
    if (!backupCompletedEvent.open(name)) {
        handleError(DatabaseOpenError,
                    "Failed to initialize database backup completed event");
        cleanup(status, 4);
        return false;
    }
    if (commitDelaySec != 0) {
        sprintf(name, "%s.dce", dbName);
        delayedCommitEventsOpened = true;
        if (!delayedCommitStopTimerEvent.open(name)) {
            handleError(DatabaseOpenError,
                        "Failed to initialize delayed commit event");
            cleanup(status, 5);
            return false;
        }
        delayedCommitStartTimerEvent.open();
        commitThreadSyncEvent.open();
    }
    backupInitEvent.open();
    backupFileName = NULL;

    fixedSizeAllocator.reset();
    allocatedSize = 0;
    deallocatedSize = 0;
    size_t indexSize = initIndexSize < dbFirstUserId
        ? size_t(dbFirstUserId) : initIndexSize;
    indexSize = DOALIGN(indexSize, dbHandlesPerPage);

    size_t fileSize = initSize ? initSize : dbDefaultInitDatabaseSize;

    if (fileSize < indexSize*sizeof(offs_t)*4) {
        fileSize = indexSize*sizeof(offs_t)*4;
    }
    fileSize = DOALIGN(fileSize, dbBitmapSegmentSize);

    for (int i = dbBitmapId + dbBitmapPages; --i >= 0;) {
        bitmapPageAvailableSpace[i] = INT_MAX;
    }
    currRBitmapPage = currPBitmapPage = dbBitmapId;
    currRBitmapOffs = currPBitmapOffs = 0;
    reservedChain = NULL;
    tables = NULL;
    modified = false;
    selfId = 0;
    maxClientId = 0;
    threadContextList.reset();
    attach();

    if (status == dbInitializationMutex::NotYetInitialized) {
        sprintf(name, "%s.cs", dbName);
        if (!cs.create(name, &monitor->sem)) {
            handleError(DatabaseOpenError, "Failed to initialize database monitor");
            cleanup(status, 6);
            return false;
        }
        if (accessType == dbConcurrentUpdate || accessType == dbConcurrentRead) {
            sprintf(name, "%s.mcs", dbName);
            if (!mutatorCS.create(name, &monitor->mutatorSem)) {
                handleError(DatabaseOpenError,
                            "Failed to initialize database monitor");
                cleanup(status, 7);
                return false;
            }
        }
        readSem.reset();
        writeSem.reset();
        upgradeSem.reset();
        monitor->nReaders = 0;
        monitor->nWriters = 0;
        monitor->nWaitReaders = 0;
        monitor->nWaitWriters = 0;
        monitor->waitForUpgrade = false;
        monitor->version = version = 1;
        monitor->users = 0;
        monitor->backupInProgress = 0;
        monitor->forceCommitCount = 0;
        monitor->lastDeadlockRecoveryTime = 0;
        monitor->delayedCommitContext = NULL;
        monitor->concurrentTransId = 1;
        monitor->commitInProgress = false;
        monitor->uncommittedChanges = false;
        monitor->clientId = 0;
        monitor->upgradeId = 0;
        monitor->modified = false;
        monitor->exclusiveLockOwner = 0;
        memset(monitor->dirtyPagesMap, 0, dbDirtyPageBitmapSize);
        memset(monitor->sharedLockOwner, 0, sizeof(monitor->sharedLockOwner));
#ifdef DO_NOT_REUSE_OID_WITHIN_SESSION
        monitor->sessionFreeList[0].head = monitor->sessionFreeList[0].tail = 0;
        monitor->sessionFreeList[1].head = monitor->sessionFreeList[1].tail = 0;
#endif

        sprintf(databaseName, "%s.%d", dbName, version);
        int rc = file.open(fileName, databaseName,
                           accessType == dbReadOnly || accessType == dbConcurrentRead, fileSize, false);
        if (rc != dbFile::ok)
        {
            char msgbuf[64];
            file.errorText(rc, msgbuf, sizeof msgbuf);
            TRACE_MSG(("File open error: %s\n", msgbuf));
            handleError(DatabaseOpenError, "Failed to create database file");
            cleanup(status, 8);
            return false;
        }
        baseAddr = (byte*)file.getAddr();
        monitor->size = fileSize = file.getSize();
        header = (dbHeader*)baseAddr;
        updatedRecordId = 0;

        if ((unsigned)header->curr > 1) {
            handleError(DatabaseOpenError, "Database file was corrupted: "
                        "invalid root index");
            cleanup(status, 9);
            return false;
        }
        if (header->initialized != 1) {
            if (accessType == dbReadOnly || accessType == dbConcurrentRead) {
                handleError(DatabaseOpenError, "Can not open uninitialized "
                            "file in read only mode");
                cleanup(status, 9);
                return false;
            }
            monitor->curr = header->curr = 0;
            header->size = fileSize;
            size_t used = dbPageSize;
            header->root[0].index = used;
            header->root[0].indexSize = indexSize;
            header->root[0].indexUsed = dbFirstUserId;
            header->root[0].freeList = 0;
            used += indexSize*sizeof(offs_t);
            header->root[1].index = used;
            header->root[1].indexSize = indexSize;
            header->root[1].indexUsed = dbFirstUserId;
            header->root[1].freeList = 0;
            used += indexSize*sizeof(offs_t);

            header->root[0].shadowIndex = header->root[1].index;
            header->root[1].shadowIndex = header->root[0].index;
            header->root[0].shadowIndexSize = indexSize;
            header->root[1].shadowIndexSize = indexSize;

            header->majorVersion= FASTDB_MAJOR_VERSION;
            header->minorVersion = FASTDB_MINOR_VERSION;
            header->mode = dbHeader::getCurrentMode();

            index[0] = (offs_t*)(baseAddr + header->root[0].index);
            index[1] = (offs_t*)(baseAddr + header->root[1].index);
            index[0][dbInvalidId] = dbFreeHandleMarker;

            size_t bitmapPages =
                (used + dbPageSize*(dbAllocationQuantum*8-1) - 1)
                / (dbPageSize*(dbAllocationQuantum*8-1));
            memset(baseAddr+used, 0xFF, (used + bitmapPages*dbPageSize)
                                        / (dbAllocationQuantum*8));
            size_t i;
            for (i = 0; i < bitmapPages; i++) {
                index[0][dbBitmapId + i] = used + dbPageObjectMarker;
                used += dbPageSize;
            }
            while (i < dbBitmapPages) {
                index[0][dbBitmapId+i] = dbFreeHandleMarker;
                i += 1;
            }
            currIndex = index[0];
            currIndexSize = dbFirstUserId;
            committedIndexSize = 0;
            initializeMetaTable();
            header->dirty = true;
            memcpy(index[1], index[0], currIndexSize*sizeof(offs_t));
            file.markAsDirty(0, used);
            file.flush(true);
            header->initialized = true;
            file.markAsDirty(0, sizeof(dbHeader));
            file.flush(true);
        } else {
            if (!header->isCompatible()) {
                handleError(DatabaseOpenError, "Incompatible database mode");
                cleanup(status, 9);
                return false;
            }
            monitor->curr = header->curr;
            if (header->dirty) {
                TRACE_MSG(("Database was not normally closed: start recovery\n"));
                if (accessType == dbReadOnly || accessType == dbConcurrentRead) {
                    handleError(DatabaseOpenError,
                                "Can not open dirty file in read only mode");
                    cleanup(status, 9);
                    return false;
                }
                recovery();
                TRACE_MSG(("Recovery completed\n"));
            } else {
                if (file.getSize() != header->size) {
                    handleError(DatabaseOpenError, "Database file was "
                                "corrupted: file size in header differs "
                                "from actual file size");
                    cleanup(status, 9);
                    return false;
                }
            }
        }
    } else {
        sprintf(name, "%s.cs", dbName);
        if (!cs.open(name, &monitor->sem)) {
            handleError(DatabaseOpenError, "Failed to open shared semaphore");
            cleanup(status, 6);
            return false;
        }
        if (accessType == dbConcurrentUpdate || accessType == dbConcurrentRead) {
            sprintf(name, "%s.mcs", dbName);
            if (!mutatorCS.open(name, &monitor->mutatorSem)) {
                handleError(DatabaseOpenError, "Failed to open shared semaphore");
                cleanup(status, 7);
                return false;
            }
        }
        version = 0;
    }
    cs.enter();
    monitor->users += 1;
    selfId = ++monitor->clientId;
#ifdef AUTO_DETECT_PROCESS_CRASH
    sprintf(databaseName + databaseNameLen, ".pid.%d", selfId);
    selfWatchDog.create(databaseName);
    watchDogMutex = new dbMutex();
#endif
    cs.leave();

    if (status == dbInitializationMutex::NotYetInitialized) {
        if (!loadScheme(true)) {
            cleanup(status, 9);
            return false;
        }
        initMutex.done();
    } else {
        if (!loadScheme(false)) {
            cleanup(status, 9);
            return false;
        }
    }
    opened = true;

    if (commitDelaySec != 0) {
        dbCriticalSection cs(delayedCommitStartTimerMutex);
        commitTimeout = commitDelay = commitDelaySec;
        commitThread.create((dbThread::thread_proc_t)delayedCommitProc, this);
        commitThreadSyncEvent.wait(delayedCommitStartTimerMutex);
    }
    return true;
}

void dbDatabase::scheduleBackup(char const* fileName, time_t period)
{
    if (backupFileName == NULL) {
        backupFileName = new char[strlen(fileName) + 1];
        strcpy(backupFileName, fileName);
        backupPeriod = period;
        backupThread.create((dbThread::thread_proc_t)backupSchedulerProc, this);
    }
}

void dbDatabase::backupScheduler()
{
    backupThread.setPriority(dbThread::THR_PRI_LOW);
    dbCriticalSection cs(backupMutex);
    while (true) {
        time_t timeout = backupPeriod;
        if (backupFileName[strlen(backupFileName)-1] != '?') {
#ifdef _WINCE
            WIN32_FIND_DATA lData;
            HANDLE lFile = ::FindFirstFile(W32_STRING(backupFileName), &lData);
            FILETIME lATime;
            if (::GetFileTime(lFile, 0l, &lATime, 0l) == TRUE)
            {
                ULARGE_INTEGER lNTime = *(ULARGE_INTEGER*)&lATime;

                time_t howOld = time(NULL) - *(time_t*)&lNTime;
                if (timeout < howOld) {
                    timeout = 0;
                        } else {
                            timeout -= howOld;
                        }
            }
            ::FindClose(lFile);
#else
            struct stat st;
            if (::stat(backupFileName, &st) == 0) {
                time_t howOld = time(NULL) - st.st_atime;
                if (timeout < howOld) {
                    timeout = 0;
                } else {
                    timeout -= howOld;
                }
            }
#endif
        }

        backupInitEvent.wait(backupMutex, timeout*1000);

        if (backupFileName != NULL) {
            if (backupFileName[strlen(backupFileName)-1] == '?') {
                time_t currTime = time(NULL);
                char* fileName = new char[strlen(backupFileName) + 32];
                struct tm* t = localtime(&currTime);
                sprintf(fileName, "%.*s-%04d.%02d.%02d_%02d.%02d.%02d",
                        (int)strlen(backupFileName)-1, backupFileName,
                        t->tm_year + 1900, t->tm_mon+1, t->tm_mday,
                        t->tm_hour, t->tm_min, t->tm_sec);
                backup(fileName, false);
                delete[] fileName;
            } else {
                char* newFileName = new char[strlen(backupFileName) + 5];
                sprintf(newFileName,"%s.new", backupFileName);
                backup(newFileName, false);
#ifdef _WINCE
                DeleteFile(W32_STRING(backupFileName));
                MoveFile(W32_STRING(newFileName), W32_STRING(backupFileName));
#else
                ::remove(backupFileName);
                ::rename(newFileName, backupFileName);
#endif
                delete[] newFileName;
            }
        } else {
            return;
        }
    }
}


void dbDatabase::recovery()
{
    int curr = header->curr;
    header->size = file.getSize();
    header->root[1-curr].indexUsed = header->root[curr].indexUsed;
    header->root[1-curr].freeList = header->root[curr].freeList;
    header->root[1-curr].index = header->root[curr].shadowIndex;
    header->root[1-curr].indexSize =
        header->root[curr].shadowIndexSize;
    header->root[1-curr].shadowIndex = header->root[curr].index;
    header->root[1-curr].shadowIndexSize =
        header->root[curr].indexSize;
#ifdef DO_NOT_REUSE_OID_WITHIN_SESSION
    monitor->sessionFreeList[1-curr] = monitor->sessionFreeList[curr];
#endif

    offs_t* dst = (offs_t*)(baseAddr+header->root[1-curr].index);
    offs_t* src = (offs_t*)(baseAddr+header->root[curr].index);
    currIndex = dst;
    for (oid_t i = 0, n = header->root[curr].indexUsed; i < n; i++) {
        if (dst[i] != src[i]) {
            dst[i] = src[i];
            file.markAsDirty(header->root[1-curr].index + i*sizeof(offs_t), sizeof(offs_t));
        }
    }
    //
    // Restore consistency of table rows l2-list
    //
    restoreTablesConsistency();
    file.markAsDirty(0, sizeof(dbHeader));
}

void dbDatabase::restoreTablesConsistency()
{
    dbTable* table = (dbTable*)getRow(dbMetaTableId);
    oid_t lastId = table->lastRow;
    if (lastId != 0) {
        dbRecord* record = getRow(lastId);
        if (record->next != 0) {
            record->next = 0;
            file.markAsDirty(currIndex[lastId], sizeof(dbRecord));
        }
    }
    oid_t tableId = table->firstRow;
    while (tableId != 0) {
        table = (dbTable*)getRow(tableId);
        lastId = table->lastRow;
        if (lastId != 0) {
            dbRecord* record = getRow(lastId);
            if (record->next != 0) {
                record->next = 0;
                file.markAsDirty(currIndex[lastId], sizeof(dbRecord));
            }
        }
        tableId = table->next;
    }
}

void dbDatabase::setConcurrency(unsigned nThreads)
{
    if (nThreads == 0) { // autodetect number of processors
        nThreads = dbThread::numberOfProcessors();
    }
    if (nThreads > dbMaxParallelSearchThreads) {
        nThreads = dbMaxParallelSearchThreads;
    }
    parThreads = nThreads;
}


bool dbDatabase::loadScheme(bool alter)
{
    if (!beginTransaction((alter && accessType != dbReadOnly && accessType != dbConcurrentRead)
                          || accessType == dbConcurrentUpdate
                          ? dbExclusiveLock : dbSharedLock))
    {
        return false;
    }

    dbTableDescriptor *desc, *next;
    dbTable* metaTable = (dbTable*)getRow(dbMetaTableId);
    oid_t first = metaTable->firstRow;
    oid_t last = metaTable->lastRow;
    int nTables = metaTable->nRows;
    oid_t tableId = first;

    for (desc = dbTableDescriptor::chain; desc != NULL; desc = next) {
        next = desc->next;
        if (desc->db != NULL && desc->db != DETACHED_TABLE && desc->db != this) {
            continue;
        }
        if (desc->db == DETACHED_TABLE) {
            desc = desc->clone();
        }
        dbFieldDescriptor* fd;
        for (fd = desc->firstField; fd != NULL; fd = fd->nextField) {
            fd->tTree = 0;
            fd->hashTable = 0;
            fd->attr &= ~dbFieldDescriptor::Updated;
        }

        int n = nTables;
        while (--n >= 0) {
            dbTable* table = (dbTable*)getRow(tableId);
            if (table == NULL) {
                handleError(DatabaseOpenError, "Database scheme is corrupted");
                return false;
            }
            oid_t next = table->next;
            if (strcmp(desc->name, (char*)table + table->name.offs) == 0) {
                if (!desc->equal(table)) {
                    if (!alter) {
                        handleError(DatabaseOpenError, "Incompatible class"
                                    " definition in application");
                        return false;
                    }
                    beginTransaction(dbExclusiveLock);
                    modified = true;
                    if (table->nRows == 0) {
                        TRACE_MSG(("Replace definition of table '%s'\n",  desc->name));
                        desc->match(table, true);
                        updateTableDescriptor(desc, tableId);
                    } else {
                        reformatTable(tableId, desc);
                    }
                } else {
                    linkTable(desc, tableId);
                }
                desc->setFlags();
                break;
            }
            if (tableId == last) {
                tableId = first;
            } else {
                tableId = next;
            }
        }
        if (n < 0) { // no match found
            if (accessType == dbReadOnly || accessType == dbConcurrentRead) {
                handleError(DatabaseOpenError, "New table definition can not "
                            "be added to read only database");
                return false;
            } else {
                TRACE_MSG(("Create new table '%s' in database\n", desc->name));
                addNewTable(desc);
                modified = true;
            }
        }
        if (accessType != dbReadOnly && accessType != dbConcurrentRead) {
            if (!addIndices(alter, desc)) {
                handleError(DatabaseOpenError, "Failed to alter indices with"
                            " active applications");
                rollback();
                return false;
            }
        }
    }
    for (desc = tables; desc != NULL; desc = desc->nextDbTable) {
        if (desc->cloneOf != NULL) {
            for (dbFieldDescriptor *fd = desc->firstField; fd != NULL; fd = fd->nextField)
            {
                if (fd->refTable != NULL) {
                    fd->refTable = lookupTable(fd->refTable);
                }
            }
        }
        desc->checkRelationship();
    }
    hashFunction = dbHashTable::getHashFunction(header->getVersion());
    commit();
    return true;
}


void dbDatabase::reformatTable(oid_t tableId, dbTableDescriptor* desc)
{
    dbTable* table = (dbTable*)putRow(tableId);

    if (desc->match(table, confirmDeleteColumns)) {
        TRACE_MSG(("New version of table '%s' is compatible with old one\n",
                   desc->name));
        updateTableDescriptor(desc, tableId);
    } else {
        TRACE_MSG(("Reformat table '%s'\n", desc->name));
        oid_t oid = table->firstRow;
        updateTableDescriptor(desc, tableId);
        while (oid != 0) {
            dbRecord* record = getRow(oid);
            size_t size =
                desc->columns->calculateNewRecordSize((byte*)record,
                                                     desc->fixedSize);
            offs_t offs = currIndex[oid];
            record = putRow(oid, size);
            byte* dst = (byte*)record;
            byte* src = baseAddr + offs;
            if (dst == src) {
                dbSmallBuffer buf(size);
                dst = (byte*)buf.base();
                desc->columns->convertRecord(dst, src, desc->fixedSize);
                memcpy(record+1, dst+sizeof(dbRecord), size-sizeof(dbRecord));
            } else {
                desc->columns->convertRecord(dst, src, desc->fixedSize);
            }
            oid = record->next;
        }
    }
}

void dbDatabase::deleteTable(dbTableDescriptor* desc)
{
    beginTransaction(dbExclusiveLock);
    modified = true;
    dbTable* table = (dbTable*)putRow(desc->tableId);
    oid_t rowId = table->firstRow;
    table->firstRow = table->lastRow = 0;
    table->nRows = 0;

    while (rowId != 0) {
        dbRecord* record = getRow(rowId);
        oid_t nextId = record->next;
        size_t size = record->size;

        removeInverseReferences(desc, rowId);

        if (rowId < committedIndexSize && index[0][rowId] == index[1][rowId]) {
            cloneBitmap(currIndex[rowId], size);
        } else {
            deallocate(currIndex[rowId], size);
        }
        freeId(rowId);
        rowId = nextId;
    }
    dbFieldDescriptor* fd;
    for (fd = desc->hashedFields; fd != NULL; fd = fd->nextHashedField) {
        dbHashTable::purge(this, fd->hashTable);
    }
    for (fd = desc->indexedFields; fd != NULL; fd = fd->nextIndexedField) {
        if (fd->type == dbField::tpRectangle) {
            dbRtree::purge(this, fd->tTree);
        } else {
            dbTtree::purge(this, fd->tTree);
        }
    }
}

void dbDatabase::dropHashTable(dbFieldDescriptor* fd)
{
    beginTransaction(dbExclusiveLock);
    modified = true;
    dbHashTable::drop(this, fd->hashTable);
    fd->hashTable = 0;
    fd->indexType &= ~HASHED;

    dbFieldDescriptor** fpp = &fd->defTable->hashedFields;
    while (*fpp != fd) {
        fpp = &(*fpp)->nextHashedField;
    }
    *fpp = fd->nextHashedField;

    dbTable* table = (dbTable*)putRow(fd->defTable->tableId);
    dbField* field = (dbField*)((char*)table + table->fields.offs);
    field[fd->fieldNo].hashTable = 0;
}

void dbDatabase::dropIndex(dbFieldDescriptor* fd)
{
    beginTransaction(dbExclusiveLock);
    modified = true;
    if (fd->type == dbField::tpRectangle) {
        dbRtree::drop(this, fd->tTree);
    } else {
        dbTtree::drop(this, fd->tTree);
    }
    fd->tTree = 0;
    fd->indexType &= ~INDEXED;

    dbFieldDescriptor** fpp = &fd->defTable->indexedFields;
    while (*fpp != fd) {
        fpp = &(*fpp)->nextIndexedField;
    }
    *fpp = fd->nextIndexedField;

    dbTable* table = (dbTable*)putRow(fd->defTable->tableId);
    dbField* field = (dbField*)((char*)table + table->fields.offs);
    field[fd->fieldNo].tTree = 0;
}

void dbDatabase::createHashTable(dbFieldDescriptor* fd)
{
    beginTransaction(dbExclusiveLock);
    modified = true;
    dbTable* table = (dbTable*)getRow(fd->defTable->tableId);
    int nRows = table->nRows;
    fd->hashTable = dbHashTable::allocate(this, 2*nRows);
    fd->attr &= ~dbFieldDescriptor::Updated;
    fd->nextHashedField = fd->defTable->hashedFields;
    fd->defTable->hashedFields = fd;
    fd->indexType |= HASHED;
    table = (dbTable*)putRow(fd->defTable->tableId);
    dbField* field = (dbField*)((char*)table + table->fields.offs);
    field[fd->fieldNo].hashTable = fd->hashTable;

    for (oid_t oid = table->firstRow; oid != 0; oid = getRow(oid)->next) {
        dbHashTable::insert(this, fd->hashTable, oid, fd->type, fd->dbsSize, fd->dbsOffs,
                            nRows);
    }
}


void dbDatabase::createIndex(dbFieldDescriptor* fd)
{
    beginTransaction(dbExclusiveLock);
    modified = true;
    if (fd->type == dbField::tpRectangle) {
        fd->tTree = dbRtree::allocate(this);
    } else {
        fd->tTree = dbTtree::allocate(this);
    }
    fd->attr &= ~dbFieldDescriptor::Updated;
    fd->nextIndexedField = fd->defTable->indexedFields;
    fd->defTable->indexedFields = fd;
    fd->indexType |= INDEXED;
    dbTable* table = (dbTable*)putRow(fd->defTable->tableId);
    dbField* field = (dbField*)((char*)table + table->fields.offs);
    field[fd->fieldNo].tTree = fd->tTree;

    for (oid_t oid = table->firstRow; oid != 0; oid = getRow(oid)->next) {
        if (fd->type == dbField::tpRectangle) {
            dbRtree::insert(this, fd->tTree, oid, fd->dbsOffs);
        } else {
            dbTtree::insert(this, fd->tTree, oid, fd->type, fd->dbsSize, fd->comparator, fd->dbsOffs);
        }
    }
}

void dbDatabase::dropTable(dbTableDescriptor* desc)
{
    deleteTable(desc);
    freeRow(dbMetaTableId, desc->tableId);

    dbFieldDescriptor* fd;
    for (fd = desc->hashedFields; fd != NULL; fd = fd->nextHashedField) {
        dbHashTable::drop(this, fd->hashTable);
    }
    for (fd = desc->indexedFields; fd != NULL; fd = fd->nextIndexedField) {
        if (fd->type == dbField::tpRectangle) {
            dbRtree::drop(this, fd->tTree);
        } else {
            dbTtree::drop(this, fd->tTree);
        }
    }
}

#define NEW_INDEX 0x80000000u

bool dbDatabase::addIndices(bool alter, dbTableDescriptor* desc)
{
    dbFieldDescriptor* fd;
    oid_t tableId = desc->tableId;
    dbTable* table = (dbTable*)getRow(tableId);
    int nRows = table->nRows;
    oid_t firstId = table->firstRow;
    int nNewIndices = 0;
    int nDelIndices = 0;
    for (fd = desc->firstField; fd != NULL; fd = fd->nextField) {
        if ((fd->indexType & HASHED) && fd->type != dbField::tpStructure) {
            if (fd->hashTable == 0) {
                if (!alter && tableId < committedIndexSize
                    && index[0][tableId] == index[1][tableId])
                {
                    // If there are some other active applications which
                    // can use this table, then they will not know
                    // about newly created index, which leads to inconsistency
                    return false;
                }
                fd->indexType |= NEW_INDEX;
                fd->hashTable = dbHashTable::allocate(this, nRows);
                nNewIndices += 1;
                TRACE_MSG(("Create hash table for field '%s'\n", fd->name));
            }
        } else if (fd->hashTable != 0) {
            if (alter) {
                TRACE_MSG(("Remove hash table for field '%s'\n", fd->name));
                nDelIndices += 1;
                fd->hashTable = 0;
            } else {
                return false;
            }
        }
        if ((fd->indexType & INDEXED) && fd->type != dbField::tpStructure) {
            if (fd->tTree == 0) {
                if (!alter && tableId < committedIndexSize
                    && index[0][tableId] == index[1][tableId])
                {
                    return false;
                }
                fd->indexType |= NEW_INDEX;
                fd->tTree = (fd->type == dbField::tpRectangle) ? dbRtree::allocate(this) : dbTtree::allocate(this);
                nNewIndices += 1;
                TRACE_MSG(("Create index for field '%s'\n", fd->name));
            }
        } else if (fd->tTree != 0) {
            if (alter) {
                nDelIndices += 1;
                TRACE_MSG(("Remove index for field '%s'\n", fd->name));
                fd->tTree = 0;
            } else {
                return false;
            }
        }
    }
    if (nNewIndices > 0) {
        modified = true;
        for (oid_t rowId = firstId; rowId != 0; rowId = getRow(rowId)->next) {
            for (fd = desc->hashedFields; fd != NULL; fd=fd->nextHashedField) {
                if (fd->indexType & NEW_INDEX) {
                    dbHashTable::insert(this, fd->hashTable, rowId,
                                        fd->type, fd->dbsSize, fd->dbsOffs, 2*nRows);
                }
            }
            for (fd=desc->indexedFields; fd != NULL; fd=fd->nextIndexedField) {
                if (fd->indexType & NEW_INDEX) {
                    if (fd->type == dbField::tpRectangle) {
                        dbRtree::insert(this, fd->tTree, rowId, fd->dbsOffs);
                    } else {
                        dbTtree::insert(this, fd->tTree, rowId,
                                        fd->type, fd->dbsSize, fd->comparator, fd->dbsOffs);
                    }
                }
            }
        }
        for (fd = desc->firstField; fd != NULL; fd = fd->nextField) {
            fd->indexType &= ~NEW_INDEX;
        }
    }
    if (nNewIndices + nDelIndices != 0) {
        table = (dbTable*)putRow(tableId);
        offs_t fieldOffs = currIndex[tableId] + table->fields.offs;
        for (fd = desc->firstField; fd != NULL; fd = fd->nextField) {
            dbField* field = (dbField*)(baseAddr + fieldOffs);
            if (field->hashTable != fd->hashTable) {
                if (field->hashTable != 0) {
                    assert(fd->hashTable == 0);
                    modified = true;
                    dbHashTable::drop(this, field->hashTable);
                    field = (dbField*)(baseAddr + fieldOffs);
                }
                field->hashTable = fd->hashTable;
            }
            if (field->tTree != fd->tTree) {
                if (field->tTree != 0) {
                    assert(fd->tTree == 0);
                    modified = true;
                    if (field->type == dbField::tpRectangle) {
                        dbRtree::drop(this, field->tTree);
                    } else {
                        dbTtree::drop(this, field->tTree);
                    }
                    field = (dbField*)(baseAddr + fieldOffs);
                }
                field->tTree = fd->tTree;
            }
            fieldOffs += sizeof(dbField);
        }
    }
    return true;
}


void dbDatabase::updateTableDescriptor(dbTableDescriptor* desc, oid_t tableId)
{
    size_t newSize = sizeof(dbTable) + desc->nFields*sizeof(dbField)
        + desc->totalNamesLength();
    linkTable(desc, tableId);

    dbTable* table = (dbTable*)getRow(tableId);
    int   nRows = table->nRows;
    oid_t first = table->firstRow;
    oid_t last = table->lastRow;
#ifdef AUTOINCREMENT_SUPPORT
    desc->autoincrementCount = table->count;
#endif

    int nFields = table->fields.size;
    offs_t fieldOffs = currIndex[tableId] + table->fields.offs;

    while (--nFields >= 0) {
        dbField* field = (dbField*)(baseAddr + fieldOffs);
        fieldOffs += sizeof(dbField);

        oid_t hashTableId = field->hashTable;
        oid_t tTreeId = field->tTree;
        int fieldType = field->type;
        if (hashTableId != 0) {
            dbHashTable::drop(this, hashTableId);
        }
        if (tTreeId != 0) {
            if (fieldType == dbField::tpRectangle) {
                dbRtree::drop(this, tTreeId);
            } else {
                dbTtree::drop(this, tTreeId);
            }
        }
    }

    table = (dbTable*)putRow(tableId, newSize);
    desc->storeInDatabase(table);
    table->firstRow = first;
    table->lastRow = last;
    table->nRows = nRows;
}

oid_t dbDatabase::addNewTable(dbTableDescriptor* desc)
{
    oid_t tableId = allocateRow(dbMetaTableId,
                                sizeof(dbTable) + desc->nFields*sizeof(dbField)
                                + desc->totalNamesLength());
    linkTable(desc, tableId);
    desc->storeInDatabase((dbTable*)getRow(tableId));
    return tableId;
}

void dbDatabase::close()
{
    detach();
    if (backupFileName != NULL) {
        {
            dbCriticalSection cs(backupMutex);
            delete[] backupFileName;
            backupFileName = NULL;
            backupInitEvent.signal();
        }
        backupThread.join();
    }
    if (commitDelay != 0) {
        delayedCommitStopTimerEvent.signal();
        {
            dbCriticalSection cs(delayedCommitStartTimerMutex);
            stopDelayedCommitThread = true;
            delayedCommitStartTimerEvent.signal();
        }
        commitDelay = 0;
        commitThread.join();
        delayedCommitStartTimerEvent.close();
        commitThreadSyncEvent.close();
    }
    {
        dbCriticalSection cs(threadContextListMutex);
        while (!threadContextList.isEmpty()) {
            delete (dbDatabaseThreadContext*)threadContextList.next;
        }
    }
    backupInitEvent.close();

#ifdef AUTO_DETECT_PROCESS_CRASH
    watchDogMutex->lock();
#endif

    if (accessType == dbConcurrentUpdate) {
        mutatorCS.enter();
    }
    cs.enter();

#ifdef AUTO_DETECT_PROCESS_CRASH
    dbL2List* wdc = &watchDogThreadContexts;
    while ((wdc = wdc->next) != &watchDogThreadContexts) {
        ((dbWatchDogContext*)wdc)->db = NULL;
    }
    if (watchDogThreadContexts.isEmpty()) {
        watchDogMutex->unlock();
        delete watchDogMutex;
    } else {
        watchDogThreadContexts.unlink();
        watchDogMutex->unlock();
    }
    selfWatchDog.close();
#endif

    delete[] databaseName;
    delete[] fileName;
    databaseName = NULL;
    fileName = NULL;
    opened = false;

    monitor->users -= 1;
    if (header != NULL && header->dirty && accessType != dbReadOnly && accessType != dbConcurrentRead && monitor->nWriters == 0)
    {
        file.flush(true);
        header->dirty = false;
        file.markAsDirty(0, sizeof(dbHeader));
    }
    cs.leave();
    if (accessType == dbConcurrentUpdate) {
        mutatorCS.leave();
    }
    dbTableDescriptor *desc, *next;
    for (desc = tables; desc != NULL; desc = next) {
        next = desc->nextDbTable;
        desc->tableId = 0;
        if (desc->cloneOf != NULL) {
            delete desc;
        } else if (!desc->fixedDatabase) {
            desc->db = NULL;
        }
    }

    file.close();
    if (initMutex.close()) {
        cs.erase();
        readSem.erase();
        writeSem.erase();
        upgradeSem.erase();
        backupCompletedEvent.erase();
        file.erase();
        if (commitDelay != 0) {
            delayedCommitStopTimerEvent.erase();
        }
        if (accessType == dbConcurrentUpdate || accessType == dbConcurrentRead) {
            mutatorCS.erase();
        }
        shm.erase();
        initMutex.erase();
    } else {
        cs.close();
        shm.close();
        readSem.close();
        writeSem.close();
        upgradeSem.close();
        backupCompletedEvent.close();
        if (commitDelay != 0) {
            delayedCommitStopTimerEvent.close();
        }
        if (accessType == dbConcurrentUpdate || accessType == dbConcurrentRead) {
            mutatorCS.close();
        }
    }
}

dbTableDescriptor* dbDatabase::lookupTable(dbTableDescriptor* origDesc)
{
    for (dbTableDescriptor* desc = tables; desc != NULL; desc = desc->nextDbTable)
    {
        if (desc == origDesc || desc->cloneOf == origDesc) {
            return desc;
        }
    }
    return NULL;
}

void dbDatabase::attach()
{
    if (threadContext.get() == NULL) {
        dbDatabaseThreadContext* ctx = new dbDatabaseThreadContext();
        {
            dbCriticalSection cs(threadContextListMutex);
            threadContextList.link(ctx);
        }
        attach(ctx);
    }
}


void dbDatabase::attach(dbDatabaseThreadContext* ctx)
{
    threadContext.set(ctx);
}

void dbDatabase::detach(int flags)
{
    if (flags & COMMIT) {
        commit();
    } else {
        monitor->uncommittedChanges = true;
        precommit();
    }
    if (flags & DESTROY_CONTEXT) {
        dbDatabaseThreadContext* ctx = threadContext.get();
        if (commitDelay != 0) {
            dbCriticalSection cs(delayedCommitStopTimerMutex);
            if (monitor->delayedCommitContext == ctx && ctx->commitDelayed) {
                ctx->removeContext = true;
            } else {
                dbCriticalSection cs(threadContextListMutex);
                delete ctx;
            }
        } else {
            dbCriticalSection cs(threadContextListMutex);
            delete ctx;
        }
        threadContext.set(NULL);
    }
}


bool dbDatabase::existsInverseReference(dbExprNode* expr, int nExistsClauses)
{
    while (true) {
        switch (expr->cop) {
          case dbvmLoadSelfReference:
          case dbvmLoadSelfArray:
            return expr->ref.field->inverseRef != NULL;
          case dbvmLoadReference:
            if (expr->ref.field->attr & dbFieldDescriptor::ComponentOfArray) {
                expr = expr->ref.base;
                continue;
            }
            // no break
          case dbvmLoadArray:
            if (expr->ref.field->inverseRef == NULL) {
                return false;
            }
            expr = expr->ref.base;
            continue;
          case dbvmGetAt:
            if (expr->operand[1]->cop != dbvmVariable
                || expr->operand[1]->offs != --nExistsClauses)
            {
                return false;
            }
            expr = expr->operand[0];
            continue;
          case dbvmDeref:
            expr = expr->operand[0];
            continue;
          default:
            return false;
        }
    }
}

bool dbDatabase::followInverseReference(dbExprNode* expr, dbExprNode* andExpr,
                                        dbAnyCursor* cursor, oid_t iref)
{
    while (expr->cop == dbvmGetAt || expr->cop == dbvmDeref ||
           (expr->cop == dbvmLoadReference
            && (expr->ref.field->attr & dbFieldDescriptor::ComponentOfArray)))
    {
        expr = expr->operand[0];
    }
    dbTable* table = (dbTable*)getRow(cursor->table->tableId);
    dbFieldDescriptor* fd = expr->ref.field->inverseRef;
    if (fd->type == dbField::tpArray) {
        byte* rec = (byte*)getRow(iref);
        dbVarying* arr = (dbVarying*)(rec + fd->dbsOffs);
        oid_t* refs = (oid_t*)(rec + arr->offs);
        if (expr->cop >= dbvmLoadSelfReference) {
            for (int n = arr->size; --n >= 0;) {
                oid_t oid = *refs++;
                if (oid != 0) {
                    if (andExpr == NULL || evaluate(andExpr, oid, table, cursor)) {
                        if (!cursor->add(oid)) {
                            return false;
                        }
                    }
                }
            }
        } else {
            for (int n = arr->size; --n >= 0;) {
                oid_t oid = *refs++;
                if (oid != 0) {
                    if (!followInverseReference(expr->ref.base, andExpr,
                                                cursor, oid))
                    {
                        return false;
                    }
                }
            }
        }
    } else {
        assert(fd->type == dbField::tpReference);
        oid_t oid = *(oid_t*)((byte*)getRow(iref) + fd->dbsOffs);
        if (oid != 0) {
            if (expr->cop >= dbvmLoadSelfReference) {
                if (andExpr == NULL || evaluate(andExpr, oid, table, cursor)) {
                    if (!cursor->add(oid)) {
                        return false;
                    }
                }
            } else {
                if (!followInverseReference(expr->ref.base, andExpr,
                                            cursor, oid))
                {
                    return false;
                }
            }
        }
    }
    return true;
}


static int referenceComparator(void* p, void* q, size_t) {
    return *(oid_t*)p < *(oid_t*)q ? -1 : *(oid_t*)p == *(oid_t*)q ? 0 : 1;
}

bool dbDatabase::isPrefixSearch(dbAnyCursor* cursor,
                                dbExprNode* expr,
                                dbExprNode* andExpr,
                                dbFieldDescriptor* &indexedField)
{
    if (expr->cop == dbvmLikeString
        && expr->operand[1]->cop ==  dbvmStringConcat
        && expr->operand[1]->operand[0]->cop == dbvmLoadSelfString
        && expr->operand[1]->operand[0]->ref.field->tTree != 0
        && expr->operand[1]->operand[1]->cop == dbvmLoadStringConstant
        && strcmp(expr->operand[1]->operand[1]->svalue.str, "%") == 0)
    {
        size_t paramBase = (size_t)cursor->paramBase;
        char* sval;
        dbExprNode* opd = expr->operand[0];
        switch (opd->cop) {
          case dbvmLoadStringConstant:
            sval = (char*)opd->svalue.str;
            break;
          case dbvmLoadVarString:
            sval = (char*)((char*)opd->var + paramBase);
            break;
#ifdef USE_STD_STRING
          case dbvmLoadVarStdString:
            sval = (char*)((std::string*)((char*)opd->var + paramBase))->c_str();
            break;
#endif
          case dbvmLoadVarStringPtr:
            sval = *(char**)((char*)opd->var + paramBase);
            break;
          default:
            return false;
        }
        dbFieldDescriptor* field = expr->operand[1]->operand[0]->ref.field;
        dbSearchContext sc;
        sc.db = this;
        sc.type = dbField::tpString;
        sc.offs = field->dbsOffs;
        sc.comparator = field->comparator;
        sc.cursor = cursor;
        sc.prefixLength = 0;
        sc.condition = andExpr ? andExpr->operand[1] : (dbExprNode*)0;
        sc.probes = 0;
        sc.firstKey = sc.lastKey = sval;
        sc.firstKeyInclusion = sc.lastKeyInclusion = true;
        dbTtree::prefixSearch(this, field->tTree, sc);
        TRACE_MSG(("Index prefix search for field %s.%s: %d probes\n",
                   field->defTable->name, field->longName, sc.probes));
        indexedField = field;
        return true;
    }
    return false;
}



bool dbDatabase::isIndexApplicable(dbAnyCursor* cursor,
                                   dbExprNode* expr, dbExprNode* andExpr,
                                   dbFieldDescriptor* &indexedField)
{
    int nExistsClauses = 0;
    while (expr->cop == dbvmExists) {
        expr = expr->operand[0];
        nExistsClauses += 1;
    }
    int cmpCop = expr->cop;

    if (dbExprNode::nodeOperands[cmpCop] < 2 && cmpCop != dbvmIsNull) {
        return false;
    }
    if (isPrefixSearch(cursor, expr, andExpr, indexedField)) {
        return true;
    }
    unsigned loadCop = expr->operand[0]->cop;

    if (loadCop - dbvmLoadSelfBool > dbvmLoadSelfRawBinary - dbvmLoadSelfBool
        && loadCop - dbvmLoadBool > dbvmLoadRawBinary - dbvmLoadBool)
    {
        return false;
    }
    dbFieldDescriptor* field = expr->operand[0]->ref.field;
    if (field->hashTable == 0 && field->tTree == 0) {
        return false;
    }
    if (loadCop >= dbvmLoadSelfBool) {
        if (isIndexApplicable(cursor, expr, andExpr)) {
            indexedField = field;
            return true;
        }
    }
    else if (existsInverseReference(expr->operand[0]->ref.base, nExistsClauses))
    {
        dbAnyCursor tmpCursor(*field->defTable, dbCursorViewOnly, NULL);
        tmpCursor.paramBase = cursor->paramBase;
        if (isIndexApplicable(&tmpCursor, expr, NULL)) {
            expr = expr->operand[0]->ref.base;
            indexedField = field;
            cursor->checkForDuplicates();
            if (andExpr != NULL) {
                andExpr = andExpr->operand[1];
            }
            for (dbSelection::segment* curr = tmpCursor.selection.first;
                 curr != NULL;
                 curr = curr->next)
            {
                for (int i = 0, n = curr->nRows; i < n; i++) {
                    if (!followInverseReference(expr, andExpr,
                                                cursor, curr->rows[i]))
                    {
                        return true;
                    }
                }
            }
            return true;
        }
    } else if (expr->operand[0]->ref.base->cop == dbvmDeref) {
        dbExprNode* ref = expr->operand[0]->ref.base->operand[0];
        if (ref->cop == dbvmLoadSelfReference) {
            dbFieldDescriptor* refField = ref->ref.field;
            if (refField->hashTable == 0 && refField->tTree == 0) {
                return false;
            }
            assert(refField->type == dbField::tpReference);
            dbAnyCursor tmpCursor(*refField->defTable, dbCursorViewOnly, NULL);
            tmpCursor.paramBase = cursor->paramBase;
            if (isIndexApplicable(&tmpCursor, expr, NULL)) {
                oid_t oid;
                indexedField = refField;
                dbSearchContext sc;
                sc.db = this;
                sc.type = dbField::tpReference;
                sc.sizeofType = sizeof(oid_t);
                sc.offs = refField->dbsOffs;
                sc.cursor = cursor;
                sc.prefixLength = 0;
                sc.comparator = &referenceComparator;
                sc.condition = andExpr ? andExpr->operand[1] : (dbExprNode*)0;
                sc.firstKey = sc.lastKey = (char*)&oid;
                sc.firstKeyInclusion = sc.lastKeyInclusion = true;
                for (dbSelection::segment* curr = tmpCursor.selection.first;
                     curr != NULL;
                     curr = curr->next)
                {
                    for (int i = 0, n = curr->nRows; i < n; i++) {
                        oid = curr->rows[i];
                        sc.probes = 0;
                        if (refField->hashTable != 0) {
                            dbHashTable::find(this, refField->hashTable, sc);
                            TRACE_MSG(("Hash table search for field %s.%s: %d probes\n",
                                       refField->defTable->name, refField->longName, sc.probes));
                        } else {
                            dbTtree::find(this, refField->tTree, sc);
                            TRACE_MSG(("Index search for field %s.%s: %d probes\n",
                                       refField->defTable->name, refField->longName, sc.probes));
                        }
                    }
                }
                return true;
            }
        }
    }
    return false;
}

bool dbDatabase::isIndexApplicable(dbAnyCursor* cursor,
                                   dbExprNode* expr, dbExprNode* andExpr)
{
    int n = dbExprNode::nodeOperands[expr->cop];
    dbFieldDescriptor* field = expr->operand[0]->ref.field;
    dbSearchContext sc;
    size_t paramBase = (size_t)cursor->paramBase;

    union {
        bool  b;
        int1  i1;
        int2  i2;
        int4  i4;
        db_int8  i8;
        real4 f4;
        real8 f8;
        oid_t oid;
        char* s;
        rectangle*  r;
        dbAnyArray* a;
    } literal[2];

    bool strop = false;
    bool binop = false;
    char* s;
    literal[0].i8 = 0;
    literal[1].i8 = 0;

    for (int i = 0; i < n-1; i++) {
        bool  bval = false;
        db_int8  ival = 0;
        real8 fval = 0;
        oid_t oid = 0;
        char* sval = NULL;
        rectangle* rect = NULL;
        dbExprNode* opd = expr->operand[i+1];
        switch (opd->cop) {
          case dbvmLoadVarArrayOfOid:
          case dbvmLoadVarArrayOfInt4:
          case dbvmLoadVarArrayOfInt8:
            literal[i].a = (dbAnyArray*)((char*)opd->var + paramBase);
            continue;
          case dbvmLoadVarArrayOfOidPtr:
          case dbvmLoadVarArrayOfInt4Ptr:
          case dbvmLoadVarArrayOfInt8Ptr:
            literal[i].a = *(dbAnyArray**)((char*)opd->var + paramBase);
            continue;
          case dbvmLoadVarBool:
            bval = *(bool*)((char*)opd->var + paramBase);
            break;
          case dbvmLoadVarInt1:
            ival = *(int1*)((char*)opd->var + paramBase);
            break;
          case dbvmLoadVarInt2:
            ival = *(int2*)((char*)opd->var + paramBase);
            break;
          case dbvmLoadVarInt4:
            ival = *(int4*)((char*)opd->var + paramBase);
            break;
          case dbvmLoadVarInt8:
            ival = *(db_int8*)((char*)opd->var + paramBase);
            break;
          case dbvmLoadVarReference:
            oid = *(oid_t*)((char*)opd->var + paramBase);
            break;
          case dbvmLoadVarRectangle:
            rect = (rectangle*)((char*)opd->var + paramBase);
            break;
          case dbvmLoadNull:
            oid = 0;
            break;
          case dbvmLoadVarReal4:
            fval = *(real4*)((char*)opd->var + paramBase);
            break;
          case dbvmLoadVarReal8:
            fval = *(real8*)((char*)opd->var + paramBase);
            break;
          case dbvmLoadVarString:
            sval = (char*)opd->var + paramBase;
            strop = true;
            break;
#ifdef USE_STD_STRING
          case dbvmLoadVarStdString:
            sval = (char*)((std::string*)((char*)opd->var + paramBase))->c_str();
            strop = true;
            break;
#endif
          case dbvmLoadVarStringPtr:
            sval = *(char**)((char*)opd->var + paramBase);
            strop = true;
            break;
          case dbvmLoadTrue:
            bval = true;
            break;
          case dbvmLoadFalse:
            bval = false;
            break;
          case dbvmLoadIntConstant:
            ival = opd->ivalue;
            break;
          case dbvmLoadRealConstant:
            fval = opd->fvalue;
            break;
          case dbvmLoadStringConstant:
            sval = (char*)opd->svalue.str;
            strop = true;
            break;
          case dbvmLoadVarRawBinary:
            sval = (char*)((char*)opd->var + paramBase);
            strop = true;
            binop = true;
            break;
          default:
            return false;
        }
        switch (field->type) {
          case dbField::tpBool:
            literal[i].b = bval;
            break;
          case dbField::tpInt1:
            literal[i].i1 = (int1)ival;
            break;
          case dbField::tpInt2:
            literal[i].i2 = (int2)ival;
            break;
          case dbField::tpInt4:
            literal[i].i4 = (int4)ival;
            break;
          case dbField::tpInt8:
            literal[i].i8 = ival;
            break;
          case dbField::tpReference:
            literal[i].oid = oid;
            break;
          case dbField::tpRectangle:
            literal[i].r = rect;
            break;
          case dbField::tpReal4:
            literal[i].f4 = (real4)fval;
            break;
          case dbField::tpReal8:
            literal[i].f8 = fval;
            break;
          case dbField::tpString:
            literal[i].s = sval;
            break;
          case dbField::tpRawBinary:
            literal[i].s = sval;
            break;
          default:
            assert(false);
        }
    }
    sc.db = this;
    sc.type = field->type;
    sc.sizeofType = field->dbsSize;
    sc.offs = field->dbsOffs;
    sc.comparator = field->comparator;
    sc.cursor = cursor;
    sc.prefixLength = 0;
    sc.condition = andExpr ? andExpr->operand[1] : (dbExprNode*)0;
    sc.probes = 0;

    switch (expr->cop) {
      case dbvmInArrayInt4:
        if (field->type == dbField::tpInt4) {
            dbAnyArray* arr = literal[0].a;
            db_int4* items = (db_int4*)arr->base();
            sc.firstKeyInclusion = sc.lastKeyInclusion = true;
            cursor->checkForDuplicates();
            if (field->hashTable != 0) {
                for (int n = (int)arr->length(); --n >= 0; items++) {
                    sc.firstKey = sc.lastKey = (char*)items;
                    dbHashTable::find(this, field->hashTable, sc);
                }
            } else {
                for (int n = (int)arr->length(); --n >= 0; items++) {
                    sc.firstKey = sc.lastKey = (char*)items;
                    dbTtree::find(this, field->tTree, sc);
                }
            }
            return true;
        }
        return false;

      case dbvmInArrayInt8:
        if (field->type == dbField::tpInt8) {
            dbAnyArray* arr = literal[0].a;
            db_int8* items = (db_int8*)arr->base();
            sc.firstKeyInclusion = sc.lastKeyInclusion = true;
            cursor->checkForDuplicates();
            if (field->hashTable != 0) {
                for (int n = (int)arr->length(); --n >= 0; items++) {
                    sc.firstKey = sc.lastKey = (char*)items;
                    dbHashTable::find(this, field->hashTable, sc);
                }
            } else {
                for (int n = (int)arr->length(); --n >= 0; items++) {
                    sc.firstKey = sc.lastKey = (char*)items;
                    dbTtree::find(this, field->tTree, sc);
                }
            }
            return true;
        }
        return false;

      case dbvmInArrayReference:
        if (field->type == dbField::tpReference) {
            dbAnyArray* arr = literal[0].a;
            oid_t* items = (oid_t*)arr->base();
            sc.firstKeyInclusion = sc.lastKeyInclusion = true;
            cursor->checkForDuplicates();
            if (field->hashTable != 0) {
                for (int n = (int)arr->length(); --n >= 0; items++) {
                    sc.firstKey = sc.lastKey = (char*)items;
                    dbHashTable::find(this, field->hashTable, sc);
                }
            } else {
                for (int n = (int)arr->length(); --n >= 0; items++) {
                    sc.firstKey = sc.lastKey = (char*)items;
                    dbTtree::find(this, field->tTree, sc);
                }
            }
            return true;
        }
        return false;

      case dbvmInArrayRectangle:
        if (field->type == dbField::tpRectangle) {
            dbAnyArray* arr = literal[0].a;
            rectangle* items = (rectangle*)arr->base();
            sc.firstKeyInclusion = dbRtree::SUBSET;
            cursor->checkForDuplicates();
            for (int n = (int)arr->length(); --n >= 0; items++) {
                sc.firstKey = (char*)items;
                dbRtree::find(this, field->tTree, sc);
            }
            return true;
        }
        return false;

      case dbvmEqRectangle:
      case dbvmLtRectangle:
      case dbvmLeRectangle:
      case dbvmGtRectangle:
      case dbvmGeRectangle:
        sc.firstKey = (char*)literal[0].r;
        sc.firstKeyInclusion = dbRtree::EQUAL + expr->cop - dbvmEqRectangle;
        dbRtree::find(this, field->tTree, sc);
        TRACE_MSG(("Spatial index search for field %s.%s: %d probes\n",
                   field->defTable->name, field->longName, sc.probes));
        return true;

      case dbvmOverlapsRectangle:
        sc.firstKey = (char*)literal[0].r;
        sc.firstKeyInclusion = dbRtree::OVERLAPS;
        dbRtree::find(this, field->tTree, sc);
        TRACE_MSG(("Spatial index search for field %s.%s: %d probes\n",
                   field->defTable->name, field->longName, sc.probes));
        return true;

      case dbvmIsNull:
      case dbvmEqReference:
      case dbvmEqInt:
      case dbvmEqBool:
      case dbvmEqReal:
      case dbvmEqString:
      case dbvmEqBinary:
        sc.firstKey = sc.lastKey =
            strop ? literal[0].s : (char*)&literal[0];
        sc.firstKeyInclusion = sc.lastKeyInclusion = true;
        if (field->hashTable != 0) {
            dbHashTable::find(this, field->hashTable, sc);
            TRACE_MSG(("Hash table search for field %s.%s: %d probes\n",
                       field->defTable->name, field->longName, sc.probes));
        } else {
            dbTtree::find(this, field->tTree, sc);
            TRACE_MSG(("Index search for field %s.%s: %d probes\n",
                       field->defTable->name, field->longName, sc.probes));
        }
        return true;
      case dbvmGtInt:
      case dbvmGtReal:
      case dbvmGtString:
      case dbvmGtBinary:
        if (field->tTree != 0) {
            sc.firstKey = strop ? literal[0].s : (char*)&literal[0];
            sc.lastKey = NULL;
            sc.firstKeyInclusion = false;
            dbTtree::find(this, field->tTree, sc);
            TRACE_MSG(("Index search for field %s.%s: %d probes\n",
                       field->defTable->name, field->longName, sc.probes));
            return true;
        }
        return false;
      case dbvmGeInt:
      case dbvmGeReal:
      case dbvmGeString:
      case dbvmGeBinary:
        if (field->tTree != 0) {
            sc.firstKey = strop ? literal[0].s : (char*)&literal[0];
            sc.lastKey = NULL;
            sc.firstKeyInclusion = true;
            dbTtree::find(this, field->tTree, sc);
            TRACE_MSG(("Index search for field %s.%s: %d probes\n",
                       field->defTable->name, field->longName, sc.probes));
            return true;
        }
        return false;
      case dbvmLtInt:
      case dbvmLtReal:
      case dbvmLtString:
      case dbvmLtBinary:
        if (field->tTree != 0) {
            sc.firstKey = NULL;
            sc.lastKey = strop ? literal[0].s : (char*)&literal[0];
            sc.lastKeyInclusion = false;
            dbTtree::find(this, field->tTree, sc);
            TRACE_MSG(("Index search for field %s.%s: %d probes\n",
                       field->defTable->name, field->longName, sc.probes));
            return true;
        }
        return false;
      case dbvmLeInt:
      case dbvmLeReal:
      case dbvmLeString:
      case dbvmLeBinary:
        if (field->tTree != 0) {
            sc.firstKey = NULL;
            sc.lastKey = strop ? literal[0].s : (char*)&literal[0];
            sc.lastKeyInclusion = true;
            dbTtree::find(this, field->tTree, sc);
            TRACE_MSG(("Index search for field %s.%s: %d probes\n",
                       field->defTable->name, field->longName, sc.probes));
            return true;
        }
        return false;
      case dbvmBetweenInt:
      case dbvmBetweenReal:
      case dbvmBetweenString:
      case dbvmBetweenBinary:
        if (field->hashTable != 0 &&
            ((strop && ((binop && memcmp(literal[0].s, literal[1].s, sc.sizeofType) == 0)
                        || (!binop && strcmp(literal[0].s, literal[1].s) == 0)))
             || (!strop && literal[0].i8 == literal[1].i8)))
        {
            sc.firstKey = strop ? literal[0].s : (char*)&literal[0];
            dbHashTable::find(this, field->hashTable, sc);
            TRACE_MSG(("Hash table search for field %s.%s: %d probes\n",
                       field->defTable->name, field->longName, sc.probes));
            return true;
        } else if (field->tTree != 0) {
            sc.firstKey = strop ? literal[0].s : (char*)&literal[0];
            sc.firstKeyInclusion = true;
            sc.lastKey = strop ? literal[1].s : (char*)&literal[1];
            sc.lastKeyInclusion = true;
            dbTtree::find(this, field->tTree, sc);
            TRACE_MSG(("Index search for field %s.%s: %d probes\n",
                       field->defTable->name, field->longName, sc.probes));
            return true;
        }
        return false;
      case dbvmLikeString:
      case dbvmLikeEscapeString:
        if ((s = findWildcard(literal[0].s, literal[1].s)) == NULL
            || ((s[1] == '\0' || s != literal[0].s) && field->tTree != 0))
        {
            if (s == NULL) {
                sc.firstKey = sc.lastKey = literal[0].s;
                sc.firstKeyInclusion = sc.lastKeyInclusion = true;
                if (field->hashTable != 0) {
                    dbHashTable::find(this, field->hashTable, sc);
                    TRACE_MSG(("Hash table search for field %s.%s: "
                               "%d probes\n", field->defTable->name,
                               field->longName, sc.probes));
                } else {
                    dbTtree::find(this, field->tTree, sc);
                    TRACE_MSG(("Index search for field %s.%s: %d probes\n",
                               field->defTable->name, field->longName,
                               sc.probes));
                }
            } else {
                int len = s - literal[0].s;
                if (len == 0) {
                    if (*s != dbMatchAnySubstring) {
                        return false;
                    }
                    sc.firstKey = NULL;
                    sc.lastKey = NULL;
                    dbTtree::find(this, field->tTree, sc);
                    TRACE_MSG(("Use index for ordering records by field "
                               "%s.%s\n", field->defTable->name,
                               field->longName));
                } else {
                    sc.firstKey = new char[len+1];
                    memcpy(sc.firstKey, literal[0].s, len);
                    sc.firstKey[len] = '\0';
                    sc.firstKeyInclusion = true;
                    sc.lastKey = NULL;
                    sc.prefixLength = len;
                    if (s[0] != dbMatchAnySubstring || s[1] != '\0') {
                        // Records selected by index do not necessarily
                        // match the pattern, so include pattern matching in
                        // condition expression
                        if (andExpr == NULL) {
                            if (expr->operand[0]->cop != dbvmLoadSelfString) {
                                dbExprNode load(dbvmLoadSelfString,
                                                expr->operand[0]->ref.field);
                                dbExprNode like(expr->cop, &load,
                                                expr->operand[1],
                                                expr->operand[2]);
                                sc.condition = &like;
                                dbTtree::find(this, field->tTree, sc);
                                like.cop = dbvmVoid;// do not destruct operands
                            } else {
                                sc.condition = expr;
                                dbTtree::find(this, field->tTree, sc);
                            }
                        } else {
                            sc.condition = andExpr;
                            dbTtree::find(this, field->tTree, sc);
                        }
                    } else {
                        dbTtree::find(this, field->tTree, sc);
                    }
                    TRACE_MSG(("Index search for prefix in LIKE expression "
                               "for field %s.%s: %d probes\n",
                               field->defTable->name, field->longName,
                               sc.probes));
                    delete[] sc.firstKey;
                    delete[] sc.lastKey;
                }
            }
            return true;
        }
    }
    return false;
}


struct SearchThreadArgument {
    dbParallelQueryContext* ctx;
    int                     id;
};


static void thread_proc parallelSearch(void* arg)
{
    SearchThreadArgument* sa = (SearchThreadArgument*)arg;
    sa->ctx->search(sa->id);
}


void dbDatabase::traverse(dbAnyCursor* cursor, dbQuery& query)
{
    const int defaultStackSize = 1024;
    oid_t buf[defaultStackSize];
    oid_t *stack = buf;
    int   stackSize = defaultStackSize;
    int   sp = 0, len;
    dbAnyArray* arr;
    oid_t oid, *refs;
    dbTable* table = (dbTable*)getRow(cursor->table->tableId);

    void* root = (void*)query.root;
    switch (query.startFrom) {
      case dbCompiledQuery::StartFromFirst:
        oid = table->firstRow;
        if (oid != 0) {
            stack[sp++] = oid;
        }
        break;
      case dbCompiledQuery::StartFromLast:
        oid = table->lastRow;
        if (oid != 0) {
            stack[sp++] = oid;
        }
        break;
      case dbCompiledQuery::StartFromRef:
        oid = *(oid_t*)root;
        if (oid != 0) {
            stack[sp++] = oid;
        }
        break;
      case dbCompiledQuery::StartFromArrayPtr:
        root = *(dbAnyArray**)root;
        // no break
      case dbCompiledQuery::StartFromArray:
        arr = (dbAnyArray*)root;
        len = arr->length();
        if (len > stackSize) {
            stackSize = len;
            stack = new oid_t[stackSize];
        }
        refs = (oid_t*)arr->base();
        while (--len >= 0) {
            oid = refs[len];
            if (oid != 0) {
                stack[sp++] = oid;
            }
        }
        break;
      default:
        assert(false);
    }
    cursor->checkForDuplicates();
    dbExprNode* condition = query.tree;
    dbFollowByNode* follow = query.follow;

    while (sp != 0) {
        oid_t curr = stack[--sp];
        if (condition->cop == dbvmVoid || evaluate(condition, curr, table, cursor)) {
            if (!cursor->add(curr)) {
                break;
            }
        } else {
            cursor->mark(curr);
        }
        byte* record = (byte*)getRow(curr);
        for (dbFollowByNode* fp = follow; fp != NULL; fp = fp->next) {
            dbFieldDescriptor* fd = fp->field;
            if (fd->type == dbField::tpArray) {
                dbVarying* vp = (dbVarying*)(record + fd->dbsOffs);
                len = vp->size;
                if (sp + len > stackSize) {
                    int newSize = len > stackSize ? len*2 : stackSize*2;
                    oid_t* newStack = new oid_t[newSize];
                    memcpy(newStack, stack, stackSize*sizeof(oid_t));
                    stackSize = newSize;
                    if (stack != buf) {
                        delete[] stack;
                    }
                    stack = newStack;
                }
                refs = (oid_t*)(record + vp->offs);
                while (--len >= 0) {
                    oid = refs[len];
                    if (oid != 0 && !cursor->isMarked(oid)) {
                        stack[sp++] = oid;
                    }
                }
            } else {
                assert(fd->type == dbField::tpReference);
                if (sp == stackSize) {
                    int newSize = stackSize*2;
                    oid_t* newStack = new oid_t[newSize];
                    memcpy(newStack, stack, stackSize*sizeof(oid_t));
                    stackSize = newSize;
                    if (stack != buf) {
                        delete[] stack;
                    }
                    stack = newStack;
                }
                oid = *(oid_t*)(record + fd->dbsOffs);
                if (oid != 0 && !cursor->isMarked(oid)) {
                    stack[sp++] = oid;
                }
            }
        }
    }
    if (stack != buf) {
        delete[] stack;
    }
    if (query.order != NULL) {
        cursor->selection.sort(this, query.order);
    }
}


bool dbDatabase::prepareQuery(dbAnyCursor* cursor, dbQuery& query)
{
    if (cursor == NULL) {
        return false;
    }
    assert(opened);
    dbDatabaseThreadContext* ctx = threadContext.get();
    assert(ctx != NULL);
    {
        dbCriticalSection cs(query.mutex);
        query.mutexLocked = true;
        if (!query.compiled() || cursor->table != query.table || schemeVersion != query.schemeVersion) {
            query.schemeVersion = schemeVersion;
            if (!ctx->compiler.compile(cursor->table, query)) {
                query.mutexLocked = false;
                return false;
            }
        }
        query.mutexLocked = false;
        return true;
    }
}

void dbDatabase::select(dbAnyCursor* cursor, dbQuery& query)
{
    assert(opened);
    dbDatabaseThreadContext* ctx = threadContext.get();
    dbFieldDescriptor* indexedField = NULL;
    assert(ctx != NULL);
    {
        dbCriticalSection cs(query.mutex);
        query.mutexLocked = true;
        if (!query.compiled() || cursor->table != query.table || schemeVersion != query.schemeVersion) {
            query.schemeVersion = schemeVersion;
            if (!ctx->compiler.compile(cursor->table, query)) {
                query.mutexLocked = false;
                return;
            }
        }
        query.mutexLocked = false;
    }
#if FASTDB_DEBUG == DEBUG_TRACE
    char buf[4096];
    if (query.elements != NULL) {
        TRACE_MSG(("Query:  select * from %s where %s\n", query.table->name,  query.dump(buf)));
    } else {
        TRACE_MSG(("Query:  select * from %s\n", query.table->name));
    }
#endif
    beginTransaction(cursor->type == dbCursorForUpdate
                     ? dbDatabase::dbExclusiveLock : dbDatabase::dbSharedLock);
    if (query.limitSpecified && query.order == NULL) {
        cursor->setStatementLimit(query);
    }

    if (query.startFrom != dbCompiledQuery::StartFromAny) {
        ctx->cursors.link(cursor);
        traverse(cursor, query);
        if (query.limitSpecified && query.order != NULL) {
            cursor->setStatementLimit(query);
            cursor->truncateSelection();
        }
        return;
    }

    dbExprNode* condition = query.tree;
    if (condition->cop == dbvmVoid && query.order == NULL && !query.limitSpecified) {
        // Empty select condition: select all records in the table
        select(cursor);
        return;
    }
    if (condition->cop == dbvmEqReference) {
        if (condition->operand[0]->cop == dbvmCurrent) {
            if (condition->operand[1]->cop == dbvmLoadVarReference) {
                cursor->setCurrent(*(dbAnyReference*)((char*)condition->operand[1]->var + (size_t)cursor->paramBase));
                return;
            } else if (condition->operand[1]->cop == dbvmIntToReference
                       && condition->operand[1]->operand[0]->cop == dbvmLoadIntConstant)
            {
                oid_t oid = (oid_t)condition->operand[1]->operand[0]->ivalue;
                cursor->setCurrent(*(dbAnyReference*)&oid);
                return;
            }
        }
        if (condition->operand[1]->cop == dbvmCurrent) {
            if (condition->operand[0]->cop == dbvmLoadVarReference) {
                cursor->setCurrent(*(dbAnyReference*)((char*)condition->operand[0]->var + (size_t)cursor->paramBase));
                return;
            } else if (condition->operand[0]->cop == dbvmIntToReference
                       && condition->operand[0]->operand[0]->cop == dbvmLoadIntConstant)
            {
                oid_t oid = (oid_t)condition->operand[0]->operand[0]->ivalue;
                cursor->setCurrent(*(dbAnyReference*)&oid);
                return;
            }
        }
    }
    ctx->cursors.link(cursor);

    if (condition->cop == dbvmAndBool) {
        if (isIndexApplicable(cursor, condition->operand[0],
                              condition, indexedField))
        {
            if (query.order != NULL) {
                if (indexedField->defTable != query.table
                    || query.order->next != NULL
                    || query.order->field != indexedField)
                {
                    cursor->selection.sort(this, query.order);
                } else if (!query.order->ascent) {
                    cursor->selection.reverse();
                }
                if (query.limitSpecified) {
                    cursor->setStatementLimit(query);
                    cursor->truncateSelection();
                }
            }
            return;
        }
    } else {
        if (condition->cop == dbvmOrBool) {
            cursor->checkForDuplicates();
        }
        while (condition->cop == dbvmOrBool
               && !cursor->isLimitReached()
               && isIndexApplicable(cursor, condition->operand[0], NULL,
                                    indexedField))
        {
            condition = condition->operand[1];
        }
        if (!cursor->isLimitReached()
            && isIndexApplicable(cursor, condition, NULL, indexedField))
        {
            if (query.order != NULL) {
                if (indexedField->defTable != query.table
                    || condition != query.tree
                    || query.order->next != NULL
                    || query.order->field != indexedField)
                {
                    cursor->selection.sort(this, query.order);
                } else if (!query.order->ascent) {
                    cursor->selection.reverse();
                }
                if (query.limitSpecified) {
                    cursor->setStatementLimit(query);
                    cursor->truncateSelection();
                }
            }
            return;
        }
    }
    if (query.order != NULL && query.order->next == NULL
        && query.order->field != NULL && query.order->field->type != dbField::tpRectangle
        && query.order->field->tTree != 0)
    {
        dbFieldDescriptor* field = query.order->field;
        TRACE_MSG(("Use index for ordering records by field %s.%s\n",
                   query.table->name, field->longName));
        if (query.limitSpecified) {
            cursor->setStatementLimit(query);
        }
        if (condition->cop == dbvmVoid) {
            if (query.order->ascent) {
                dbTtree::traverseForward(this, field->tTree, cursor);
            } else {
                dbTtree::traverseBackward(this, field->tTree, cursor);
            }
        } else {
            if (query.order->ascent) {
                dbTtree::traverseForward(this,field->tTree,cursor,condition);
            } else {
                dbTtree::traverseBackward(this,field->tTree,cursor,condition);
            }
        }
        return;
    }

    dbTable* table = (dbTable*)getRow(cursor->table->tableId);
    int n = parThreads-1;
    if (cursor->getNumberOfRecords() == 0
        && n > 0 && table->nRows >= parallelScanThreshold
        && cursor->limit >= dbDefaultSelectionLimit)
    {
        dbPooledThread* thread[dbMaxParallelSearchThreads];
        SearchThreadArgument sa[dbMaxParallelSearchThreads];
        dbParallelQueryContext par(this, table, &query, cursor);
        int i;
        for (i = 0; i < n; i++) {
            sa[i].id = i;
            sa[i].ctx = &par;
            thread[i] = threadPool.create((dbThread::thread_proc_t)parallelSearch, &sa[i]);
        }
        par.search(i);
        for (i = 0; i < n; i++) {
            threadPool.join(thread[i]);
        }
        if (query.order != NULL) {
            oid_t rec[dbMaxParallelSearchThreads];
            for (i = 0; i <= n; i++) {
                if (par.selection[i].first != NULL) {
                    rec[i] = par.selection[i].first->rows[0];
                } else {
                    rec[i] = 0;
                }
            }
            while (true) {
                int min = -1;
                for (i = 0; i <= n; i++) {
                    if (rec[i] != 0
                        && (min < 0 || dbSelection::compare(rec[i], rec[min],
                                                            query.order) < 0))
                    {
                        min = i;
                    }
                }
                if (min < 0) {
                    return;
                }
                oid_t oid =
                    par.selection[min].first->rows[par.selection[min].pos];
                cursor->selection.add(oid);
                par.selection[min].pos += 1;
                if (par.selection[min].pos == par.selection[min].first->nRows){
                    par.selection[min].pos = 0;
                    dbSelection::segment* next=par.selection[min].first->next;
                    delete par.selection[min].first;
                    if ((par.selection[min].first = next) == NULL) {
                        rec[min] = 0;
                        continue;
                    }
                }
                oid = par.selection[min].first->rows[par.selection[min].pos];
                rec[min] = oid;
            }
        } else {
            for (i = 0; i <= n; i++) {
                if (par.selection[i].first != NULL) {
                    par.selection[i].first->prev = cursor->selection.last;
                    if (cursor->selection.last == NULL) {
                        cursor->selection.first = par.selection[i].first;
                    } else {
                        cursor->selection.last->next = par.selection[i].first;
                    }
                    cursor->selection.last = par.selection[i].last;
                    cursor->selection.nRows += par.selection[i].nRows;
                }
            }
        }
    } else {
        oid_t oid = table->firstRow;
        if (!cursor->isLimitReached()) {
            while (oid != 0) {
                if (evaluate(condition, oid, table, cursor)) {
                    if (!cursor->add(oid)) {
                        break;
                    }
                }
                oid = getRow(oid)->next;
            }
        }
        if (query.order != NULL) {
            cursor->selection.sort(this, query.order);
        }
    }
    if (query.limitSpecified && query.order != NULL) {
        cursor->setStatementLimit(query);
        cursor->truncateSelection();
    }
}

void dbDatabase::select(dbAnyCursor* cursor)
{
    assert(opened);
    beginTransaction(cursor->type == dbCursorForUpdate ? dbExclusiveLock : dbSharedLock);
    dbTable* table = (dbTable*)getRow(cursor->table->tableId);
    cursor->firstId = table->firstRow;
    cursor->lastId = table->lastRow;
    cursor->selection.nRows = table->nRows;
    cursor->allRecords = true;
    threadContext.get()->cursors.link(cursor);
}


void dbDatabase::remove(dbTableDescriptor* desc, oid_t delId)
{
    modified = true;

    removeInverseReferences(desc, delId);

    dbFieldDescriptor* fd;
    for (fd = desc->hashedFields; fd != NULL; fd = fd->nextHashedField){
        dbHashTable::remove(this, fd->hashTable, delId, fd->type, fd->dbsSize, fd->dbsOffs);
    }
    for (fd = desc->indexedFields; fd != NULL; fd = fd->nextIndexedField) {
        if (fd->type == dbField::tpRectangle) {
            dbRtree::remove(this, fd->tTree, delId, fd->dbsOffs);
        } else {
            dbTtree::remove(this, fd->tTree, delId, fd->type, fd->dbsSize, fd->comparator, fd->dbsOffs);
        }
    }

    freeRow(desc->tableId, delId);
    updateCursors(delId, true);
}


dbRecord* dbDatabase::putRow(oid_t oid, size_t newSize)
{
    offs_t offs = currIndex[oid];
    if (oid < committedIndexSize && index[0][oid] == index[1][oid]) {
        size_t pageNo = oid/dbHandlesPerPage;
        monitor->dirtyPagesMap[pageNo >> 5] |= 1 << (pageNo & 31);
        cloneBitmap(offs, getRow(oid)->size);
        offs_t pos = allocate(newSize);
        currIndex[oid] = pos;
    } else {
        size_t oldSize = getRow(oid)->size;
        if (oldSize != newSize) {
            offs_t pos = allocate(newSize);
            currIndex[oid] = pos;
            cloneBitmap(offs, oldSize);
            deallocate(offs, oldSize);
        }
    }
    dbRecord* record = (dbRecord*)(baseAddr + currIndex[oid]);
    record->next = ((dbRecord*)(baseAddr + offs))->next;
    record->prev = ((dbRecord*)(baseAddr + offs))->prev;
    record->size = newSize;
    return record;
}

void dbDatabase::allocateRow(oid_t tableId, oid_t oid, size_t size)
{
    offs_t pos = allocate(size);
    currIndex[oid] = pos;
    dbTable* table = (dbTable*)putRow(tableId);
    dbRecord* record = getRow(oid);
    record->size = size;
    record->next = 0;
    record->prev = table->lastRow;
    if (table->lastRow != 0) {
        if (accessType == dbConcurrentUpdate) {
            putRow(table->lastRow)->next = oid;
            table = (dbTable*)getRow(tableId);
        } else {
            //
            // Optimisation hack: avoid cloning of the last record.
            // Possible inconsistency in L2-list will be eliminated by recovery
            // procedure.
            //
            getRow(table->lastRow)->next = oid;
            file.markAsDirty(currIndex[table->lastRow], sizeof(dbRecord));
        }
    } else {
        table->firstRow = oid;
    }
    table->lastRow = oid;
    table->nRows += 1;
#ifdef AUTOINCREMENT_SUPPORT
    table->count += 1;
#endif
}

void dbDatabase::freeRow(oid_t tableId, oid_t oid)
{
    dbTable* table = (dbTable*)putRow(tableId);
    dbRecord* del = getRow(oid);
    size_t size = del->size;
    oid_t next = del->next;
    oid_t prev = del->prev;
    table->nRows -= 1;
    if (prev == 0) {
        table->firstRow = next;
    }
    if (next == 0) {
        table->lastRow = prev;
    }
    if (prev != 0) {
        putRow(prev)->next = next;
    }
    if (next != 0) {
        putRow(next)->prev = prev;
    }
    if (oid < committedIndexSize && index[0][oid] == index[1][oid]) {
        cloneBitmap(currIndex[oid], size);
    } else {
        deallocate(currIndex[oid], size);
    }
    freeId(oid);
}

void dbDatabase::freeObject(oid_t oid)
{
    int marker = currIndex[oid] & dbInternalObjectMarker;
    if (oid < committedIndexSize && index[0][oid] == index[1][oid]) {
        cloneBitmap(currIndex[oid] - marker, internalObjectSize[marker]);
    } else {
        deallocate(currIndex[oid] - marker, internalObjectSize[marker]);
    }
    freeId(oid);
}


void dbDatabase::update(oid_t oid, dbTableDescriptor* desc, void const* record)
{
    assert(opened);
    beginTransaction(dbExclusiveLock);
    size_t size =
        desc->columns->calculateRecordSize((byte*)record, desc->fixedSize);

    byte* src = (byte*)record;
    desc->columns->markUpdatedFields((byte*)getRow(oid), src);
    updatedRecordId = oid;

    dbFieldDescriptor* fd;
    for (fd = desc->inverseFields; fd != NULL; fd = fd->nextInverseField) {
        if (fd->type == dbField::tpArray) {
            dbAnyArray* arr = (dbAnyArray*)(src + fd->appOffs);
            int n = arr->length();
            oid_t* newrefs = (oid_t*)arr->base();

            byte* old = (byte*)getRow(oid);
            int m = ((dbVarying*)(old + fd->dbsOffs))->size;
            int offs = ((dbVarying*)(old + fd->dbsOffs))->offs;
            int i, j, k;
            old += offs;

            for (i = j = 0; i < m; i++) {
                oid_t oldref = *((oid_t*)old + i);
                if (oldref != 0) {
                    for (k = j; j < n && newrefs[j] != oldref; j++);
                    if (j == n) {
                        for (j = k--; k >= 0 && newrefs[k] != oldref; k--);
                        if (k < 0) {
                            removeInverseReference(fd, oid, oldref);
                            old = (byte*)getRow(oid) + offs;
                        }
                    } else {
                        j += 1;
                    }
                }
            }
            for (i = j = 0; i < n; i++) {
                if (newrefs[i] != 0) {
                    for(k=j; j < m && newrefs[i] != *((oid_t*)old+j); j++);
                    if (j == m) {
                        for (j=k--; k >= 0 && newrefs[i] != *((oid_t*)old+k); k--);
                        if (k < 0) {
                            insertInverseReference(fd, oid, newrefs[i]);
                            old = (byte*)getRow(oid) + offs;
                        }
                    } else {
                        j += 1;
                    }
                }
            }
        } else {
            oid_t newref = *(oid_t*)(src + fd->appOffs);
            byte* old = (byte*)getRow(oid);
            oid_t oldref = *(oid_t*)(old + fd->dbsOffs);
            if (newref != oldref) {
                if (oldref != 0) {
                    removeInverseReference(fd, oid, oldref);
                }
                if (newref != 0) {
                    insertInverseReference(fd, oid, newref);
                }
            }
        }
    }
    updatedRecordId = 0;

    for (fd = desc->hashedFields; fd != NULL; fd = fd->nextHashedField) {
        if (fd->attr & dbFieldDescriptor::Updated) {
            dbHashTable::remove(this, fd->hashTable, oid, fd->type, fd->dbsSize, fd->dbsOffs);
        }
    }
    for (fd = desc->indexedFields; fd != NULL; fd = fd->nextIndexedField) {
        if (fd->attr & dbFieldDescriptor::Updated) {
            if (fd->type == dbField::tpRectangle) {
                dbRtree::remove(this, fd->tTree, oid, fd->dbsOffs);
            } else {
                dbTtree::remove(this, fd->tTree, oid, fd->type, fd->dbsSize, fd->comparator, fd->dbsOffs);
            }
        }
    }

    byte* old = (byte*)getRow(oid);
    byte* dst = (byte*)putRow(oid, size);
    if (dst == old) {
        dbSmallBuffer buf(size);
        byte* temp = (byte*)buf.base();
        desc->columns->storeRecordFields(temp, src, desc->fixedSize, dbFieldDescriptor::Update);
        memcpy(dst+sizeof(dbRecord), temp+sizeof(dbRecord), size-sizeof(dbRecord));
    } else {
        desc->columns->storeRecordFields(dst, src, desc->fixedSize, dbFieldDescriptor::Update);
    }
    modified = true;

    for (fd = desc->hashedFields; fd != NULL; fd = fd->nextHashedField) {
        if (fd->attr & dbFieldDescriptor::Updated) {
            dbHashTable::insert(this, fd->hashTable, oid, fd->type, fd->dbsSize,
                                fd->dbsOffs, 0);
        }
    }
    for (fd = desc->indexedFields; fd != NULL; fd = fd->nextIndexedField) {
        if (fd->attr & dbFieldDescriptor::Updated) {
            if (fd->type == dbField::tpRectangle) {
                dbRtree::insert(this, fd->tTree, oid, fd->dbsOffs);
            } else {
                dbTtree::insert(this, fd->tTree, oid, fd->type, fd->dbsSize, fd->comparator, fd->dbsOffs);
            }
            fd->attr &= ~dbFieldDescriptor::Updated;
        }
    }
    for (fd = desc->hashedFields; fd != NULL; fd = fd->nextHashedField) {
        fd->attr &= ~dbFieldDescriptor::Updated;
    }
    updateCursors(oid);
}


void dbDatabase::insertRecord(dbTableDescriptor* desc, dbAnyReference* ref,
                              void const* record)
{
    assert(opened);
    beginTransaction(dbExclusiveLock);
    modified = true;
    size_t size =
        desc->columns->calculateRecordSize((byte*)record, desc->fixedSize);

    dbTable* table = (dbTable*)getRow(desc->tableId);
#ifdef AUTOINCREMENT_SUPPORT
    desc->autoincrementCount = table->count + 1;
#endif
    int nRows = table->nRows+1;
    oid_t oid = allocateRow(desc->tableId, size);
    byte* src = (byte*)record;
    byte* dst = (byte*)getRow(oid);
    desc->columns->storeRecordFields(dst, src, desc->fixedSize, dbFieldDescriptor::Insert);
    ref->oid = oid;
    dbFieldDescriptor* fd;
    for (fd = desc->inverseFields; fd != NULL; fd = fd->nextInverseField) {
        if (fd->type == dbField::tpArray) {
            dbAnyArray* arr = (dbAnyArray*)(src + fd->appOffs);
            int n = arr->length();
            oid_t* refs = (oid_t*)arr->base();
            while (--n >= 0) {
                if (refs[n] != 0) {
                    insertInverseReference(fd, oid, refs[n]);
                }
            }
        } else {
            oid_t ref = *(oid_t*)(src + fd->appOffs);
            if (ref != 0) {
                insertInverseReference(fd, oid, ref);
            }
        }
    }
    for (fd = desc->hashedFields; fd != NULL; fd = fd->nextHashedField) {
        dbHashTable::insert(this, fd->hashTable, oid, fd->type, fd->dbsSize, fd->dbsOffs,
                            nRows);
    }
    for (fd = desc->indexedFields; fd != NULL; fd = fd->nextIndexedField) {
        if (fd->type == dbField::tpRectangle) {
            dbRtree::insert(this, fd->tTree, oid, fd->dbsOffs);
        } else {
            dbTtree::insert(this, fd->tTree, oid, fd->type, fd->dbsSize, fd->comparator, fd->dbsOffs);
        }
    }
}


inline void dbDatabase::extend(offs_t size)
{
    size_t oldSize = header->size;

    if (size > oldSize) {
#ifdef DISKLESS_CONFIGURATION
        handleError(FileLimitExeeded);
#endif
        if (fileSizeLimit != 0 && size > fileSizeLimit) {
            handleError(FileLimitExeeded);
        }
        dbDatabaseThreadContext* ctx = threadContext.get();
        assert(ctx != NULL);
        if (ctx->mutatorCSLocked && !ctx->writeAccess) {
            beginTransaction(dbCommitLock);
        }
        if (oldSize*2 > size) {
            if (fileSizeLimit == 0 || oldSize*2 <= fileSizeLimit) {
                if (offs_t(oldSize*2) == 0) { // overflow
                    handleError(FileLimitExeeded);
                }
                size = oldSize*2;
            } else if (fileSizeLimit > size) {
                size = fileSizeLimit;
            }
        }
        TRACE_MSG(("Extend database file from %ld to %ld bytes\n",
                   (long)header->size, (long)size));
        header->size = size;
        version = ++monitor->version;
        sprintf(databaseName + databaseNameLen, ".%d", version);
        int status = file.setSize(size, databaseName);
        if (status != dbFile::ok) {
            handleError(FileError, "Failed to extend file", status);
        }
        // file.markAsDirty(oldSize, size - oldSize);
        byte* addr = (byte*)file.getAddr();
        long shift = addr - baseAddr;
        if (shift != 0) {
            size_t base = (size_t)baseAddr;
            for (dbL2List* cursor = ctx->cursors.next;
                 cursor != &ctx->cursors;
                 cursor = cursor->next)
            {
                ((dbAnyCursor*)cursor)->adjustReferences(base, oldSize, shift);
            }
            baseAddr = addr;
            index[0] = (offs_t*)((char*)index[0] + shift);
            index[1] = (offs_t*)((char*)index[1] + shift);
            currIndex = (offs_t*)((char*)currIndex + shift);
            header = (dbHeader*)addr;
        }
    }
}


inline bool dbDatabase::wasReserved(offs_t pos, size_t size)
{
    for (dbLocation* location = reservedChain; location != NULL; location = location->next) {
        if (pos - location->pos < location->size || location->pos - pos < size) {
            return true;
        }
    }
    return false;
}

inline void dbDatabase::reserveLocation(dbLocation& location, offs_t pos, size_t size)
{
    location.pos = pos;
    location.size = size;
    location.next = reservedChain;
    reservedChain = &location;
}

inline void dbDatabase::commitLocation()
{
    reservedChain = reservedChain->next;
}

inline int ilog2(offs_t val)
{
    int    log;
    size_t pow;
    for (log = dbAllocationQuantumBits, pow = dbAllocationQuantum; pow <= val; pow <<= 1, log += 1);
    return log-1;
}

void dbDatabase::getMemoryStatistic(dbMemoryStatistic& stat)
{
    stat.free = 0;
    stat.used = 0;
    stat.nHoles = 0;
    stat.minHoleSize = (offs_t)header->size;
    stat.maxHoleSize = 0;
    for (int l = 0; l < dbDatabaseOffsetBits; l++) {
        stat.nHolesOfSize[l] = 0;
    }
    offs_t holeSize = 0;

    for (oid_t i = dbBitmapId; i < dbBitmapId + dbBitmapPages && currIndex[i] != dbFreeHandleMarker; i++){
        register byte* bitmap = get(i);
        for (size_t j = 0; j < dbPageSize; j++) {
            unsigned mask = bitmap[j];
            int count = 0;
            while (mask != 0) {
                while ((mask & 1) == 0) {
                    holeSize += 1;
                    mask >>= 1;
                    count += 1;
                }
                if (holeSize > 0) {
                    offs_t size = holeSize << dbAllocationQuantumBits;
                    if (size > stat.maxHoleSize) {
                        stat.maxHoleSize = size;
                    }
                    if (size < stat.minHoleSize) {
                        stat.minHoleSize = size;
                    }
                    stat.nHolesOfSize[ilog2(size)] += 1;
                    stat.free += size;
                    stat.nHoles += 1;
                    holeSize = 0;
                }
                while ((mask & 1) != 0) {
                    stat.used += dbAllocationQuantum;
                    count += 1;
                    mask >>= 1;
                }
            }
            holeSize += 8 - count;
        }
    }
    if (holeSize > 0) {
        offs_t size = holeSize << dbAllocationQuantumBits;
        if (size > stat.maxHoleSize) {
            stat.maxHoleSize = size;
        }
        if (size < stat.minHoleSize) {
            stat.minHoleSize = size;
        }
        stat.nHolesOfSize[ilog2(size)] += 1;
        stat.free += size;
        stat.nHoles += 1;
    }
}

bool dbDatabase::isFree(offs_t pos, int objBitSize)
{
    size_t quantNo = pos / dbAllocationQuantum;
    oid_t  pageId = dbBitmapId + quantNo / (dbPageSize*8);
    int    offs = quantNo % (dbPageSize*8) / 8;
    byte*  p = put(pageId) + offs;
    int    bitOffs = quantNo & 7;

    if (objBitSize > 8 - bitOffs) {
        objBitSize -= 8 - bitOffs;
        if ((*p++ & (-1 << bitOffs)) != 0) {
            return false;
        }
        offs += 1;
        while ((size_t)(objBitSize + offs*8) > dbPageSize*8) {
            int n = dbPageSize - offs;
            while (--n >= 0) {
                if (*p++ != 0) {
                    return false;
                }
            }
            p = put(++pageId);
            objBitSize -= (dbPageSize - offs)*8;
            offs = 0;
        }
        while ((objBitSize -= 8) > 0) {
            if (*p++ != 0) {
                return false;
            }
        }
        return (*p & ((1 << (objBitSize + 8)) - 1)) == 0;
    } else {
        return (*p & (((1 << objBitSize) - 1) << bitOffs)) == 0;
    }
}

void dbDatabase::markAsAllocated(offs_t pos, int objBitSize)
{
    size_t quantNo = pos / dbAllocationQuantum;
    oid_t  pageId = dbBitmapId + quantNo / (dbPageSize*8);
    int    offs = quantNo % (dbPageSize*8) / 8;
    byte*  p = put(pageId) + offs;
    int    bitOffs = quantNo & 7;

    if (objBitSize > 8 - bitOffs) {
        objBitSize -= 8 - bitOffs;
        *p++ |= -1 << bitOffs;
        offs += 1;
        while ((size_t)(objBitSize + offs*8) > dbPageSize*8) {
            memset(p, 0xFF, dbPageSize - offs);
            p = put(++pageId);
            objBitSize -= (dbPageSize - offs)*8;
            offs = 0;
        }
        while ((objBitSize -= 8) > 0) {
            *p++ = 0xFF;
        }
        *p |= (1 << (objBitSize + 8)) - 1;
    } else {
        *p |= ((1 << objBitSize) - 1) << bitOffs;
    }
}


offs_t dbDatabase::allocate(size_t size, oid_t oid)
{
     static byte const firstHoleSize [] = {
        8,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
        5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
        6,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
        5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
        7,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
        5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
        6,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
        5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0
    };
    static byte const lastHoleSize [] = {
        8,7,6,6,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };
    static byte const maxHoleSize [] = {
        8,7,6,6,5,5,5,5,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
        5,4,3,3,2,2,2,2,3,2,2,2,2,2,2,2,4,3,2,2,2,2,2,2,3,2,2,2,2,2,2,2,
        6,5,4,4,3,3,3,3,3,2,2,2,2,2,2,2,4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,1,
        5,4,3,3,2,2,2,2,3,2,1,1,2,1,1,1,4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,1,
        7,6,5,5,4,4,4,4,3,3,3,3,3,3,3,3,4,3,2,2,2,2,2,2,3,2,2,2,2,2,2,2,
        5,4,3,3,2,2,2,2,3,2,1,1,2,1,1,1,4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,1,
        6,5,4,4,3,3,3,3,3,2,2,2,2,2,2,2,4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,1,
        5,4,3,3,2,2,2,2,3,2,1,1,2,1,1,1,4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,0
    };
    static byte const maxHoleOffset [] = {
        0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,0,1,5,5,5,5,5,5,0,5,5,5,5,5,5,5,
        0,1,2,2,0,3,3,3,0,1,6,6,0,6,6,6,0,1,2,2,0,6,6,6,0,1,6,6,0,6,6,6,
        0,1,2,2,3,3,3,3,0,1,4,4,0,4,4,4,0,1,2,2,0,1,0,3,0,1,0,2,0,1,0,5,
        0,1,2,2,0,3,3,3,0,1,0,2,0,1,0,4,0,1,2,2,0,1,0,3,0,1,0,2,0,1,0,7,
        0,1,2,2,3,3,3,3,0,4,4,4,4,4,4,4,0,1,2,2,0,5,5,5,0,1,5,5,0,5,5,5,
        0,1,2,2,0,3,3,3,0,1,0,2,0,1,0,4,0,1,2,2,0,1,0,3,0,1,0,2,0,1,0,6,
        0,1,2,2,3,3,3,3,0,1,4,4,0,4,4,4,0,1,2,2,0,1,0,3,0,1,0,2,0,1,0,5,
        0,1,2,2,0,3,3,3,0,1,0,2,0,1,0,4,0,1,2,2,0,1,0,3,0,1,0,2,0,1,0,0
    };

    setDirty();

    size = DOALIGN(size, dbAllocationQuantum);
    int objBitSize = size >> dbAllocationQuantumBits;
    offs_t pos;
    oid_t i, firstPage, lastPage;
    int holeBitSize = 0;
    register int    alignment = size & (dbPageSize-1);
    const size_t    inc = dbPageSize/dbAllocationQuantum/8;
    register size_t offs;

    const int  pageBits = dbPageSize*8;
    int        holeBeforeFreePage  = 0;
    oid_t      freeBitmapPage = 0;
    dbLocation location;

    lastPage = dbBitmapId + dbBitmapPages;
    allocatedSize += size;

    if (alignment == 0) {
        firstPage = currPBitmapPage;
        offs = DOALIGN(currPBitmapOffs, inc);
    } else {
        int retries = -1;
        do {
            retries += 1;
            pos = fixedSizeAllocator.allocate(size);
        } while (pos != 0 && (wasReserved(pos, size) || !isFree(pos, objBitSize)));
        fixedSizeAllocator.retries = retries;

        if (pos != 0) {
            reserveLocation(location, pos, size);
            markAsAllocated(pos, objBitSize);
            commitLocation();
            file.markAsDirty(pos, size);
            return pos;
        }

        firstPage = currRBitmapPage;
        offs = currRBitmapOffs;
    }

    while (true) {
        if (alignment == 0) {
            // allocate page object
            for (i = firstPage; i < lastPage && currIndex[i] != dbFreeHandleMarker; i++){
                int spaceNeeded = objBitSize - holeBitSize < pageBits
                    ? objBitSize - holeBitSize : pageBits;
                if (bitmapPageAvailableSpace[i] <= spaceNeeded) {
                    holeBitSize = 0;
                    offs = 0;
                    continue;
                }
                register byte* begin = get(i);
                size_t startOffs = offs;
                while (offs < dbPageSize) {
                    if (begin[offs++] != 0) {
                        offs = DOALIGN(offs, inc);
                        holeBitSize = 0;
                    } else if ((holeBitSize += 8) == objBitSize) {
                        pos = ((offs_t(i-dbBitmapId)*dbPageSize + offs)*8
                               - holeBitSize) << dbAllocationQuantumBits;
                        if (wasReserved(pos, size)) {
                            offs += objBitSize >> 3;
                            startOffs = offs = DOALIGN(offs, inc);
                            holeBitSize = 0;
                            continue;
                        }
                        extend(pos + size);
                        reserveLocation(location, pos, size);
                        currPBitmapPage = i;
                        currPBitmapOffs = offs;
                        if (oid != 0) {
                            offs_t prev = currIndex[oid];
                            memcpy(baseAddr+pos,
                                   baseAddr+(prev&~dbInternalObjectMarker), size);
                            currIndex[oid] = (prev & dbInternalObjectMarker) + pos;
                        }
                        begin = put(i);
                        size_t holeBytes = holeBitSize >> 3;
                        if (holeBytes > offs) {
                            memset(begin, 0xFF, offs);
                            holeBytes -= offs;
                            begin = put(--i);
                            offs = dbPageSize;
                        }
                        while (holeBytes > dbPageSize) {
                            memset(begin, 0xFF, dbPageSize);
                            holeBytes -= dbPageSize;
                            bitmapPageAvailableSpace[i] = 0;
                            begin = put(--i);
                        }
                        memset(&begin[offs-holeBytes], 0xFF, holeBytes);
                        commitLocation();
                        file.markAsDirty(pos, size);
                        return pos;
                    }
                }
                if (startOffs == 0 && holeBitSize == 0
                    && spaceNeeded < bitmapPageAvailableSpace[i])
                {
                    bitmapPageAvailableSpace[i] = spaceNeeded;
                }
                offs = 0;
            }
        } else {
            for (i=firstPage; i<lastPage && currIndex[i] != dbFreeHandleMarker; i++){
                int spaceNeeded = objBitSize - holeBitSize < pageBits
                    ? objBitSize - holeBitSize : pageBits;
                if (bitmapPageAvailableSpace[i] <= spaceNeeded) {
                    holeBitSize = 0;
                    offs = 0;
                    continue;
                }
                register byte* begin = get(i);
                size_t startOffs = offs;

                while (offs < dbPageSize) {
                    int mask = begin[offs];
                    if (holeBitSize + firstHoleSize[mask] >= objBitSize) {
                        pos = ((offs_t(i-dbBitmapId)*dbPageSize + offs)*8
                               - holeBitSize) << dbAllocationQuantumBits;
                        if (wasReserved(pos, size)) {
                            startOffs = offs += (objBitSize + 7) >> 3;
                            holeBitSize = 0;
                            continue;
                        }
                        extend(pos + size);
                        reserveLocation(location, pos, size);
                        currRBitmapPage = i;
                        currRBitmapOffs = offs;
                        if (oid != 0) {
                            offs_t prev = currIndex[oid];
                            memcpy(baseAddr+pos,
                                   baseAddr+(prev&~dbInternalObjectMarker), size);
                            currIndex[oid] = (prev & dbInternalObjectMarker) + pos;
                        }
                        begin = put(i);
                        begin[offs] |= (1 << (objBitSize - holeBitSize)) - 1;
                        if (holeBitSize != 0) {
                            if (size_t(holeBitSize) > offs*8) {
                                memset(begin, 0xFF, offs);
                                holeBitSize -= offs*8;
                                begin = put(--i);
                                offs = dbPageSize;
                            }
                            while (holeBitSize > pageBits) {
                                memset(begin, 0xFF, dbPageSize);
                                holeBitSize -= pageBits;
                                bitmapPageAvailableSpace[i] = 0;
                                begin = put(--i);
                            }
                            while ((holeBitSize -= 8) > 0) {
                                begin[--offs] = 0xFF;
                            }
                            begin[offs-1] |= ~((1 << -holeBitSize) - 1);
                        }
                        commitLocation();
                        file.markAsDirty(pos, size);
                        return pos;
                    } else if (maxHoleSize[mask] >= objBitSize) {
                        int holeBitOffset = maxHoleOffset[mask];
                        pos = ((offs_t(i-dbBitmapId)*dbPageSize + offs)*8 +
                               holeBitOffset) << dbAllocationQuantumBits;
                        if (wasReserved(pos, size)) {
                            startOffs = offs += (objBitSize + 7) >> 3;
                            holeBitSize = 0;
                            continue;
                        }
                        extend(pos + size);
                        reserveLocation(location, pos, size);
                        currRBitmapPage = i;
                        currRBitmapOffs = offs;
                        if (oid != 0) {
                            offs_t prev = currIndex[oid];
                            memcpy(baseAddr+pos,
                                   baseAddr+(prev&~dbInternalObjectMarker), size);
                            currIndex[oid] = (prev & dbInternalObjectMarker) + pos;
                        }
                        begin = put(i);
                        begin[offs] |= ((1 << objBitSize) - 1) << holeBitOffset;
                        commitLocation();
                        file.markAsDirty(pos, size);
                        return pos;
                    }
                    offs += 1;
                    if (lastHoleSize[mask] == 8) {
                        holeBitSize += 8;
                    } else {
                        holeBitSize = lastHoleSize[mask];
                    }
                }
                if (startOffs == 0 && holeBitSize == 0
                    && spaceNeeded < bitmapPageAvailableSpace[i])
                {
                    bitmapPageAvailableSpace[i] = spaceNeeded;
                }
                offs = 0;
            }
        }
        if (firstPage == dbBitmapId) {
            if (freeBitmapPage > i) {
                i = freeBitmapPage;
                holeBitSize = holeBeforeFreePage;
            }
            if (i == dbBitmapId + dbBitmapPages) {
                handleError(OutOfMemoryError, NULL, size);
            }
            assert(currIndex[i] == dbFreeHandleMarker);

            size_t extension = (size > extensionQuantum)
                             ? size : extensionQuantum;
            int morePages =
                (extension + dbPageSize*(dbAllocationQuantum*8-1) - 1)
                / (dbPageSize*(dbAllocationQuantum*8-1));

            if (size_t(i + morePages) > dbBitmapId + dbBitmapPages) {
                morePages =
                    (size + dbPageSize*(dbAllocationQuantum*8-1) - 1)
                    / (dbPageSize*(dbAllocationQuantum*8-1));
                if (size_t(i + morePages) > dbBitmapId + dbBitmapPages) {
                    handleError(OutOfMemoryError, NULL, size);
                }
            }
            objBitSize -= holeBitSize;
            int skip = DOALIGN(objBitSize, dbPageSize/dbAllocationQuantum);
            pos = ((i-dbBitmapId) << (dbPageBits+dbAllocationQuantumBits+3))
                + (skip << dbAllocationQuantumBits);
            extend(pos + morePages*dbPageSize);
            file.markAsDirty(pos, morePages*dbPageSize);
            memset(baseAddr + pos, 0, morePages*dbPageSize);
            memset(baseAddr + pos, 0xFF, objBitSize>>3);
            *(baseAddr + pos + (objBitSize>>3)) = (1 << (objBitSize&7))-1;
            memset(baseAddr + pos + (skip>>3), 0xFF,
                   morePages*(dbPageSize/dbAllocationQuantum/8));

            oid_t j = i;
            while (--morePages >= 0) {
                monitor->dirtyPagesMap[j/dbHandlesPerPage/32]
                    |= 1 << int(j/dbHandlesPerPage & 31);
                currIndex[j++] = pos + dbPageObjectMarker;
                pos += dbPageSize;
            }
            freeBitmapPage = j;
            j = i + objBitSize / pageBits;
            if (alignment != 0) {
                currRBitmapPage = j;
                currRBitmapOffs = 0;
            } else {
                currPBitmapPage = j;
                currPBitmapOffs = 0;
            }
            while (j > i) {
                bitmapPageAvailableSpace[--j] = 0;
            }

            pos = (offs_t(i-dbBitmapId)*dbPageSize*8 - holeBitSize)
                << dbAllocationQuantumBits;
            if (oid != 0) {
                offs_t prev = currIndex[oid];
                memcpy(baseAddr + pos,
                       baseAddr + (prev & ~dbInternalObjectMarker), size);
                currIndex[oid] = (prev & dbInternalObjectMarker) + pos;
            }

            if (holeBitSize != 0) {
                reserveLocation(location, pos, size);
                while (holeBitSize > pageBits) {
                    holeBitSize -= pageBits;
                    memset(put(--i), 0xFF, dbPageSize);
                    bitmapPageAvailableSpace[i] = 0;
                }
                byte* cur = (byte*)put(--i) + dbPageSize;
                while ((holeBitSize -= 8) > 0) {
                    *--cur = 0xFF;
                }
                *(cur-1) |= ~((1 << -holeBitSize) - 1);
                commitLocation();
            }
            file.markAsDirty(pos, size);
            return pos;
        }
        freeBitmapPage = i;
        holeBeforeFreePage = holeBitSize;
        holeBitSize = 0;
        lastPage = firstPage + 1;
        firstPage = dbBitmapId;
        offs = 0;
    }
}

void dbDatabase::deallocate(offs_t pos, size_t size)
{
    assert(pos != 0 && (pos & (dbAllocationQuantum-1)) == 0);
    size_t quantNo = pos / dbAllocationQuantum;
    int    objBitSize = (size+dbAllocationQuantum-1) / dbAllocationQuantum;
    oid_t  pageId = dbBitmapId + quantNo / (dbPageSize*8);
    size_t offs = quantNo % (dbPageSize*8) / 8;
    byte*  p = put(pageId) + offs;
    int    bitOffs = quantNo & 7;

    if ((deallocatedSize += objBitSize*dbAllocationQuantum) >= freeSpaceReuseThreshold)
    {
        deallocatedSize = 0;
        currRBitmapPage = currPBitmapPage = dbBitmapId;
        currRBitmapOffs = currPBitmapOffs = 0;
    } else {
        if ((size_t(pos) & (dbPageSize-1)) == 0 && size >= dbPageSize) {
            if (pageId == currPBitmapPage && offs < currPBitmapOffs) {
                currPBitmapOffs = offs;
            }
        } else {
            if (fixedSizeAllocator.free(pos, size)) {
                deallocatedSize -= objBitSize*dbAllocationQuantum;
            } else if (pageId == currRBitmapPage && offs < currRBitmapOffs) {
                currRBitmapOffs = offs;
            }
        }
    }
    bitmapPageAvailableSpace[pageId] = INT_MAX;

    if (objBitSize > 8 - bitOffs) {
        objBitSize -= 8 - bitOffs;
        *p++ &= (1 << bitOffs) - 1;
        offs += 1;
        while (objBitSize + offs*8 > dbPageSize*8) {
            memset(p, 0, dbPageSize - offs);
            p = put(++pageId);
            bitmapPageAvailableSpace[pageId] = INT_MAX;
            objBitSize -= (dbPageSize - offs)*8;
            offs = 0;
        }
        while ((objBitSize -= 8) > 0) {
            *p++ = 0;
        }
        *p &= ~((1 << (objBitSize + 8)) - 1);
    } else {
        *p &= ~(((1 << objBitSize) - 1) << bitOffs);
    }
}

void dbDatabase::cloneBitmap(offs_t pos, size_t size)
{
    size_t quantNo = pos / dbAllocationQuantum;
    int    objBitSize = (size+dbAllocationQuantum-1) / dbAllocationQuantum;
    oid_t  pageId = dbBitmapId + quantNo / (dbPageSize*8);
    size_t offs = quantNo % (dbPageSize*8) / 8;
    int    bitOffs = quantNo & 7;

    put(pageId);
    if (objBitSize > 8 - bitOffs) {
        objBitSize -= 8 - bitOffs;
        offs += 1;
        while (objBitSize + offs*8 > dbPageSize*8) {
            put(++pageId);
            objBitSize -= (dbPageSize - offs)*8;
            offs = 0;
        }
    }
}


void dbDatabase::setDirty()
{
    if (!header->dirty) {
        if (accessType == dbReadOnly) {
            handleError(DatabaseReadOnly, "Attempt to modify readonly database");
        }
        header->dirty = true;
        file.markAsDirty(0, sizeof(dbHeader));
        file.flush(true);
    }
    monitor->modified = true;
    modified = true;
}

oid_t dbDatabase::allocateId(int n)
{
    setDirty();

    oid_t oid;
    int curr = 1-header->curr;
    if (n == 1) {
#ifdef DO_NOT_REUSE_OID_WITHIN_SESSION
        if (monitor->sessionFreeList[curr].tail != 0) {
            if ((oid = monitor->sessionFreeList[curr].head) != 0) {
                currIndex[monitor->sessionFreeList[curr].tail] = currIndex[oid];
                unsigned i = monitor->sessionFreeList[curr].tail / dbHandlesPerPage;
                monitor->dirtyPagesMap[i >> 5] |= 1 << (i & 31);

                monitor->sessionFreeList[curr].head = (oid_t)(currIndex[oid] - dbFreeHandleMarker);
                i = oid / dbHandlesPerPage;
                monitor->dirtyPagesMap[i >> 5] |= 1 << (i & 31);
                return oid;
            }
        }
 #endif
        if ((oid = header->root[curr].freeList) != 0) {
            header->root[curr].freeList = (oid_t)(currIndex[oid] - dbFreeHandleMarker);
            unsigned i = oid / dbHandlesPerPage;
            monitor->dirtyPagesMap[i >> 5] |= 1 << (i & 31);
            return oid;
        }
    }
    if (currIndexSize + n > header->root[curr].indexSize) {
        size_t oldIndexSize = header->root[curr].indexSize;
        size_t newIndexSize = oldIndexSize * 2;
        while (newIndexSize < oldIndexSize + n) {
            newIndexSize = newIndexSize*2;
        }
        TRACE_MSG(("Extend index size from %ld to %ld\n",
                   oldIndexSize, newIndexSize));
        offs_t newIndex = allocate(newIndexSize*sizeof(offs_t));
        offs_t oldIndex = header->root[curr].index;
        memcpy(baseAddr + newIndex, currIndex, currIndexSize*sizeof(offs_t));
        currIndex = index[curr] = (offs_t*)(baseAddr + newIndex);
        header->root[curr].index = newIndex;
        header->root[curr].indexSize = newIndexSize;
        deallocate(oldIndex, oldIndexSize*sizeof(offs_t));
    }
    oid = currIndexSize;
    header->root[curr].indexUsed = currIndexSize += n;
    return oid;
}

void dbDatabase::freeId(oid_t oid, int n)
{
    int curr = 1-header->curr;
    oid_t freeList = header->root[curr].freeList;
    while (--n >= 0) {
        unsigned i = oid / dbHandlesPerPage;
        monitor->dirtyPagesMap[i >> 5] |= 1 << (i & 31);
        currIndex[oid] = freeList + dbFreeHandleMarker;
#ifdef DO_NOT_REUSE_OID_WITHIN_SESSION
        if (monitor->sessionFreeList[curr].tail == 0) {
            monitor->sessionFreeList[curr].tail = oid;
            monitor->sessionFreeList[curr].head = freeList;
        }
#endif
        freeList = oid++;
    }
    header->root[curr].freeList = freeList;
}

#ifdef AUTO_DETECT_PROCESS_CRASH

void dbDatabase::watchDogThread(dbWatchDogContext* ctx) {
    dbMutex* mutex = ctx->mutex;
    if (ctx->watchDog.watch()) {
        mutex->lock();
        if (ctx->db != NULL) {
            ctx->db->cs.enter();
            ctx->db->revokeLock(ctx->clientId);
            ctx->db->cs.leave();
        }
        ctx->watchDog.close();
    } else {
        mutex->lock();
    }
    bool isEmpty = false;
    dbDatabase* db = ctx->db;
    if (db != NULL) {
        db->cs.enter();
        delete ctx;
        db->cs.leave();
    } else {
        isEmpty = ctx->isEmpty();
        delete ctx;
    }
    mutex->unlock();
    if (isEmpty) {
        delete mutex;
    }
}

void dbDatabase::revokeLock(int clientId)
{
    TRACE_MSG(("Revoke lock: writers %d, readers %d, lock owner %d, crashed process %d\n",
               monitor->nWriters, monitor->nReaders, monitor->exclusiveLockOwner, clientId));
    if (monitor->nWriters != 0 && monitor->exclusiveLockOwner == clientId) {
        if (accessType != dbReadOnly && accessType != dbConcurrentRead) {
            TRACE_MSG(("Revoke exclusive lock, start recovery\n"));
            checkVersion();
            recovery();
            TRACE_MSG(("Recovery completed\n"));
            monitor->exclusiveLockOwner = 0;
            monitor->nWriters -= 1;
            monitor->ownerPid.clear();
            assert(monitor->nWriters == 0 && !monitor->waitForUpgrade);
            if (monitor->nWaitWriters != 0) {
                monitor->nWaitWriters -= 1;
                monitor->nWriters = 1;
                writeSem.signal();
            } else if (monitor->nWaitReaders != 0) {
                monitor->nReaders = monitor->nWaitReaders;
                monitor->nWaitReaders = 0;
                readSem.signal(monitor->nReaders);
            }
        } else {
            handleError(Deadlock, "Owner of exclusive database lock is crashed");
        }
    } else {
        int nReaders = monitor->nReaders;
        for (int i = 0; i < nReaders; i++) {
            if (monitor->sharedLockOwner[i] == clientId) {
                TRACE_MSG(("Revoke shared lock\n"));
                while (++i < nReaders) {
                    monitor->sharedLockOwner[i-1] = monitor->sharedLockOwner[i];
                }
                monitor->sharedLockOwner[i-1] = 0;
                monitor->nReaders -= 1;
                if (monitor->nReaders == 1 && monitor->waitForUpgrade) {
                    assert(monitor->nWriters == 0);
#ifdef AUTO_DETECT_PROCESS_CRASH
                    removeLockOwner(monitor->upgradeId);
                    monitor->upgradeId = 0;
#endif
                    monitor->waitForUpgrade = false;
                    monitor->nWaitWriters -= 1;
                    monitor->nWriters = 1;
                    monitor->nReaders = 0;
                    upgradeSem.signal();
                } else if (monitor->nReaders == 0) {
                    if (monitor->nWaitWriters != 0) {
                        assert(monitor->nWriters == 0 && !monitor->waitForUpgrade);
                        monitor->nWaitWriters -= 1;
                        monitor->nWriters = 1;
                        writeSem.signal();
                    }
                }
                break;
            }
        }
    }
}


void dbDatabase::startWatchDogThreads()
{
    while (maxClientId < monitor->clientId) {
        int id = ++maxClientId;
        if (id != selfId) {
            sprintf(databaseName + databaseNameLen, ".pid.%d", id);
            dbWatchDogContext* ctx = new dbWatchDogContext();
            if (ctx->watchDog.open(databaseName)) {
                watchDogThreadContexts.link(ctx);
                ctx->clientId = id;
                ctx->db = this;
                ctx->mutex = watchDogMutex;
                ctx->thread.create((dbThread::thread_proc_t)watchDogThread, ctx);
            } else {
                revokeLock(id);
                delete ctx;
            }
        }
    }
}


void dbDatabase::addLockOwner()
{
    int nReaders = monitor->nReaders;
    assert(nReaders <= dbMaxReaders && nReaders > 0);
    while (monitor->sharedLockOwner[--nReaders] != 0) {
        assert(nReaders != 0);
    }
    monitor->sharedLockOwner[nReaders] = selfId;
}

void dbDatabase::removeLockOwner(int selfId)
{
    int id = 0;
    int i = monitor->nReaders;
    do {
        assert(i > 0);
        int nextId = monitor->sharedLockOwner[--i];
        monitor->sharedLockOwner[i] = id;
        id = nextId;
    } while (id != selfId);
}
#endif

bool dbDatabase::isCommitted()
{
    dbDatabaseThreadContext* ctx = threadContext.get();
    return ctx == NULL ||
           (!ctx->writeAccess && !ctx->readAccess && !ctx->mutatorCSLocked);
}

bool dbDatabase::isUpdateTransaction()
{
    dbDatabaseThreadContext* ctx = threadContext.get();
    return ctx != NULL && ctx->isMutator;
}

bool dbDatabase::isAttached()
{
    return threadContext.get() != NULL;
}

bool dbDatabase::beginTransaction(dbLockType lockType)
{
    dbDatabaseThreadContext* ctx = threadContext.get();

    if (commitDelay != 0 && lockType != dbCommitLock) {
        dbCriticalSection cs(delayedCommitStopTimerMutex);
        monitor->forceCommitCount += 1;
        if (monitor->delayedCommitContext == ctx && ctx->commitDelayed) {
            // skip delayed transaction because this thread is starting new transaction
            monitor->delayedCommitContext = NULL;
            ctx->commitDelayed = false;
            if (commitTimerStarted != 0) {
                time_t elapsed = time(NULL) - commitTimerStarted;
                if (commitTimeout < elapsed) {
                    commitTimeout = 0;
                } else {
                    commitTimeout -= elapsed;
                }
            }
            delayedCommitStopTimerEvent.signal();
        }
    }

    if (lockType != dbSharedLock) {
        ctx->isMutator = true;
    }
    if (accessType == dbConcurrentUpdate && lockType != dbCommitLock) {
        if (!ctx->mutatorCSLocked) {
            mutatorCS.enter();
            ctx->mutatorCSLocked = true;
#ifdef RECOVERABLE_CRITICAL_SECTION
            if (monitor->modified) {
                // mutatorCS lock was revoked
                monitor->modified = false;
                checkVersion();
                recovery();
            }
#endif
        }
    } else if (lockType != dbSharedLock) {
        if (!ctx->writeAccess) {
//            assert(accessType != dbReadOnly && accessType != dbConcurrentRead);
            cs.enter();
#ifdef AUTO_DETECT_PROCESS_CRASH
            startWatchDogThreads();
#endif
            if (ctx->readAccess) {
                assert(monitor->nWriters == 0);
                TRACE_MSG(("Attempt to upgrade lock from shared to exclusive can cause deadlock\n"));

                if (monitor->nReaders != 1) {
                    if (monitor->waitForUpgrade) {
                        handleError(Deadlock);
                    }
                    monitor->waitForUpgrade = true;
                    monitor->upgradeId = selfId;
                    monitor->nWaitWriters += 1;
                    cs.leave();

                    if (commitDelay != 0) {
                        delayedCommitStopTimerEvent.signal();
                    }
                    while (!upgradeSem.wait(waitLockTimeout)
                           || !(monitor->nWriters == 1 && monitor->nReaders == 0))
                    {
                        // There are no writers, so some reader was died
                        cs.enter();
                        unsigned currTime = dbSystem::getCurrentTimeMsec();
                        if (currTime - monitor->lastDeadlockRecoveryTime
                            >= waitLockTimeout)
                        {
                            // Ok, let's try to "remove" this reader
                            monitor->lastDeadlockRecoveryTime = currTime;
                            if (--monitor->nReaders == 1) {
                                // Looks like we are recovered
#ifdef AUTO_DETECT_PROCESS_CRASH
                                removeLockOwner(selfId);
#endif
                                monitor->nWriters = 1;
                                monitor->nReaders = 0;
                                monitor->nWaitWriters -= 1;
                                monitor->waitForUpgrade = false;
                                cs.leave();
                                break;
                            }
                        }
                        cs.leave();
                    }
                } else {
#ifdef AUTO_DETECT_PROCESS_CRASH
                    removeLockOwner(selfId);
#endif
                    monitor->nWriters = 1;
                    monitor->nReaders = 0;
                    cs.leave();
                }
            } else {
                if (monitor->nWriters != 0 || monitor->nReaders != 0) {
                    monitor->nWaitWriters += 1;
                    cs.leave();
                    if (commitDelay != 0) {
                        delayedCommitStopTimerEvent.signal();
                    }
                    while (!writeSem.wait(waitLockTimeout)
                           || !(monitor->nWriters == 1 && monitor->nReaders == 0))
                    {
                        cs.enter();
                        unsigned currTime = dbSystem::getCurrentTimeMsec();
                        if (currTime - monitor->lastDeadlockRecoveryTime
                            >= waitLockTimeout)
                        {
                            monitor->lastDeadlockRecoveryTime = currTime;
                            if (monitor->nWriters != 0) {
                                // writer was died
                                checkVersion();
                                recovery();
                                monitor->nWriters = 1;
                                monitor->nWaitWriters -= 1;
                                cs.leave();
                                break;
                            } else {
                                // some reader was died
                                // Ok, let's try to "remove" this reader
                                if (--monitor->nReaders == 0) {
                                    // Looks like we are recovered
                                    monitor->nWriters = 1;
                                    monitor->nWaitWriters -= 1;
                                    cs.leave();
                                    break;
                                }
                            }
                        }
                        cs.leave();
                    }
                } else {
                    monitor->nWriters = 1;
                    cs.leave();
                }
            }
            monitor->ownerPid = ctx->currPid;
#ifdef AUTO_DETECT_PROCESS_CRASH
            monitor->exclusiveLockOwner = selfId;
#endif
            ctx->writeAccess = true;
        } else {
            if (monitor->ownerPid != ctx->currPid) {
                handleError(LockRevoked);
            }
        }
    } else {
        if (!ctx->readAccess && !ctx->writeAccess) {
            cs.enter();
#ifdef AUTO_DETECT_PROCESS_CRASH
            startWatchDogThreads();
#endif
            if (monitor->nWriters + monitor->nWaitWriters != 0) {
                monitor->nWaitReaders += 1;
                cs.leave();
                if (commitDelay != 0) {
                    delayedCommitStopTimerEvent.signal();
                }
                while (!readSem.wait(waitLockTimeout)
                       || !(monitor->nWriters == 0 && monitor->nReaders > 0))
                {
                    cs.enter();
                    unsigned currTime = dbSystem::getCurrentTimeMsec();
                    if (currTime - monitor->lastDeadlockRecoveryTime
                        >= waitLockTimeout)
                    {
                        monitor->lastDeadlockRecoveryTime = currTime;
                        if (monitor->nWriters != 0) {
                            // writer was died
                            checkVersion();
                            recovery();
                            monitor->nWriters = 0;
                        } else {
                            // some potential writer was died
                            monitor->nWaitWriters -= 1;
                        }
                        monitor->nReaders += 1;
                        monitor->nWaitReaders -= 1;
                        cs.leave();
                        break;
                    }
                    cs.leave();
                }
#ifdef AUTO_DETECT_PROCESS_CRASH
                cs.enter();
                addLockOwner();
                cs.leave();
#endif
            } else {
                monitor->nReaders += 1;
#ifdef AUTO_DETECT_PROCESS_CRASH
                addLockOwner();
#endif
                cs.leave();
            }
            ctx->readAccess = true;
        }
    }
    if (lockType != dbCommitLock) {
        if (commitDelay != 0) {
            dbCriticalSection cs(delayedCommitStopTimerMutex);
            monitor->forceCommitCount -= 1;
        }
        if (!checkVersion()) {
            return false;
        }
        cs.enter();
        index[0] = (offs_t*)(baseAddr + header->root[0].index);
        index[1] = (offs_t*)(baseAddr + header->root[1].index);
        int curr = monitor->curr;
        if (accessType != dbConcurrentRead) {
            currIndex = index[1-curr];
            currIndexSize = header->root[1-curr].indexUsed;
            committedIndexSize = header->root[curr].indexUsed;
        } else {
            currIndex = index[curr];
            currIndexSize = header->root[curr].indexUsed;
            committedIndexSize = header->root[curr].indexUsed;
        }
        cs.leave();
    }
    return true;
}

bool dbDatabase::checkVersion()
{
    if (version != monitor->version) {
        sprintf(databaseName+databaseNameLen, ".%d", monitor->version);
        if (version == 0) {
            if (file.open(fileName, databaseName, accessType == dbReadOnly || accessType == dbConcurrentRead, monitor->size, false)
                != dbFile::ok)
            {
                handleError(DatabaseOpenError, "Failed to open database file");
                endTransaction(); // release locks
                return false;
            }
        } else {
            int status = file.setSize(header->size, databaseName, false);
            if (status != dbFile::ok) {
                handleError(FileError, "Failed to reopen database file", status);
                endTransaction(); // release locks
                return false;
            }
        }
        version = monitor->version;
        baseAddr = (byte*)file.getAddr();
        header = (dbHeader*)baseAddr;
        assert(file.getSize() == header->size);
    }
    return true;
}

void dbDatabase::precommit()
{
    //assert(accessType != dbConcurrentUpdate);
    dbDatabaseThreadContext* ctx = threadContext.get();
    if (ctx != NULL && (ctx->writeAccess || ctx->readAccess)) {
        ctx->concurrentId = monitor->concurrentTransId;
        endTransaction(ctx);
    }
}


void dbDatabase::delayedCommit()
{
    dbCriticalSection cs(delayedCommitStartTimerMutex);
    commitThreadSyncEvent.signal();
    while (!stopDelayedCommitThread) {
        delayedCommitStartTimerEvent.wait(delayedCommitStartTimerMutex);
        delayedCommitStartTimerEvent.reset();
        bool deferredCommit = false;
        {
            dbCriticalSection cs2(delayedCommitStopTimerMutex);
            if (!stopDelayedCommitThread && monitor->forceCommitCount == 0) {
                commitTimerStarted = time(NULL);
                deferredCommit = true;
            }
        }
        if (deferredCommit) {
            delayedCommitStopTimerEvent.wait((unsigned)(commitTimeout*1000));
            delayedCommitStopTimerEvent.reset();
        }
        {
            dbCriticalSection cs2(delayedCommitStopTimerMutex);
            dbDatabaseThreadContext* ctx = monitor->delayedCommitContext;
            if (ctx != NULL) {
                commitTimeout = commitDelay;
                monitor->delayedCommitContext = NULL;
                threadContext.set(ctx);
                commit(ctx);
                ctx->commitDelayed = false;
                if (ctx->removeContext) {
                    dbCriticalSection cs(threadContextListMutex);
                    delete ctx;
                }
            }
        }
    }
}

void dbDatabase::waitTransactionAcknowledgement()
{
}

void dbDatabase::commit()
{
    dbDatabaseThreadContext* ctx = threadContext.get();
    if (ctx != NULL && !ctx->commitDelayed) {
        if (ctx->writeAccess) {
            if (monitor->ownerPid != ctx->currPid) {
                handleError(LockRevoked);
            }
        }
        cs.enter();
        bool hasSomethingToCommit = modified && !monitor->commitInProgress
            && (monitor->uncommittedChanges || ctx->writeAccess || ctx->mutatorCSLocked || ctx->concurrentId == monitor->concurrentTransId);
        cs.leave();
        if (hasSomethingToCommit) {
            if (!ctx->writeAccess) {
                beginTransaction(ctx->mutatorCSLocked ? dbCommitLock : dbExclusiveLock);
            }
            if (commitDelay != 0) {
                dbCriticalSection cs(delayedCommitStartTimerMutex);
                monitor->delayedCommitContext = ctx;
                ctx->commitDelayed = true;
                delayedCommitStartTimerEvent.signal();
            } else {
                commit(ctx);
            }
        } else {
            if (ctx->writeAccess || ctx->readAccess || ctx->mutatorCSLocked) {
                endTransaction(ctx);
            }
        }
    }
}

void dbDatabase::commit(dbDatabaseThreadContext* ctx)
{
    //
    // commit transaction
    //
    int curr = header->curr;
    int4 *map = monitor->dirtyPagesMap;
    size_t oldIndexSize = header->root[curr].indexSize;
    size_t newIndexSize = header->root[1-curr].indexSize;
    if (newIndexSize > oldIndexSize) {
        offs_t newIndex = allocate(newIndexSize*sizeof(offs_t));
        header->root[1-curr].shadowIndex = newIndex;
        header->root[1-curr].shadowIndexSize = newIndexSize;
        cloneBitmap(header->root[curr].index, oldIndexSize*sizeof(offs_t));
        deallocate(header->root[curr].index, oldIndexSize*sizeof(offs_t));
    }

    //
    // Enable read access to the database
    //
    cs.enter();
    assert(ctx->writeAccess);
    monitor->commitInProgress = true;
    monitor->sharedLockOwner[0] = monitor->exclusiveLockOwner;
    monitor->exclusiveLockOwner = 0;
    monitor->nWriters -= 1;
    monitor->nReaders += 1;
    monitor->ownerPid.clear();
    if (accessType == dbConcurrentUpdate) {
        // now readers will see updated data
        monitor->curr ^= 1;
    }
    if (monitor->nWaitReaders != 0) {
        monitor->nReaders += monitor->nWaitReaders;
        readSem.signal(monitor->nWaitReaders);
        monitor->nWaitReaders = 0;
    }
    ctx->writeAccess = false;
    ctx->readAccess = true;

    // Copy values of this fields to local variables since them can be changed by read-only transaction in concurrent update mode
    size_t   committedIndexSize = this->committedIndexSize;
    offs_t*  currIndex = this->currIndex;
    size_t   currIndexSize = this->currIndexSize;

    cs.leave();

    size_t   nPages = committedIndexSize / dbHandlesPerPage;
    offs_t*  srcIndex = currIndex;
    offs_t*  dstIndex = index[curr];

    for (size_t i = 0; i < nPages; i++) {
        if (map[i >> 5] & (1 << (i & 31))) {
            file.markAsDirty(header->root[1-curr].index + i*dbPageSize, dbPageSize);
            for (size_t j = 0; j < dbHandlesPerPage; j++) {
                offs_t offs = dstIndex[j];
                if (srcIndex[j] != offs) {
                    if (!(offs & dbFreeHandleMarker)) {
                        size_t marker = offs & dbInternalObjectMarker;
                        if (marker != 0) {
                            deallocate(offs-marker, internalObjectSize[marker]);
                        } else {
                            deallocate(offs, ((dbRecord*)(baseAddr+offs))->size);
                        }
                    }
                }
            }
        }
        dstIndex += dbHandlesPerPage;
        srcIndex += dbHandlesPerPage;
    }
    file.markAsDirty(header->root[1-curr].index + nPages*dbPageSize,
                     (currIndexSize - nPages*dbHandlesPerPage)*sizeof(offs_t));
    offs_t* end = index[curr] + committedIndexSize;
    while (dstIndex < end) {
        offs_t offs = *dstIndex;
        if (*srcIndex != offs) {
            if (!(offs & dbFreeHandleMarker)) {
                size_t marker = offs & dbInternalObjectMarker;
                if (marker != 0) {
                    deallocate(offs-marker, internalObjectSize[marker]);
                } else {
                    deallocate(offs, ((dbRecord*)(baseAddr+offs))->size);
                }
            }
        }
        dstIndex += 1;
        srcIndex += 1;
    }
    file.markAsDirty(0, sizeof(dbHeader));
    file.flush();

    cs.enter();
    while (monitor->backupInProgress) {
        cs.leave();
        backupCompletedEvent.wait();
        cs.enter();
    }
    header->curr = curr ^= 1;
    cs.leave();

    file.markAsDirty(0, sizeof(dbHeader));
#ifdef SYNCHRONOUS_REPLICATION
    waitTransactionAcknowledgement();
#else
    file.flush();
#endif

    file.markAsDirty(0, sizeof(dbHeader));
    header->root[1-curr].indexUsed = currIndexSize;
    header->root[1-curr].freeList  = header->root[curr].freeList;
#ifdef DO_NOT_REUSE_OID_WITHIN_SESSION
    monitor->sessionFreeList[1-curr] = monitor->sessionFreeList[curr];
#endif

    if (newIndexSize != oldIndexSize) {
        header->root[1-curr].index=header->root[curr].shadowIndex;
        header->root[1-curr].indexSize=header->root[curr].shadowIndexSize;
        header->root[1-curr].shadowIndex=header->root[curr].index;
        header->root[1-curr].shadowIndexSize=header->root[curr].indexSize;
        file.markAsDirty(header->root[1-curr].index, currIndexSize*sizeof(offs_t));
        memcpy(baseAddr + header->root[1-curr].index, currIndex,
               currIndexSize*sizeof(offs_t));
        memset(map, 0, 4*((currIndexSize+dbHandlesPerPage*32-1)
                          / (dbHandlesPerPage*32)));
    } else {
        byte* srcIndex = (byte*)currIndex;
        byte* dstIndex = (byte*)index[1-curr];

        for (size_t i = 0; i < nPages; i++) {
            if (map[i >> 5] & (1 << (i & 31))) {
                map[i >> 5] -= (1 << (i & 31));
                memcpy(dstIndex, srcIndex, dbPageSize);
                file.markAsDirty(header->root[1-curr].index + i*dbPageSize, dbPageSize);
            }
            srcIndex += dbPageSize;
            dstIndex += dbPageSize;
        }
        if (currIndexSize > nPages*dbHandlesPerPage) {
            memcpy(dstIndex, srcIndex,
                   sizeof(offs_t)*(currIndexSize-nPages*dbHandlesPerPage));
            file.markAsDirty(header->root[1-curr].index + nPages*dbPageSize,
                             sizeof(offs_t)*(currIndexSize-nPages*dbHandlesPerPage));
            memset(map + (nPages>>5), 0,
                   ((currIndexSize + dbHandlesPerPage*32 - 1)
                    / (dbHandlesPerPage*32) - (nPages>>5))*4);
        }
    }
    cs.enter();
    modified = false;
    monitor->modified = false;
    monitor->uncommittedChanges = false;
    monitor->commitInProgress = false;
    if (accessType != dbConcurrentUpdate) {
        monitor->curr = curr;
    }
    monitor->concurrentTransId += 1;
    cs.leave();

    if (ctx->writeAccess || ctx->readAccess || ctx->mutatorCSLocked) {
        endTransaction(ctx);
    }
}

void dbDatabase::rollback()
{
    dbDatabaseThreadContext* ctx = threadContext.get();

    if (modified
        && (monitor->uncommittedChanges || ctx->writeAccess || ctx->mutatorCSLocked || ctx->concurrentId == monitor->concurrentTransId))
    {
        if (!ctx->writeAccess && !ctx->mutatorCSLocked) {
            beginTransaction(dbExclusiveLock);
        }
        int curr = header->curr;
        byte* dstIndex = baseAddr + header->root[curr].shadowIndex;
        byte* srcIndex = (byte*)index[curr];

        currRBitmapPage = currPBitmapPage = dbBitmapId;
        currRBitmapOffs = currPBitmapOffs = 0;

        size_t nPages =
            (committedIndexSize + dbHandlesPerPage - 1) / dbHandlesPerPage;
        int4 *map = monitor->dirtyPagesMap;
        if (header->root[1-curr].index != header->root[curr].shadowIndex) {
            memcpy(dstIndex, srcIndex,  nPages*dbPageSize);
            file.markAsDirty( header->root[curr].shadowIndex, nPages*dbPageSize);
        } else {
            for (size_t i = 0; i < nPages; i++) {
                if (map[i >> 5] & (1 << (i & 31))) {
                    memcpy(dstIndex, srcIndex, dbPageSize);
                    file.markAsDirty(header->root[1-curr].index + i*dbPageSize, dbPageSize);
                }
                srcIndex += dbPageSize;
                dstIndex += dbPageSize;
            }
        }

        header->root[1-curr].indexSize = header->root[curr].shadowIndexSize;
        header->root[1-curr].indexUsed = header->root[curr].indexUsed;
        header->root[1-curr].freeList  = header->root[curr].freeList;
        header->root[1-curr].index = header->root[curr].shadowIndex;
#ifdef DO_NOT_REUSE_OID_WITHIN_SESSION
        monitor->sessionFreeList[1-curr] = monitor->sessionFreeList[curr];
#endif

        memset(map, 0,
               size_t((currIndexSize+dbHandlesPerPage*32-1) / (dbHandlesPerPage*32))*4);

        file.markAsDirty(0, sizeof(dbHeader));
        modified = false;
        monitor->uncommittedChanges = false;
        monitor->concurrentTransId += 1;
        restoreTablesConsistency();
    }
    if (monitor->users != 0) { // if not abandon
        endTransaction(ctx);
    }
}

void dbDatabase::updateCursors(oid_t oid, bool removed)
{
    dbDatabaseThreadContext* ctx = threadContext.get();
    if (ctx != NULL) {
        for (dbAnyCursor* cursor = (dbAnyCursor*)ctx->cursors.next;
             cursor != &ctx->cursors;
             cursor = (dbAnyCursor*)cursor->next)
        {
            if (cursor->currId == oid) {
                if (removed) {
                    cursor->currId = 0;
                } else if (cursor->record != NULL/* && !cursor->updateInProgress*/) {
                    cursor->fetch();
                }
            }
        }
    }
}


void dbDatabase::endTransaction(dbDatabaseThreadContext* ctx)
{
    if (!ctx->commitDelayed) {
        while (!ctx->cursors.isEmpty()) {
            ((dbAnyCursor*)ctx->cursors.next)->reset();
        }
    }
    if (ctx->writeAccess) {
        cs.enter();
        ctx->isMutator = false;
        monitor->nWriters -= 1;
        monitor->exclusiveLockOwner = 0;
        monitor->ownerPid.clear();
        assert(monitor->nWriters == 0 && !monitor->waitForUpgrade);
        if (monitor->nWaitWriters != 0) {
            monitor->nWaitWriters -= 1;
            monitor->nWriters = 1;
            writeSem.signal();
        } else if (monitor->nWaitReaders != 0) {
            monitor->nReaders = monitor->nWaitReaders;
            monitor->nWaitReaders = 0;
            readSem.signal(monitor->nReaders);
        }
        cs.leave();
    } else if (ctx->readAccess) {
        cs.enter();
#ifdef AUTO_DETECT_PROCESS_CRASH
        removeLockOwner(selfId);
#endif
        monitor->nReaders -= 1;
        if (monitor->nReaders == 1 && monitor->waitForUpgrade) {
            assert(monitor->nWriters == 0);
#ifdef AUTO_DETECT_PROCESS_CRASH
            removeLockOwner(monitor->upgradeId);
            monitor->upgradeId = 0;
#endif
            monitor->waitForUpgrade = false;
            monitor->nWaitWriters -= 1;
            monitor->nWriters = 1;
            monitor->nReaders = 0;
            upgradeSem.signal();
        } else if (monitor->nReaders == 0) {
            if (monitor->nWaitWriters != 0) {
                assert(monitor->nWriters == 0 && !monitor->waitForUpgrade);
                monitor->nWaitWriters -= 1;
                monitor->nWriters = 1;
                writeSem.signal();
            }
        }
        cs.leave();
    }
    ctx->writeAccess = false;
    ctx->readAccess = false;
    if (ctx->mutatorCSLocked) {
        ctx->mutatorCSLocked = false;
        mutatorCS.leave();
    }
}


void dbDatabase::linkTable(dbTableDescriptor* table, oid_t tableId)
{
    assert(((void)"Table can be used only in one database",
            table->tableId == 0));
    table->db = this;
    table->nextDbTable = tables;
    table->tableId = tableId;
    tables = table;
}

void dbDatabase::unlinkTable(dbTableDescriptor* table)
{
    dbTableDescriptor** tpp;
    for (tpp = &tables; *tpp != table; tpp = &(*tpp)->nextDbTable);
    *tpp = table->nextDbTable;
    table->tableId = 0;
    if (!table->fixedDatabase) {
        table->db = NULL;
    }
}

dbTableDescriptor* dbDatabase::findTableByName(char const* name)
{
    char* sym = (char*)name;
    dbSymbolTable::add(sym, tkn_ident);
    return findTable(sym);
}


dbTableDescriptor* dbDatabase::findTable(char const* name)
{
    for (dbTableDescriptor* desc=tables; desc != NULL; desc=desc->nextDbTable)
    {
        if (desc->name == name) {
            return desc;
        }
    }
    return NULL;
}

void dbDatabase::insertInverseReference(dbFieldDescriptor* fd, oid_t inverseId,
                                        oid_t targetId)
{
    byte buf[1024];
    if (inverseId == targetId) {
        return;
    }
    fd = fd->inverseRef;
    if (fd->type == dbField::tpArray) {
        dbTableDescriptor* desc = fd->defTable;
        dbRecord* rec = getRow(targetId);
        dbVarying* arr = (dbVarying*)((byte*)rec + fd->dbsOffs);
        size_t arrSize = arr->size;
        size_t arrOffs = arr->offs;
        offs_t oldOffs = currIndex[targetId];
        size_t newSize = desc->fixedSize;
        size_t lastOffs = desc->columns->sizeWithoutOneField(fd, (byte*)rec, newSize);
        size_t newArrOffs = DOALIGN(newSize, sizeof(oid_t));
        size_t oldSize = rec->size;
        newSize = newArrOffs + (arrSize + 1)*sizeof(oid_t);
        if (newSize > oldSize) {
            newSize = newArrOffs + (arrSize+1)*sizeof(oid_t)*2;
        } else {
            newSize = oldSize;
        }

        byte* dst = (byte*)putRow(targetId, newSize);
        byte* src = baseAddr + oldOffs;
        byte* tmp = NULL;

        if (dst == src) {
            if (arrOffs == newArrOffs && newArrOffs > lastOffs) {
                *((oid_t*)((byte*)rec + newArrOffs) + arrSize) = inverseId;
                arr->size += 1;
                updateCursors(targetId);
                return;
            }
            if (oldSize > sizeof(buf)) {
                src = tmp = dbMalloc(oldSize);
            } else {
                src = buf;
            }
            memcpy(src, rec, oldSize);
        }
        desc->columns->copyRecordExceptOneField(fd, dst, src, desc->fixedSize);

        arr = (dbVarying*)(dst + fd->dbsOffs);
        arr->size = arrSize + 1;
        arr->offs = newArrOffs;
        memcpy(dst + newArrOffs, src + arrOffs, arrSize*sizeof(oid_t));
        *((oid_t*)(dst + newArrOffs) + arrSize) = inverseId;
        if (tmp != NULL) {
            dbFree(tmp);
        }
    } else {
        if (fd->indexType & INDEXED) {
            dbTtree::remove(this, fd->tTree, targetId, fd->type, fd->dbsSize, fd->comparator, fd->dbsOffs);
        }
        oid_t oldRef = *(oid_t*)((byte*)getRow(targetId) + fd->dbsOffs);
        if (oldRef != 0) {
            removeInverseReference(fd, targetId, oldRef);
        }
         *(oid_t*)((byte*)putRow(targetId) + fd->dbsOffs) = inverseId;

        if (fd->indexType & INDEXED) {
            dbTtree::insert(this, fd->tTree, targetId, fd->type, fd->dbsSize, fd->comparator, fd->dbsOffs);
        }
    }
    updateCursors(targetId);
}


void dbDatabase::removeInverseReferences(dbTableDescriptor* desc, oid_t oid)
{
    dbVisitedObject* chain = visitedChain;
    dbVisitedObject  vo(oid, chain);
    visitedChain = &vo;

    dbFieldDescriptor* fd;
    for (fd = desc->inverseFields; fd != NULL; fd = fd->nextInverseField) {
        if (fd->type == dbField::tpArray) {
            dbVarying* arr = (dbVarying*)((byte*)getRow(oid) + fd->dbsOffs);
            int n = arr->size;
            int offs = arr->offs + n*sizeof(oid_t);
            while (--n >= 0) {
                offs -= sizeof(oid_t);
                oid_t ref = *(oid_t*)((byte*)getRow(oid) + offs);
                if (ref != 0) {
                    removeInverseReference(fd, oid, ref);
                }
            }
        } else {
            oid_t ref = *(oid_t*)((byte*)getRow(oid) + fd->dbsOffs);
            if (ref != 0) {
                removeInverseReference(fd, oid, ref);
            }
        }
    }

    visitedChain = chain;
}


void dbDatabase::removeInverseReference(dbFieldDescriptor* fd,
                                        oid_t inverseId,
                                        oid_t targetId)
{
    if (inverseId == targetId || targetId == updatedRecordId ||
        (currIndex[targetId] & dbFreeHandleMarker) != 0)
    {
        return;
    }
    for (dbVisitedObject* vo = visitedChain; vo != NULL; vo = vo->next) {
        if (vo->oid == targetId) {
            return;
        }
    }
    byte* rec = (byte*)putRow(targetId);
    if ((fd->indexType & DB_FIELD_CASCADE_DELETE)
        && ((fd->inverseRef->type != dbField::tpArray) ||
            ((dbVarying*)(rec + fd->inverseRef->dbsOffs))->size <= 1))
    {
        remove(fd->inverseRef->defTable, targetId);
        return;
    }

    fd = fd->inverseRef;
    if (fd->type == dbField::tpArray) {
        dbVarying* arr = (dbVarying*)(rec + fd->dbsOffs);
        oid_t* p = (oid_t*)(rec + arr->offs);
        for (int n = arr->size, i = n; --i >= 0;) {
            if (p[i] == inverseId) {
                while (++i < n) {
                    p[i-1] = p[i];
                }
                arr->size -= 1;
                break;
            }
        }
    } else {
        if (*(oid_t*)(rec + fd->dbsOffs) == inverseId) {
            if (fd->indexType & INDEXED) {
                dbTtree::remove(this, fd->tTree, targetId, fd->type, fd->dbsSize, fd->comparator, fd->dbsOffs);
            }

            *(oid_t*)((byte*)putRow(targetId) + fd->dbsOffs) = 0;

            if (fd->indexType & INDEXED) {
                dbTtree::insert(this, fd->tTree, targetId, fd->type, fd->dbsSize, fd->comparator, fd->dbsOffs);
            }
        }
    }
    updateCursors(targetId);
}

bool dbDatabase::completeDescriptorsInitialization()
{
    dbTableDescriptor* desc;
    bool result = true;
    for (desc = tables; desc != NULL; desc = desc->nextDbTable) {
        dbFieldDescriptor* fd;
        for (fd = desc->firstField; fd != NULL; fd = fd->nextField) {
            if (fd->refTableName != NULL) {
                fd->refTable = findTable(fd->refTableName);
                if (fd->refTable == NULL) {
                    result = false;
                }
            }
        }
        if (result) {
            desc->checkRelationship();
        }
    }
    return result;
}


bool dbDatabase::backup(char const* file, bool compactify)
{
    dbFile f;
    if (f.create(file, !compactify) != dbFile::ok) {
        return false;
    }
    bool result = backup(&f, compactify);
    f.close();
    return result;
}


bool dbDatabase::backup(dbFile* f, bool compactify)
{
    bool result = true;
    backupCompletedEvent.reset();
    cs.enter();
    if (monitor->backupInProgress) {
        cs.leave();
        return false; // no two concurrent backups are possible
    }
    monitor->backupInProgress = true;
    cs.leave();
    if (compactify) {
        int     curr = header->curr;
        size_t  nObjects = header->root[1-curr].indexUsed;
        size_t  i;
        size_t  nIndexPages = (header->root[1-curr].indexSize + dbHandlesPerPage - 1) / dbHandlesPerPage;
        offs_t* newIndex = new offs_t[nIndexPages*dbHandlesPerPage];

        memset(newIndex, 0, nIndexPages*dbPageSize);
        offs_t used = (nIndexPages*2 + 1)*dbPageSize;
        offs_t start = used;
        used += DOALIGN(getRow(dbMetaTableId)->size, dbAllocationQuantum);

        for (i = dbFirstUserId; i < nObjects; i++) {
            offs_t offs = currIndex[i];
            if (!(offs & dbFreeHandleMarker)) {
                int marker = offs & dbInternalObjectMarker;
                newIndex[i] = used | marker;
                used += DOALIGN(marker ? internalObjectSize[marker] : getRow(i)->size,
                                dbAllocationQuantum);
            } else {
                newIndex[i] = offs;
            }
        }
        size_t bitmapPages =
            (used + dbPageSize*(dbAllocationQuantum*8-1) - 1)
            / (dbPageSize*(dbAllocationQuantum*8-1));
        size_t bitmapSize = bitmapPages*dbPageSize;
        for (i = dbFirstUserId; i < nObjects; i++) {
            if (!(newIndex[i] & dbFreeHandleMarker)) {
                newIndex[i] += bitmapSize;
            }
        }
        used += bitmapSize;

        for (i = 0; i < bitmapPages; i++) {
            newIndex[dbBitmapId+i] = start | dbPageObjectMarker;
            start += dbPageSize;
        }
        while (i < dbBitmapPages) {
            newIndex[dbBitmapId+i] = dbFreeHandleMarker;
            i += 1;
        }
        newIndex[dbMetaTableId] = start;
        newIndex[dbInvalidId] =  dbFreeHandleMarker;

        byte page[dbPageSize];
        memset(page, 0, sizeof page);
        dbHeader* newHeader = (dbHeader*)page;
        offs_t newFileSize = DOALIGN(used, dbPageSize);
        newHeader->size = newFileSize;
        newHeader->curr = 0;
        newHeader->dirty = 0;
        newHeader->initialized = true;
        newHeader->majorVersion = header->majorVersion;
        newHeader->minorVersion = header->minorVersion;
        newHeader->mode = header->mode;
        newHeader->root[0].index = newHeader->root[1].shadowIndex = dbPageSize;
        newHeader->root[0].shadowIndex = newHeader->root[1].index = dbPageSize + nIndexPages*dbPageSize;
        newHeader->root[0].shadowIndexSize = newHeader->root[0].indexSize =
            newHeader->root[1].shadowIndexSize = newHeader->root[1].indexSize = nIndexPages*dbHandlesPerPage;
        newHeader->root[0].indexUsed = newHeader->root[1].indexUsed = nObjects;
        newHeader->root[0].freeList = newHeader->root[1].freeList = header->root[1-curr].freeList;
        result &= f->write(page, dbPageSize);

        result &= f->write(newIndex, nIndexPages*dbPageSize);
        result &= f->write(newIndex, nIndexPages*dbPageSize);

        int bits = (int)((used >> dbAllocationQuantumBits) - (offs_t)(bitmapPages-1)*dbPageSize*8);
        memset(page, 0xFF, sizeof page);
        while (--bitmapPages != 0) {
            result &= f->write(page, dbPageSize);
        }
        if (size_t(bits >> 3) < dbPageSize) {
            memset(page + (bits >> 3) + 1, 0, dbPageSize - (bits >> 3) - 1);
            page[bits >> 3] = (1 << (bits & 7)) - 1;
        }
        result &= f->write(page, dbPageSize);

        result &= f->write(baseAddr + currIndex[dbMetaTableId], DOALIGN(getRow(dbMetaTableId)->size, dbAllocationQuantum));

        for (i = dbFirstUserId; i < nObjects; i++) {
            offs_t offs = newIndex[i];
            if (!(offs & dbFreeHandleMarker)) {
                int marker = offs & dbInternalObjectMarker;
                size_t size = DOALIGN(marker ? internalObjectSize[marker] : getRow(i)->size,
                                      dbAllocationQuantum);
                result &= f->write(baseAddr + currIndex[i] - marker, size);
            }
        }
        if (used != newFileSize) {
            assert(newFileSize - used < dbPageSize);
            size_t align = (size_t)(newFileSize - used);
            memset(page, 0, align);
            result &= f->write(page, align);
        }
        delete[] newIndex;
    } else { // end if compactify
        const size_t segmentSize = 64*1024;
        byte* p = baseAddr;
        size_t size = (size_t)header->size;
        result = true;
        while (size > segmentSize && result) {
            result = f->write(p, segmentSize);
            p += segmentSize;
            size -= segmentSize;
        }
        if (result) {
            result = f->write(p, size);
        }
    }
    monitor->backupInProgress = false;
    backupCompletedEvent.signal();
    return result;
}

dbDatabase::dbDatabase(dbAccessType type, size_t dbInitSize,
                       size_t dbExtensionQuantum, size_t dbInitIndexSize,
                       int nThreads
#ifdef NO_PTHREADS
                       , dbThreadMode threadMode
#endif
#ifdef REPLICATION_SUPPORT
                       , dbReplicationMode replicationMode
#endif
) : accessType(type),
    initSize(dbInitSize),
    extensionQuantum(dbExtensionQuantum),
    initIndexSize(dbInitIndexSize),
    freeSpaceReuseThreshold(dbExtensionQuantum),
    parallelScanThreshold(dbDefaultParallelScanThreshold)
{
#ifdef AUTO_DETECT_PROCESS_CRASH
    assert(type != dbConcurrentUpdate);
#endif
#if dbDatabaseOffsetBits > 32 ||  dbDatabaseOidBits > 32
    assert(sizeof(size_t) == 8);
#endif
    bitmapPageAvailableSpace = new int[dbBitmapId + dbBitmapPages];
    setConcurrency(nThreads);
    tables = NULL;
    commitDelay = 0;
    commitTimeout = 0;
    commitTimerStarted = 0;
    backupFileName = NULL;
    backupPeriod = 0;
    databaseName = NULL;
    fileName = NULL;
    opened = false;
    fileSizeLimit = 0;
    errorHandler = NULL;
    confirmDeleteColumns = false;
    schemeVersion = 0;
    visitedChain = NULL;
    header = NULL;
}

dbDatabase::~dbDatabase()
{
    delete[] bitmapPageAvailableSpace;
    delete[] databaseName;
    delete[] fileName;
}



dbDatabase::dbErrorHandler dbDatabase::setErrorHandler(dbDatabase::dbErrorHandler newHandler, void* context)
{
    dbErrorHandler prevHandler = errorHandler;
    errorHandler = newHandler;
    errorHandlerContext = context;
    return prevHandler;
}

dbTableDescriptor* dbDatabase::loadMetaTable()
{
    dbTable* table = (dbTable*)getRow(dbMetaTableId);
    dbTableDescriptor* metatable = new dbTableDescriptor(table);
    linkTable(metatable, dbMetaTableId);
    oid_t tableId = table->firstRow;
    while (tableId != 0) {
        table = (dbTable*)getRow(tableId);
        dbTableDescriptor* desc;
        for (desc = tables; desc != NULL && desc->tableId != tableId; desc = desc->nextDbTable);
        if (desc == NULL) {
            desc = new dbTableDescriptor(table);
            linkTable(desc, tableId);
            desc->setFlags();
        }
        tableId = table->next;
    }
    if (!completeDescriptorsInitialization()) {
        handleError(DatabaseOpenError, "Reference to undefined table");
    }
    return metatable;
}

#ifdef REPLICATION_SUPPORT

#define MAX_LOST_TRANSACTIONS 100

char const* statusText[] = {
    "OFFLINE",
    "ONLINE",
    "ACTIVE",
    "STANDBY",
    "RECOVERED"
};

char const* requestText[] = {
    "CONNECT",
    "RECOVERY",
    "GET_STATUS",
    "STATUS",
    "UPDATE_PAGE",
    "NEW_ACTIVE_NODE",
    "CHANGE_ACTIVE_NODE",
    "CLOSE",
    "READY",
    "COMMITTED"
};

bool dbReplicatedDatabase::isReplicated()
{
    return true;
}

dbReplicatedDatabase::dbReplicatedDatabase(dbAccessType type,
                                           size_t dbInitSize,
                                           size_t dbExtensionQuantum,
                                           size_t dbInitIndexSize,
                                           int nThreads)
: dbDatabase(type, dbInitSize, dbExtensionQuantum, dbInitIndexSize, nThreads)
{
    pollInterval = dbDefaultPollInterval;
    waitReadyTimeout = dbWaitReadyTimeout;
    waitStatusTimeout = dbWaitStatusTimeout;
    recoveryConnectionAttempts = dbRecoveryConnectionAttempts;
    startupConnectionAttempts = dbStartupConnectionAttempts;
    replicationWriteTimeout = dbReplicationWriteTimeout;
    maxAsyncRecoveryIterations = dbMaxAsyncRecoveryIterations;
}

bool dbReplicatedDatabase::open(OpenParameters& params)
{
    accessType = params.accessType;
    extensionQuantum = params.extensionQuantum;
    initIndexSize = params.initIndexSize;
    initSize = params.initSize;
    freeSpaceReuseThreshold = params.freeSpaceReuseThreshold;
    setConcurrency(params.nThreads);
    pollInterval = params.pollInterval;
    waitReadyTimeout = params.waitReadyTimeout;
    waitStatusTimeout = params.waitStatusTimeout;
    recoveryConnectionAttempts = params.recoveryConnectionAttempts;
    startupConnectionAttempts = params.startupConnectionAttempts;
    replicationWriteTimeout = params.replicationWriteTimeout;
    maxAsyncRecoveryIterations = params.maxAsyncRecoveryIterations;
    return open(params.databaseName, params.databaseFilePath, params.nodeId, params.nodeAddresses, params.nNodes);
}

bool dbReplicatedDatabase::open(char const* dbName, char const* fiName,
                                int id, char* servers[], int nServers)
{
    int i;
    char buf [64];
    ReplicationRequest rr;

    assert(accessType != dbReadOnly);

    this->id = id;
    this->nServers = nServers;
    con = new dbConnection[nServers];
    serverURL = servers;
    delete[] databaseName;
    delete[] fileName;
    commitDelay = 0;
    commitTimeout = 0;
    commitTimerStarted = 0;
    waitLockTimeout = INFINITE;
    delayedCommitEventsOpened = false;
    backupFileName = NULL;
    backupPeriod = 0;
    opened = false;
    header = NULL;
    stopDelayedCommitThread = false;
    databaseNameLen = strlen(dbName);
    char* name = new char[databaseNameLen+16];
    sprintf(name, "%s.in", dbName);
    databaseName = name;
    if (fiName == NULL) {
        fileName = new char[databaseNameLen + 5];
        sprintf(fileName, "%s.fdb", dbName);
    } else {
        fileName = new char[strlen(fiName)+1];
        strcpy(fileName, fiName);
    }

    dbInitializationMutex::initializationStatus status = initMutex.initialize(name);
    if (status == dbInitializationMutex::InitializationError) {
        handleError(DatabaseOpenError, "Failed to start database initialization");
        return false;
    }
    if (status != dbInitializationMutex::NotYetInitialized) {
        handleError(DatabaseOpenError, "Database is already started");
        return false;
    }
    sprintf(name, "%s.dm", dbName);
    int shmRes = shm.open(name) ;
    if (shmRes < 0) {
        handleError(DatabaseOpenError, "Failed to open database monitor");
        cleanup(status, 0);
        return false;
    }
    // If the shared memory segment was created, it needs to be initialized
    if (shmRes == 1)
    {
        status = dbInitializationMutex::NotYetInitialized ;
    }
#if defined(USE_POSIX_MMAP) && USE_POSIX_MMAP
    if (status == dbInitializationMutex::AlreadyInitialized)
    {
       // Still Holding initMutex here in POSIX case (not so for SysV)
       // Safe to clear initMutex now
       initMutex.done() ;
    }
#endif
    monitor = shm.get();
    sprintf(name, "%s.ws", dbName);
    if (!writeSem.open(name)) {
        handleError(DatabaseOpenError,
                    "Failed to initialize database writers semaphore");
        cleanup(status, 1);
        return false;
    }
    sprintf(name, "%s.rs", dbName);
    if (!readSem.open(name)) {
        handleError(DatabaseOpenError,
                    "Failed to initialize database readers semaphore");
        cleanup(status, 2);
        return false;
    }
    sprintf(name, "%s.us", dbName);
    if (!upgradeSem.open(name)) {
        handleError(DatabaseOpenError,
                    "Failed to initialize database upgrade semaphore");
        cleanup(status, 3);
        return false;
    }
    sprintf(name, "%s.bce", dbName);
    if (!backupCompletedEvent.open(name)) {
        handleError(DatabaseOpenError,
                    "Failed to initialize database backup completed event");
        cleanup(status, 4);
        return false;
    }
    backupInitEvent.open();
    backupFileName = NULL;

    fixedSizeAllocator.reset();
    allocatedSize = 0;
    size_t indexSize = initIndexSize < dbFirstUserId
        ? size_t(dbFirstUserId) : initIndexSize;
    indexSize = DOALIGN(indexSize, dbHandlesPerPage);

    size_t fileSize = initSize ? initSize : dbDefaultInitDatabaseSize;
    fileSize = DOALIGN(fileSize, dbBitmapSegmentSize);

    if (fileSize < indexSize*sizeof(offs_t)*4) {
        fileSize = indexSize*sizeof(offs_t)*4;
    }

    for (i = dbBitmapId + dbBitmapPages; --i >= 0;) {
        bitmapPageAvailableSpace[i] = INT_MAX;
    }
    currRBitmapPage = currPBitmapPage = dbBitmapId;
    currRBitmapOffs = currPBitmapOffs = 0;
    reservedChain = NULL;
    tables = NULL;
    modified = false;
    selfId = 0;
    maxClientId = 0;
    attach();

    sprintf(name, "%s.cs", dbName);
    if (!cs.create(name, &monitor->sem)) {
        handleError(DatabaseOpenError, "Failed to initialize database monitor");
        cleanup(status, 5);
        return false;
    }
    if (accessType == dbConcurrentUpdate || accessType == dbConcurrentRead) {
        sprintf(name, "%s.mcs", dbName);
        if (!mutatorCS.create(name, &monitor->mutatorSem)) {
            cleanup(status, 6);
            handleError(DatabaseOpenError,
                        "Failed to initialize database monitor");
            return false;
        }
    }
    readSem.reset();
    writeSem.reset();
    upgradeSem.reset();
    monitor->nReaders = 0;
    monitor->nWriters = 0;
    monitor->nWaitReaders = 0;
    monitor->nWaitWriters = 0;
    monitor->waitForUpgrade = false;
    monitor->version = version = 1;
    monitor->users = 0;
    monitor->backupInProgress = 0;
    monitor->forceCommitCount = 0;
    monitor->lastDeadlockRecoveryTime = 0;
    monitor->delayedCommitContext = NULL;
    monitor->concurrentTransId = 1;
    monitor->commitInProgress = false;
    monitor->uncommittedChanges = false;
    monitor->clientId = 0;
    monitor->upgradeId = 0;
    monitor->modified = false;
    monitor->exclusiveLockOwner = 0;
    memset(monitor->dirtyPagesMap, 0, dbDirtyPageBitmapSize);
    memset(monitor->sharedLockOwner, 0, sizeof(monitor->sharedLockOwner));
#ifdef DO_NOT_REUSE_OID_WITHIN_SESSION
    monitor->sessionFreeList[0].head = monitor->sessionFreeList[0].tail = 0;
    monitor->sessionFreeList[1].head = monitor->sessionFreeList[1].tail = 0;
#endif

    sprintf(databaseName, "%s.%d", dbName, version);
    if (file.open(fileName, databaseName, false, fileSize, true) != dbFile::ok)
    {
        handleError(DatabaseOpenError, "Failed to create database file");
        cleanup(status, 8);
        return false;
    }
    baseAddr = (byte*)file.getAddr();
    monitor->size = fileSize = file.getSize();
    header = (dbHeader*)baseAddr;

    if ((unsigned)header->curr > 1) {
        handleError(DatabaseOpenError, "Database file was corrupted: "
                    "invalid root index");
        cleanup(status, 9);
        return false;
    }
    acceptSock = socket_t::create_global(servers[id]);
    if (!acceptSock->is_ok()) {
        acceptSock->get_error_text(buf, sizeof buf);
        dbTrace("<<<FATAL>>> Failed to create accept socket: %s\n", buf);
        cleanup(status, 9);
        delete acceptSock;
        return false;
    }
    FD_ZERO(&inputSD);
    int acceptSockHnd = acceptSock->get_handle();
    FD_SET(acceptSockHnd, &inputSD);
    nInputSD = acceptSockHnd+1;
    startEvent.open(false);
    recoveredEvent.open(false);
    int connectionAttempts = startupConnectionAttempts;
    bool recoveryNeeded = false;

    if (!header->initialized) {
        monitor->curr = header->curr = 0;
        header->size = fileSize;
        size_t used = dbPageSize;
        header->root[0].index = used;
        header->root[0].indexSize = indexSize;
        header->root[0].indexUsed = dbFirstUserId;
        header->root[0].freeList = 0;
        used += indexSize*sizeof(offs_t);
        header->root[1].index = used;
        header->root[1].indexSize = indexSize;
        header->root[1].indexUsed = dbFirstUserId;
        header->root[1].freeList = 0;
        used += indexSize*sizeof(offs_t);

        header->root[0].shadowIndex = header->root[1].index;
        header->root[1].shadowIndex = header->root[0].index;
        header->root[0].shadowIndexSize = indexSize;
        header->root[1].shadowIndexSize = indexSize;

        header->majorVersion= FASTDB_MAJOR_VERSION;
        header->minorVersion = FASTDB_MINOR_VERSION;
        header->mode = dbHeader::getCurrentMode();

        index[0] = (offs_t*)(baseAddr + header->root[0].index);
        index[1] = (offs_t*)(baseAddr + header->root[1].index);
        index[0][dbInvalidId] = dbFreeHandleMarker;

        size_t bitmapPages =
            (used + dbPageSize*(dbAllocationQuantum*8-1) - 1)
            / (dbPageSize*(dbAllocationQuantum*8-1));
        memset(baseAddr+used, 0xFF, (used + bitmapPages*dbPageSize)
               / (dbAllocationQuantum*8));
        size_t i;
        for (i = 0; i < bitmapPages; i++) {
            index[0][dbBitmapId + i] = used + dbPageObjectMarker;
            used += dbPageSize;
        }
        while (i < dbBitmapPages) {
            index[0][dbBitmapId+i] = dbFreeHandleMarker;
            i += 1;
        }
        currIndex = index[0];
        currIndexSize = dbFirstUserId;
        committedIndexSize = 0;
        initializeMetaTable();
        header->dirty = true;
        if (accessType == dbConcurrentRead) {
            modified = false;
        }
        memcpy(index[1], index[0], currIndexSize*sizeof(offs_t));
        file.markAsDirty(0, used);
        file.flush(true);
        header->initialized = true;
        file.markAsDirty(0, sizeof(dbHeader));
        con[id].status = (accessType == dbConcurrentRead) ? ST_RECOVERED : ST_ONLINE;
    } else {
        if (!header->isCompatible()) {
            handleError(DatabaseOpenError, "Incompatible database mode");
            cleanup(status, 9);
            delete acceptSock;
            return false;
        }
        monitor->curr = header->curr;
        if (header->dirty) {
            recoveryNeeded = true;
            dbTrace("Replicated node %d was not normally closed\n", id);
            con[id].status = ST_RECOVERED;
            if (accessType != dbConcurrentRead) {
                connectionAttempts = recoveryConnectionAttempts;
            }
        } else {
            con[id].status = ST_ONLINE;
        }
    }
    cs.enter();
    monitor->users += 1;
    selfId = ++monitor->clientId;
#ifdef AUTO_DETECT_PROCESS_CRASH
    sprintf(databaseName + databaseNameLen, ".pid.%d", selfId);
    selfWatchDog.create(databaseName);
    watchDogMutex = new dbMutex();
#endif
    opened = true;
    cs.leave();

    if (accessType == dbConcurrentUpdate) {
        initMutex.done();
    }
    readerThread.create(startReader, this);
  pollNodes:
    bool startup = true;
    bool standalone;
    int nOnlineNodes = 0;
    int minOnlineNodes = nServers;
    if (accessType == dbConcurrentRead && nServers > 1) {
        minOnlineNodes = 2;
    }
    activeNodeId = -1;

    do {
        standalone = true;
        if (nOnlineNodes == minOnlineNodes) {
            dbThread::sleep(1);
        }
        nOnlineNodes = 1;
        for (i = 0; i < nServers && nOnlineNodes < minOnlineNodes; i++) {
            if (i != id) {
                socket_t* s = con[i].reqSock;
                if (s == NULL) {
                    TRACE_IMSG(("Try to connect to node %d address '%s'\n", i, servers[i]));
                    s = socket_t::connect(servers[i],
                                          socket_t::sock_global_domain,
                                          connectionAttempts);
                    if (!s->is_ok()) {
                        s->get_error_text(buf, sizeof buf);
                        dbTrace("Failed to establish connection with node %d: %s\n", i, buf);
                        delete s;
                        continue;
                    }
                    TRACE_IMSG(("Establish connection with node %d address '%s'\n", i, servers[i]));
                }
                standalone = false;
                rr.op = ReplicationRequest::RR_GET_STATUS;
                rr.nodeId = id;
                bool success = false;
                if (con[i].reqSock == NULL) {
                    TRACE_IMSG(("Send GET_STATUS request to node %d and wait for response\n", i));
                    if (!s->write(&rr, sizeof rr) || !s->read(&rr, sizeof rr)) {
                        s->get_error_text(buf, sizeof buf);
                        dbTrace("Failed to get status from node %d: %s\n", i, buf);
                        delete s;
                    } else {
                        TRACE_IMSG(("Node %d returns status %s\n", i, statusText[rr.status]));
                        addConnection(i, s);
                        con[i].status = rr.status;
                        con[i].updateCounter = rr.size;
                        success = true;
                    }
                } else {
                    con[i].statusEvent.reset();
                    con[i].waitStatusEventFlag += 1;
                    TRACE_IMSG(("Send GET_STATUS request to node %d\n", i));
                    if (writeReq(i, rr)) {
                        dbCriticalSection cs(startCS);
                        lockConnection(i);
                        if (!con[i].statusEvent.wait(startCS, waitStatusTimeout)) {
                            dbTrace("Failed to get status from node %d\n", i);
                            deleteConnection(i);
                        } else {
                            TRACE_IMSG(("Received response from node %d with status %s\n", i, statusText[con[i].status]));
                            success = true;
                        }
                        unlockConnection(i);
                    }
                    con[i].waitStatusEventFlag -= 1;
                }
                if (success) {
                    nOnlineNodes += 1;
                    TRACE_IMSG(("Status of node %d is %s\n", i, statusText[con[i].status]));
                    if (con[i].status == ST_ACTIVE) {
                        startup = false;
                        activeNodeId = i;
                    } else if (con[i].status == ST_STANDBY) {
                        startup = false;
                    } else if (con[i].status == ST_ONLINE
                        && con[i].updateCounter > file.updateCounter)
                    {
                        TRACE_IMSG(("Change status of current node to RECOVERED because its updateCounter=%d and update counter of active node is %d\n", file.updateCounter, con[i].updateCounter));
                        con[id].status = ST_RECOVERED;
                    }
                }
            }
        }
    } while (!standalone && con[id].status == ST_RECOVERED && activeNodeId < 0 && (id != 0 || nOnlineNodes < minOnlineNodes));

    if (!startup) {
        //
        // The node was activated after the active node start the user application
        // So the node's data is out of date. Mark it as recovered.
        //
        TRACE_IMSG(("Change status of node connected after application startup to RECOVERED\n"));
        con[id].status = ST_RECOVERED;
    }
    file.configure(this);
    TRACE_IMSG(("My status is %s\n", statusText[con[id].status]));

    if (con[id].status == ST_RECOVERED && activeNodeId < 0) {
        if (recoveryNeeded) {
            dbTrace("Database was not normally closed: start recovery\n");
            recovery();
            recoveryNeeded = false;
        }
        con[id].status = ST_ONLINE;
    }

    if (con[id].status == ST_ONLINE) {
        for (activeNodeId = 0;
             activeNodeId < id && con[activeNodeId].status != ST_ONLINE;
             activeNodeId++);
        if (activeNodeId == id && accessType != dbConcurrentRead) {
            dbTrace("Node %d becomes active at startup\n", id);
            //
            // Nobody else pretends to be active, so I will be...
            //
            for (i = 0; i < nServers; i++) {
                lockConnection(i);
                if (i != id && con[i].status == ST_ONLINE) {
                    dbCriticalSection cs(startCS);
                    TRACE_IMSG(("Waiting ready event from node %d\n", i));
                    if (!con[i].readyEvent.wait(startCS, waitReadyTimeout)) {
                        dbTrace("Didn't receive ready event from node %d\n", i);
                        deleteConnection(i);
                        unlockConnection(i);
                        goto pollNodes;
                    }
                    TRACE_IMSG(("Received ready event from node %d\n", i));
                }
                unlockConnection(i);
            }
            con[id].status = ST_ACTIVE;
            for (i = 0; i < nServers; i++) {
                if (con[i].status == ST_ONLINE) {
                    con[i].status = ST_STANDBY;
                    rr.op = ReplicationRequest::RR_STATUS;
                    rr.nodeId = i;
                    rr.status = ST_STANDBY;
                    TRACE_IMSG(("Send STANDBY status to node %d\n", i));
                    writeReq(i, rr);
                } else if (con[i].status == ST_STANDBY) {
                    rr.op = ReplicationRequest::RR_CHANGE_ACTIVE_NODE;
                    rr.nodeId = id;
                    rr.status = ST_STANDBY;
                    TRACE_IMSG(("Send CHANGE_ACTIVE_NODE message to node %d\n", i));
                    writeReq(i, rr);
                }
            }
        } else if (activeNodeId < id) {
            rr.op = ReplicationRequest::RR_READY;
            rr.nodeId = id;
            TRACE_IMSG(("Send READY status to node %d\n", activeNodeId));
            if (!writeReq(activeNodeId, rr)) {
                TRACE_IMSG(("Failed to send READY request to node %d\n", activeNodeId));
                goto pollNodes;
            }
        }
    } else if (activeNodeId >= 0) {
        TRACE_IMSG(("Send RECOVERY request to node %d\n", activeNodeId));
        rr.op = ReplicationRequest::RR_RECOVERY;
        rr.size = file.getUpdateCountTableSize();
        rr.nodeId = id;
        if (!writeReq(activeNodeId, rr, file.diskUpdateCount, rr.size*sizeof(int))) {
            TRACE_IMSG(("Failed to send RECOVERY request to node %d\n", activeNodeId));
            goto pollNodes;
        }
    }
    TRACE_IMSG(("My new status is %s\n", statusText[con[id].status]));
    if (con[id].status != ST_ACTIVE) {
        dbCriticalSection cs(startCS);
        if (accessType == dbConcurrentRead) {
            if (con[id].status == ST_RECOVERED) {
                recoveredEvent.wait(startCS);
                dbTrace("Recovery of node %d is completed\n", id);
            }
            monitor->curr = header->curr;
        } else {
            if (!standalone) {
                startEvent.wait(startCS);
            } else {
                con[id].status = ST_ACTIVE;
            }
            baseAddr = (byte*)file.getAddr();
            header = (dbHeader*)baseAddr;
            if (opened) {
                int curr = header->curr;
                monitor->curr = curr;
                offs_t* shadowIndex = (offs_t*)(baseAddr + header->root[curr].index);
                offs_t* currIndex = (offs_t*)(baseAddr + header->root[1-curr].index);
                for (size_t i = 0, size = header->root[curr].indexUsed; i < size; i++) {
                    if (currIndex[i] != shadowIndex[i]) {
                        currIndex[i] = shadowIndex[i];
                        file.markAsDirty(header->root[1-curr].index + i*sizeof(offs_t), sizeof(offs_t));
                    }
                }
                this->currIndex = currIndex;
                header->size = file.getSize();
                header->root[1-curr].index = header->root[curr].shadowIndex;
                header->root[1-curr].indexSize = header->root[curr].shadowIndexSize;
                header->root[1-curr].shadowIndex = header->root[curr].index;
                header->root[1-curr].shadowIndexSize = header->root[curr].indexSize;
                header->root[1-curr].indexUsed = header->root[curr].indexUsed;
                header->root[1-curr].freeList  = header->root[curr].freeList;
#ifdef DO_NOT_REUSE_OID_WITHIN_SESSION
                monitor->sessionFreeList[1-curr] = monitor->sessionFreeList[curr];
#endif

                file.markAsDirty(0, sizeof(dbHeader));
                file.updateCounter += MAX_LOST_TRANSACTIONS;
                restoreTablesConsistency();
                file.flush();
            }
        }
    }
    if (opened) {
        if (loadScheme(true)) {
            if (accessType != dbConcurrentUpdate) {
                initMutex.done();
            }
            return true;
        } else {
            if (accessType == dbConcurrentRead) {
                do {
                    dbTableDescriptor *desc, *next;
                    for (desc = tables; desc != NULL; desc = next) {
                        next = desc->nextDbTable;
                        if (desc->cloneOf != NULL) {
                            delete desc;
                        } else if (!desc->fixedDatabase) {
                            desc->db = NULL;
                        }
                    }
                    tables = NULL;
                    TRACE_IMSG(("Database schema was not yet initialized\n"));
                    endTransaction(); // release locks
                    dbThread::sleep(1);
                } while (!loadScheme(false));
                return true;
            }
            dbTrace("Failed to load database scheme\n");
            opened = false;
        }
    }
    if (accessType != dbConcurrentUpdate) {
        initMutex.done();
    }
#ifdef PROTECT_DATABASE
    if (accessType == dbConcurrentRead) {
        file.protect(0, header->size);
    }
#endif
    readerThread.join();
    delete[] con;
    delete acceptSock;
    dbDatabase::close();
    return false;
}


void thread_proc dbReplicatedDatabase::startReader(void* arg)
{
    ((dbReplicatedDatabase*)arg)->reader();
}

int dbReplicatedDatabase::getNumberOfOnlineNodes()
{
    int n = 0;
    for (int i = 0; i < nServers; i++) {
        if (con[i].status != ST_OFFLINE) {
            n += 1;
        }
    }
    return n;
}

void dbReplicatedDatabase::deleteConnection(int nodeId)
{
    dbTrace("Delete connection with node %d, used counter %d\n", nodeId, con[nodeId].useCount);
    {
        dbCriticalSection cs(sockCS);
        socket_t* reqSock = con[nodeId].reqSock;
        socket_t* respSock = con[nodeId].respSock;
        while (con[nodeId].useCount > 1) {
            con[nodeId].waitUseEventFlag += 1;
            con[nodeId].useCount -= 1;
            con[nodeId].useEvent.reset();
            con[nodeId].useEvent.wait(sockCS);
            con[nodeId].waitUseEventFlag -= 1;
            con[nodeId].useCount += 1;
        }
        assert(con[nodeId].useCount == 1);
        con[nodeId].status = ST_OFFLINE;
        if (con[nodeId].reqSock != NULL) {
            assert(con[nodeId].reqSock == reqSock);
            FD_CLR(reqSock->get_handle(), &inputSD);
            delete reqSock;
            con[nodeId].reqSock = NULL;
        }
        if (con[nodeId].respSock != NULL) {
            assert(con[nodeId].respSock == respSock);
            FD_CLR(respSock->get_handle(), &inputSD);
            delete respSock;
            con[nodeId].respSock = NULL;
        }
    }
    if (nodeId == activeNodeId) {
        changeActiveNode();
    }
}


void dbReplicatedDatabase::changeActiveNode()
{
    ReplicationRequest rr;
    activeNodeId = -1;
    TRACE_IMSG(("Change active node\n"));
    if (con[id].status == ST_STANDBY && accessType != dbConcurrentRead) {
        int i;
        for (i = 0; i < id; i++) {
            if (con[i].status == ST_ONLINE || con[i].status == ST_STANDBY) {
                rr.op = ReplicationRequest::RR_GET_STATUS;
                TRACE_IMSG(("Send GET_STATUS request to node %d to choose new active node\n", i));
                rr.nodeId = id;
                if (writeReq(i, rr)) {
                    return;
                }
            }
        }
        dbTrace("Activate stand-by server %d\n", id);
        for (i = 0; i < nServers; i++) {
            if (con[i].status != ST_OFFLINE && i != id) {
                TRACE_IMSG(("Send NEW_ACTIVE_NODE request to node %d\n", i));
                rr.op = ReplicationRequest::RR_NEW_ACTIVE_NODE;
                rr.nodeId = id;
                if (writeReq(i, rr)) {
                    con[i].status = ST_STANDBY;
                }
            }
        }
        con[id].status = ST_ACTIVE;
        {
            dbCriticalSection cs(startCS);
            startEvent.signal();
        }
    }
}


void dbReplicatedDatabase::reader()
{
    char buf[256];
    ReplicationRequest rr;
    bool statusRequested = false;
    bool closed = false;
    dbDatabaseThreadContext* ctx = NULL;

    if (accessType != dbAllAccess) { // concurrent update
        ctx = new dbDatabaseThreadContext();
        threadContext.set(ctx);
    }

    while (opened) {
        timeval tv;
        tv.tv_sec = pollInterval / 1000;
        tv.tv_usec = pollInterval % 1000 * 1000;
        fd_set ready = inputSD;
        int rc = ::select(nInputSD, &ready, NULL, NULL, &tv);
        if (rc == 0) { // timeout
            if (!closed && con[id].status == ST_STANDBY) {
                if (statusRequested || activeNodeId < 0) {
                    TRACE_MSG(("Initiate change of active node %d\n", activeNodeId));
                    changeActiveNode();
                } else {
                    rr.op = ReplicationRequest::RR_GET_STATUS;
                    rr.nodeId = id;
                    if (!writeResp(activeNodeId, rr)) {
                        dbTrace("Connection with active server lost\n");
                        activeNodeId = -1;
                        changeActiveNode();
                    } else {
                        TRACE_MSG(("Send GET_STATUS request to node %d\n", activeNodeId));
                        statusRequested = true;
                    }
                }
            }
        } else if (rc < 0) {
#ifndef _WIN32
            if (errno != EINTR)
#endif
            {
                dbTrace("Select failed: %d\n", errno);
                tv.tv_sec = 0;
                tv.tv_usec = 0;
                FD_ZERO(&ready);
                for (int i = nInputSD; --i >= 0;) {
                    FD_SET(i, &ready);
                    if (::select(i+1, &ready, NULL, NULL, &tv) < 0) {
                        FD_CLR(i, &inputSD);
                        for (int j = nServers; --j >= 0;) {
                            lockConnection(j);
                            if ((con[j].respSock != NULL && con[j].respSock->get_handle() == i) ||
                                (con[j].reqSock != NULL && con[j].reqSock->get_handle() == i))
                            {
                                deleteConnection(j);
                            }
                            unlockConnection(j);
                        }
                    }
                    FD_CLR(i, &ready);
                }
            }
        } else {
            statusRequested = false;
            if (FD_ISSET(acceptSock->get_handle(), &ready)) {
                socket_t* s = acceptSock->accept();
                if (s != NULL && s->read(&rr, sizeof rr)) {
                    int op = rr.op;
                    int nodeId = rr.nodeId;
                    if (op != ReplicationRequest::RR_GET_STATUS) {
                        dbTrace("Receive unexpected request %d\n", rr.op);
                        delete s;
                    } else if (nodeId >= nServers) {
                        dbTrace("Receive request from node %d while master was configured only for %d nodes\n", rr.op, nServers);
                        delete s;
                    } else {
                        TRACE_IMSG(("Send STATUS response %s for GET_STATUS request from node %d\n",
                                   statusText[con[id].status], nodeId));
                        rr.op = ReplicationRequest::RR_STATUS;
                        rr.nodeId = id;
                        rr.status = con[id].status;
                        rr.size = file.updateCounter;
                        if (!s->write(&rr, sizeof rr)) {
                            s->get_error_text(buf, sizeof buf);
                            dbTrace("Failed to write response: %s\n", buf);
                            delete s;
                        } else {
                            lockConnection(nodeId);
                            if (con[nodeId].respSock != NULL) {
                                deleteConnection(nodeId);
                            }
                            {
                                dbCriticalSection cs(sockCS);
                                con[nodeId].respSock = s;
                                int hnd = s->get_handle();
                                if (hnd >= nInputSD) {
                                    nInputSD = hnd+1;
                                }
                                FD_SET(hnd, &inputSD);
                            }
                            unlockConnection(nodeId);
                        }
                    }
                } else if (s == NULL) {
                    acceptSock->get_error_text(buf, sizeof buf);
                    dbTrace("Accept failed: %s\n", buf);
                } else {
                    s->get_error_text(buf, sizeof buf);
                    dbTrace("Failed to read login request: %s\n", buf);
                    delete s;
                }
            }
            for (int i = nServers; --i >= 0;) {
                lockConnection(i);
                socket_t* s;
                if (((s = con[i].respSock) != NULL && FD_ISSET(s->get_handle(), &ready))
                    || ((s = con[i].reqSock) != NULL && FD_ISSET(s->get_handle(), &ready)))

                {
                    if (!s->read(&rr, sizeof rr)) {
                        dbTrace("Failed to read response from node %d\n", i);
                        deleteConnection(i);
                        if (closed && i == activeNodeId) {
                            dbCriticalSection cs(startCS);
                            opened = false;
                            startEvent.signal();
                            delete ctx;
                            unlockConnection(i);
                            return;
                        }
                    } else {
                        TRACE_MSG(("Receive request %s, status %s, size %d from node %d\n",
                                   requestText[rr.op],
                                   ((rr.status <= ST_RECOVERED) ? statusText[rr.status] : "?"),
                                   rr.size, rr.nodeId));
                        switch (rr.op) {
                          case ReplicationRequest::RR_COMMITTED:
                          {
                              dbCriticalSection cs(commitCS);
                              con[i].committedEvent.signal();
                              break;
                          }
                          case ReplicationRequest::RR_GET_STATUS:
                            rr.op = ReplicationRequest::RR_STATUS;
                            rr.nodeId = id;
                            rr.status = con[id].status;
                            rr.size = file.updateCounter;
                            TRACE_MSG(("Send RR_STATUS response %s to node %d\n",
                                       statusText[con[id].status], i));
                            writeResp(i, rr);
                            break;
                          case ReplicationRequest::RR_STATUS:
                            if (con[rr.nodeId].status == ST_RECOVERED && rr.status == ST_STANDBY) {
                                dbCriticalSection cs(startCS);
                                monitor->curr = header->curr;
                                recoveredEvent.signal();
                            }
                            con[rr.nodeId].status = rr.status;
                            con[rr.nodeId].updateCounter = rr.size;
                            if (con[rr.nodeId].waitStatusEventFlag) {
                                con[rr.nodeId].statusEvent.signal();
                            } else if (activeNodeId < 0 && rr.nodeId < id && rr.status == ST_RECOVERED) {
                                TRACE_MSG(("activeNodeId=%d rr.nodeId=%d, rr.status=%d\n", activeNodeId, rr.nodeId, rr.status));
                                changeActiveNode();
                            }
                            statusRequested = false;
                            break;
                          case ReplicationRequest::RR_NEW_ACTIVE_NODE:
                            TRACE_MSG(("New active node is %d\n", rr.nodeId));
                            activeNodeId = rr.nodeId;
                            statusRequested = false;
                            break;
                          case ReplicationRequest::RR_CHANGE_ACTIVE_NODE:
                            TRACE_MSG(("Change active node to %d\n", rr.nodeId));
                            activeNodeId = rr.nodeId;
                            statusRequested = false;
                            if (accessType != dbConcurrentRead) {
                                rr.op = ReplicationRequest::RR_RECOVERY;
                                rr.size = file.getUpdateCountTableSize();
                                rr.nodeId = id;
                                writeReq(activeNodeId, rr, file.diskUpdateCount, rr.size*sizeof(int));
                            }
                            break;
                          case ReplicationRequest::RR_CLOSE:
                            if (accessType != dbConcurrentRead) {
                                closed = true;
                            }
                            break;
                          case ReplicationRequest::RR_RECOVERY:
                            {
                                int* updateCountTable = new int[file.getMaxPages()];

                                if (rr.size != 0 && !s->read(updateCountTable, rr.size*sizeof(int))) {
                                    dbTrace("Failed to read update counter table\n");
                                    deleteConnection(i);
                                } else {
                                    con[i].status = ST_RECOVERED;
                                    file.recovery(i, updateCountTable, rr.size);
                                }
                                break;
                            }
                          case ReplicationRequest::RR_RECOVER_PAGE:
                            TRACE_MSG(("Recovere page at address %x size %d\n", rr.page.offs, rr.size));
                            if (!file.updatePages(s, rr.page.offs, rr.page.updateCount, rr.size))
                            {
                                dbTrace("Failed to recover page %lx\n", (long)rr.page.offs);
                                activeNodeId = -1;
                            }
                            break;
                          case ReplicationRequest::RR_UPDATE_PAGE:
                            TRACE_MSG(("Update page at address %x size %d\n", rr.page.offs, rr.size));
                            if ((accessType != dbAllAccess
                                 && !file.concurrentUpdatePages(s, rr.page.offs, rr.page.updateCount, rr.size))
                                || (accessType == dbAllAccess
                                    && !file.updatePages(s, rr.page.offs, rr.page.updateCount,
                                                         rr.size)))
                            {
                                dbTrace("Failed to update page %lx\n", (long)rr.page.offs);
                                activeNodeId = -1;
                            }
                            break;
                          case ReplicationRequest::RR_READY:
                            con[rr.nodeId].readyEvent.signal();
                            break;
                          default:
                            dbTrace("Unexpected request %d from node %d\n", rr.op, i);
                        }
                    }
                }
                unlockConnection(i);
            }
        }
    }
    delete ctx;
}

void dbReplicatedDatabase::close()
{
    assert(opened);
    detach();
    opened = false;
    readerThread.join();
    file.flush();
    if (header != NULL && header->dirty && accessType != dbConcurrentRead)
    {
        header->dirty = false;
        file.markAsDirty(0, sizeof(dbHeader));
        file.flush(false);
    }
    ReplicationRequest rr;
    rr.op = ReplicationRequest::RR_CLOSE;
    rr.nodeId = id;
    for (int i = nServers; --i >= 0;) {
        if (con[i].reqSock != NULL
            && (con[i].status == ST_STANDBY || con[i].status == ST_RECOVERED))
        {
            con[i].reqSock->write(&rr, sizeof rr);
        }
    }
    dbDatabase::close();
    delete[] con;
    delete acceptSock;
    startEvent.close();
}

bool dbReplicatedDatabase::writeReq(int nodeId, ReplicationRequest const& hdr,
                                    void* body, size_t bodySize)
{
    bool result;
    lockConnection(nodeId);
    dbCriticalSection cs(con[nodeId].writeCS);
    socket_t* s = con[nodeId].reqSock;
    if (s == NULL) {
        s = con[nodeId].respSock;
    }
    if (s != NULL) {
        if (!s->write(&hdr, sizeof hdr, replicationWriteTimeout) ||
            (bodySize != 0 && !s->write(body, bodySize, replicationWriteTimeout)))
        {
            char buf[64];
            s->get_error_text(buf, sizeof buf);
            dbTrace("Failed to write request to node %d: %s\n", nodeId, buf);
            deleteConnection(nodeId);
            result = false;
        } else {
            result = true;
        }
    } else {
        TRACE_MSG(("Connection %d request socket is not opened\n", nodeId));
        result = false;
    }
    unlockConnection(nodeId);
    return result;
}

bool dbReplicatedDatabase::writeResp(int nodeId, ReplicationRequest const& hdr)
{
    lockConnection(nodeId);
    bool result;
    socket_t* s = con[nodeId].respSock;
    if (s == NULL) {
        s = con[nodeId].reqSock;
    }
    if (s != NULL) {
        if (!s->write(&hdr, sizeof hdr)) {
            char buf[64];
            s->get_error_text(buf, sizeof buf);
            dbTrace("Failed to write request to node %d: %s\n", nodeId, buf);
            deleteConnection(nodeId);
            result = false;
        } else {
            result = true;
        }
    } else {
        result = false;
    }
    unlockConnection(nodeId);
    return result;
}

void dbReplicatedDatabase::lockConnection(int nodeId)
{
    dbCriticalSection cs(sockCS);
    con[nodeId].useCount += 1;
}

void dbReplicatedDatabase::unlockConnection(int nodeId)
{
    dbCriticalSection cs(sockCS);
    if (--con[nodeId].useCount == 0 && con[nodeId].waitUseEventFlag) {
        con[nodeId].useEvent.signal();
    }
}

void dbReplicatedDatabase::addConnection(int nodeId, socket_t* s)
{
    TRACE_MSG(("Add connection with node %d\n", nodeId));
    lockConnection(nodeId);
    if (con[nodeId].reqSock != NULL) {
        deleteConnection(nodeId);
    }
    {
        dbCriticalSection cs(sockCS);
        con[nodeId].reqSock = s;
        int hnd = s->get_handle();
        if (hnd >= nInputSD) {
            nInputSD = hnd+1;
        }
        FD_SET(hnd, &inputSD);
    }
    unlockConnection(nodeId);
}

void dbReplicatedDatabase::waitTransactionAcknowledgement()
{
    int i;
    int n = nServers;
    dbCriticalSection cs(commitCS);
    for (i = 0; i < n; i++) {
        con[i].committedEvent.reset();
    }
    file.flush();
    for (i = 0; i < n; i++) {
        if (con[i].status == dbReplicatedDatabase::ST_STANDBY) {
            con[i].committedEvent.wait(commitCS);
        }
    }
}

#endif

bool dbHeader::isCompatible()
{
    return getVersion() < 329 || getCurrentMode() == mode;
}

int dbHeader::getCurrentMode()
{
    int mode = MODE_RECTANGLE_DIM * RECTANGLE_DIMENSION * sizeof(RECTANGLE_COORDINATE_TYPE);
#if dbDatabaseOffsetBits > 32
    mode |= MODE_OFFS_64;
#endif
#if dbDatabaseOidBits > 32
    mode |= MODE_OID_64;
#endif
#ifdef AUTOINCREMENT_SUPPORT
    mode |= MODE_AUTOINCREMENT;
#endif
    return mode;
}

END_FASTDB_NAMESPACE
