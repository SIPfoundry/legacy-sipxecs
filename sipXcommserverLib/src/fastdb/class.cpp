//-< CLASS.CPP >-----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update:  1-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Metaclass information
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "fastdb.h"
#include "compiler.h"
#include "symtab.h"

BEGIN_FASTDB_NAMESPACE

#ifndef CHECK_RECORD_SIZE
#define CHECK_RECORD_SIZE 1
#endif

dbTableDescriptor* dbTableDescriptor::chain;

void* dbFieldDescriptor::operator new(size_t size EXTRA_DEBUG_NEW_PARAMS)
{
    return dbMalloc(size);
}

void  dbFieldDescriptor::operator delete(void* p EXTRA_DEBUG_NEW_PARAMS)
{
    dbFree(p);
}

dbFieldDescriptor::dbFieldDescriptor(char const* name)
{
    next = prev = this;
    this->name = (char*)name;
    longName = NULL;
    dbSymbolTable::add(this->name, tkn_ident, FASTDB_CLONE_ANY_IDENTIFIER);
    appOffs = dbsOffs = 0;
    defTable = refTable = NULL;
    refTableName = NULL;
    components = NULL;
    inverseRefName = NULL;
    inverseRef = NULL;
    indexType = 0;
    method = NULL;
    attr = OneToOneMapping;
    tTree = 0;
    hashTable = 0;
    comparator = (dbUDTComparator)&memcmp;
}


dbFieldDescriptor::dbFieldDescriptor(char const*        fieldName,
                                     size_t             offs,
                                     size_t             size,
                                     int                index,
                                     char const*        inverse,
                                     dbFieldDescriptor* fieldComponents)
{
    next = prev = this;
    name = (char*)fieldName;
    longName = NULL;
    dbSymbolTable::add(name, tkn_ident, FASTDB_CLONE_ANY_IDENTIFIER);
    appOffs = (int)offs;
    dbsOffs = 0;
    alignment = appSize = dbsSize = (int)size;
    defTable = refTable = NULL;
    refTableName = NULL;
    indexType = index;
    type = appType = dbField::tpStructure;
    inverseRefName = (char*)inverse;
    if (inverseRefName != NULL) {
        dbSymbolTable::add(inverseRefName, tkn_ident, FASTDB_CLONE_ANY_IDENTIFIER);
    }
    inverseRef = NULL;
    components = fieldComponents;
    method = NULL;
    attr = 0;
    tTree = 0;
    hashTable = 0;
    comparator = (dbUDTComparator)&memcmp;
}

dbFieldDescriptor::~dbFieldDescriptor()
{
    if (type == dbField::tpString) {
        delete components;
    } else if (type == dbField::tpStructure) {
        dbFieldDescriptor* last = components->prev;
        while (last->method != NULL) {
            dbFieldDescriptor* prev = last->prev;
            delete last->method;
            delete last;
            if (last == components) {
                break;
            }
            last = prev;
        }
    }
    delete[] longName;
}


dbFieldDescriptor* dbFieldDescriptor::find(const char* name)
{
    dbFieldDescriptor* field = components;
    do {
        if (field->name == name) {
            return field;
        }
    } while ((field = field->next) != components);
    return NULL;
}


void dbFieldDescriptor::adjustReferences(byte* record,
                                         size_t base, size_t size, long shift)
{
    dbFieldDescriptor* fd = this;
    do {

        if (fd->type == dbField::tpArray) {
            byte** ptr = (byte**)((dbAnyArray*)(record + fd->appOffs) + 1);
            if ((unsigned long)*ptr - base <= size) {
                *ptr += shift;
            } else {
                if (fd->attr & HasArrayComponents) {
                    int nElems = ((dbAnyArray*)(record+fd->appOffs))->length();
                    byte* arrBase = *ptr;
                    while (--nElems >= 0) {
                        fd->components->adjustReferences(arrBase,
                                                         base, size, shift);
                        arrBase += fd->components->appSize;
                    }
                }
            }
        } else if (fd->type == dbField::tpString) {
            char** str = (char**)(record + fd->appOffs);
            if ((unsigned long)*str - base <= size) {
                *str += shift;
            }
        } else if (fd->attr & HasArrayComponents) {
            fd->components->adjustReferences(record + fd->appOffs,
                                             base, size, shift);
        }
    } while ((fd = fd->next) != this);
}


size_t dbFieldDescriptor::calculateRecordSize(byte* base, size_t offs)
{
    dbFieldDescriptor* fd = this;
    do {
        switch (fd->appType) {
          case dbField::tpArray:
            {
                int nElems = ((dbAnyArray*)(base + fd->appOffs))->length();
                offs = DOALIGN(offs, fd->components->alignment)
                    + nElems*fd->components->dbsSize;
                if (fd->attr & HasArrayComponents) {
                    byte* elem = (byte*)((dbAnyArray*)(base+fd->appOffs))->base();
                    dbFieldDescriptor* component = fd->components;
                    size_t elemSize = component->appSize;
                    while (--nElems >= 0) {
                        offs = component->calculateRecordSize(elem, offs);
                        elem += elemSize;
                    }
                }
                continue;
            }
          case dbField::tpString:
            {
                char* str = *(char**)(base + fd->appOffs);
                if (str != NULL) {
                    offs += strlen(str) + 1;
                } else {
                    offs += 1;
                }
                continue;
            }
#ifdef USE_STD_STRING
          case dbField::tpStdString:
            offs += ((std::string*)(base + fd->appOffs))->length() + 1;
            continue;
#endif
          default:
            if (fd->attr & HasArrayComponents) {
                offs = fd->components->calculateRecordSize(base+fd->appOffs, offs);
            }
        }
    } while ((fd = fd->next) != this);
    return offs;
}



size_t dbFieldDescriptor::calculateNewRecordSize(byte* base, size_t offs)
{
    dbFieldDescriptor* fd = this;
    do {
        if (fd->type == dbField::tpArray) {
            if (fd->oldDbsType == dbField::tpUnknown) {
                continue;
            }
            int nElems = ((dbVarying*)(base + fd->oldDbsOffs))->size;
            offs = DOALIGN(offs, fd->components->alignment)
                 + nElems*fd->components->dbsSize;
            if (fd->attr & HasArrayComponents) {
                byte* elem = base + ((dbVarying*)(base+fd->oldDbsOffs))->offs;
                while (--nElems >= 0) {
                    offs = fd->components->calculateNewRecordSize(elem, offs);
                    elem += fd->components->oldDbsSize;
                }
            }
        } else if (fd->type == dbField::tpString) {
            if (fd->oldDbsType == dbField::tpUnknown) {
                offs += 1;
            } else {
                offs += ((dbVarying*)(base + fd->oldDbsOffs))->size;
            }
        } else if (fd->attr & HasArrayComponents) {
            offs = fd->components->calculateNewRecordSize(base, offs);
        }
    } while ((fd = fd->next) != this);
    return offs;
}

void dbFieldDescriptor::fetchRecordFields(byte* dst, byte* src)
{
    dbFieldDescriptor* fd = this;
    do {
        switch (fd->appType) {
          case dbField::tpBool:
            *(bool*)(dst+fd->appOffs) = *(bool*)(src+fd->dbsOffs);
            break;
          case dbField::tpInt1:
            *(int1*)(dst+fd->appOffs) = *(int1*)(src+fd->dbsOffs);
            break;
          case dbField::tpInt2:
            *(int2*)(dst+fd->appOffs) = *(int2*)(src+fd->dbsOffs);
            break;
          case dbField::tpInt4:
            *(int4*)(dst+fd->appOffs) = *(int4*)(src+fd->dbsOffs);
            break;
          case dbField::tpInt8:
            *(db_int8*)(dst+fd->appOffs) = *(db_int8*)(src+fd->dbsOffs);
            break;
          case dbField::tpReal4:
            *(real4*)(dst+fd->appOffs) = *(real4*)(src+fd->dbsOffs);
            break;
          case dbField::tpReal8:
            *(real8*)(dst+fd->appOffs) = *(real8*)(src+fd->dbsOffs);
            break;
          case dbField::tpRawBinary:
            memcpy(dst+fd->appOffs, src+fd->dbsOffs, fd->dbsSize);
            break;
          case dbField::tpString:
            *(char**)(dst+fd->appOffs) =
                (char*)(src + ((dbVarying*)(src+fd->dbsOffs))->offs);
            break;
#ifdef USE_STD_STRING
          case dbField::tpStdString:
            ((std::string*)(dst + fd->appOffs))->assign((char*)(src + ((dbVarying*)(src+fd->dbsOffs))->offs),
                                                   ((dbVarying*)(src+fd->dbsOffs))->size - 1);
            break;
#endif
          case dbField::tpArray:
            {
                int nElems = ((dbVarying*)(src+fd->dbsOffs))->size;
                byte* srcElem = src + ((dbVarying*)(src+fd->dbsOffs))->offs;
                dbAnyArray* array = (dbAnyArray*)(dst+fd->appOffs);
                if (fd->attr & dbFieldDescriptor::OneToOneMapping) {
                    fd->arrayAllocator(array, srcElem, nElems);
                } else {
                    fd->arrayAllocator(array, NULL, nElems);
                    byte* dstElem = (byte*)array->base();
                    dbFieldDescriptor* component = fd->components;
                    while (--nElems >= 0) {
                        component->fetchRecordFields(dstElem, srcElem);
                        dstElem += component->appSize;
                        srcElem += component->dbsSize;
                    }
                }
            }
            break;
          case dbField::tpReference:
            ((dbAnyReference*)(dst+fd->appOffs))->oid =
                *(oid_t*)(src+fd->dbsOffs);
            break;
          case dbField::tpRectangle:
            *(rectangle*)(dst+fd->appOffs) = *(rectangle*)(src+fd->dbsOffs);
            break;
          case dbField::tpStructure:
            fd->components->fetchRecordFields(dst + fd->appOffs, src);
            break;
          default:
            return;
        }
    } while ((fd = fd->next) != this);
}


size_t dbFieldDescriptor::storeRecordFields(byte* dst, byte* src, size_t offs, StoreMode mode)
{
    dbFieldDescriptor* fd = this;
    do {
#ifdef AUTOINCREMENT_SUPPORT
        if ((fd->indexType & AUTOINCREMENT) != 0) {
            if (mode == Insert || (mode == Import && *(int4*)(src+fd->appOffs) == 0)) {
                assert (fd->appType == dbField::tpInt4);
                *(int4*)(dst+fd->dbsOffs) = *(int4*)(src+fd->appOffs) = fd->defTable->autoincrementCount;
                continue;
            } else if (mode == Import) {
                if (*(int4*)(src+fd->appOffs) > fd->defTable->autoincrementCount) {
                    fd->defTable->autoincrementCount = *(int4*)(src+fd->appOffs);
                }
            }
        }
#endif
        switch (fd->appType) {
          case dbField::tpBool:
            *(bool*)(dst+fd->dbsOffs) = *(bool*)(src+fd->appOffs);
            break;
          case dbField::tpInt1:
            *(int1*)(dst+fd->dbsOffs) = *(int1*)(src+fd->appOffs);
            break;
          case dbField::tpInt2:
            *(int2*)(dst+fd->dbsOffs) = *(int2*)(src+fd->appOffs);
            break;
          case dbField::tpInt4:
            *(int4*)(dst+fd->dbsOffs) = *(int4*)(src+fd->appOffs);
            break;
          case dbField::tpInt8:
            *(db_int8*)(dst+fd->dbsOffs) = *(db_int8*)(src+fd->appOffs);
            break;
          case dbField::tpReal4:
            *(real4*)(dst+fd->dbsOffs) = *(real4*)(src+fd->appOffs);
            break;
          case dbField::tpReal8:
            *(real8*)(dst+fd->dbsOffs) = *(real8*)(src+fd->appOffs);
            break;
          case dbField::tpRawBinary:
            memcpy(dst+fd->dbsOffs, src+fd->appOffs, fd->dbsSize);
            break;
#ifdef USE_STD_STRING
          case dbField::tpStdString:
            {
                ((dbVarying*)(dst+fd->dbsOffs))->offs = offs;
                std::string* str = (std::string*)(src + fd->appOffs);
                int len = str->length();
                str->copy((char*)dst + offs, len);
                dst[offs + len] = '\0';
                ((dbVarying*)(dst+fd->dbsOffs))->size = len+1;
                offs += (len+1);
            }
            break;
#endif
          case dbField::tpString:
            {
                ((dbVarying*)(dst+fd->dbsOffs))->offs = offs;
                char* str = *(char**)(src + fd->appOffs);
                if (str != NULL) {
                    strcpy((char*)dst + offs, str);
                    offs += ((dbVarying*)(dst+fd->dbsOffs))->size = strlen(str) + 1;
                } else {
                    *((char*)dst + offs) = '\0';
                    ((dbVarying*)(dst+fd->dbsOffs))->size = 1;
                    offs += 1;
                }
            }
            break;
          case dbField::tpArray:
            {
                int nElems = ((dbAnyArray*)(src + fd->appOffs))->length();
                byte* srcElem=(byte*)((dbAnyArray*)(src+fd->appOffs))->base();
                offs = DOALIGN(offs, fd->components->alignment);
                byte* dstElem = dst+offs;
                ((dbVarying*)(dst+fd->dbsOffs))->size = nElems;
                ((dbVarying*)(dst+fd->dbsOffs))->offs = offs;
                size_t sizeElem = fd->components->dbsSize;
                size_t offsElem = nElems*sizeElem;
                offs += offsElem;
                if (srcElem != NULL) {
                    if (fd->attr & dbFieldDescriptor::OneToOneMapping) {
                        memcpy(dstElem, srcElem, offsElem);
                    } else {
                        dbFieldDescriptor* component = fd->components;
                        while (--nElems >= 0) {
                            offsElem =
                                component->storeRecordFields(dstElem, srcElem, offsElem, mode);
                            offsElem -= sizeElem;
                            dstElem += sizeElem;
                            srcElem += component->appSize;
                        }
                        offs += offsElem;
                    }
                }

            }
            break;
          case dbField::tpReference:
            *(oid_t*)(dst+fd->dbsOffs) =
                ((dbAnyReference*)(src+fd->appOffs))->oid;
            break;
          case dbField::tpRectangle:
            *(rectangle*)(dst+fd->dbsOffs) = *(rectangle*)(src+fd->appOffs);
            break;
          case dbField::tpStructure:
            offs = fd->components->storeRecordFields(dst, src+fd->appOffs, offs, mode);
            break;
          default:
            return offs;
        }
    } while ((fd = fd->next) != this);

    return offs;
}


void dbFieldDescriptor::markUpdatedFields(byte* dst, byte* src)
{
    dbFieldDescriptor* fd = this;
    do {
        if (fd->indexType & (HASHED|INDEXED)) {
            switch (fd->appType) {
              case dbField::tpBool:
                if (*(bool*)(dst+fd->dbsOffs) != *(bool*)(src+fd->appOffs)) {
                    fd->attr |= Updated;
                }
                break;
              case dbField::tpInt1:
                if (*(int1*)(dst+fd->dbsOffs) != *(int1*)(src+fd->appOffs)) {
                    fd->attr |= Updated;
                }
                break;
              case dbField::tpInt2:
                if (*(int2*)(dst+fd->dbsOffs) != *(int2*)(src+fd->appOffs)) {
                    fd->attr |= Updated;
                }
                break;
              case dbField::tpInt4:
                if (*(int4*)(dst+fd->dbsOffs) != *(int4*)(src+fd->appOffs)) {
                    fd->attr |= Updated;
                }
                break;
              case dbField::tpInt8:
                if (*(db_int8*)(dst+fd->dbsOffs) != *(db_int8*)(src+fd->appOffs)) {
                    fd->attr |= Updated;
                }
                break;
              case dbField::tpReference:
                if (*(oid_t*)(dst+fd->dbsOffs) != *(oid_t*)(src+fd->appOffs)) {
                    fd->attr |= Updated;
                }
                break;
              case dbField::tpRectangle:
                if (*(rectangle*)(dst+fd->dbsOffs) != *(rectangle*)(src+fd->appOffs)) {
                    fd->attr |= Updated;
                }
                break;
              case dbField::tpReal4:
                if (*(real4*)(dst+fd->dbsOffs) != *(real4*)(src+fd->appOffs)) {
                    fd->attr |= Updated;
                }
                break;
              case dbField::tpReal8:
                if (*(real8*)(dst+fd->dbsOffs) != *(real8*)(src+fd->appOffs)) {
                    fd->attr |= Updated;
                }
                break;
              case dbField::tpRawBinary:
                if (memcmp(dst+fd->dbsOffs, src+fd->appOffs, fd->dbsSize) != 0) {
                    fd->attr |= Updated;
                }
                break;
              case dbField::tpString:
                if (strcmp((char*)dst + ((dbVarying*)(dst+fd->dbsOffs))->offs,
                           *(char**)(src + fd->appOffs)) != 0)
                {
                    fd->attr |= Updated;
                }
                break;
#ifdef USE_STD_STRING
              case dbField::tpStdString:
                if (*(std::string*)(src + fd->appOffs) != (char*)(dst + ((dbVarying*)(dst+fd->dbsOffs))->offs)) {
                    fd->attr |= Updated;
                }
                break;
#endif
              case dbField::tpStructure:
                fd->components->markUpdatedFields(dst, src+fd->appOffs);
                break;
              case dbField::tpArray:
                break;
              default:
                return;
            }
        }
    } while ((fd = fd->next) != this);
}


size_t dbFieldDescriptor::convertRecord(byte* dst, byte* src, size_t offs)
{
    dbFieldDescriptor* fd = this;
    int1  i1;
    int2  i2;
    int4  i4;
    db_int8  i8;
    real4 f4;
    real8 f8;
    bool  b;
    do {
        switch (fd->type) {
          case dbField::tpBool:
            switch (fd->oldDbsType) {
              case dbField::tpBool:
                b = *(bool*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt1:
                b = *(int1*)(src + fd->oldDbsOffs) != 0;
                break;
              case dbField::tpInt2:
                b = *(int2*)(src + fd->oldDbsOffs) != 0;
                break;
              case dbField::tpInt4:
                b = *(int4*)(src + fd->oldDbsOffs) != 0;
                break;
              case dbField::tpInt8:
                b = *(db_int8*)(src + fd->oldDbsOffs) != 0;
                break;
              case dbField::tpReal4:
                b = *(real4*)(src + fd->oldDbsOffs) != 0;
                break;
              case dbField::tpReal8:
                b = *(real8*)(src + fd->oldDbsOffs) != 0;
                break;
              default:
                b = false;
            }
            *(bool*)(dst + fd->dbsOffs) = b;
            break;

          case dbField::tpInt1:
            switch (fd->oldDbsType) {
              case dbField::tpBool:
                i1 = *(bool*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt1:
                i1 = *(int1*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt2:
                i1 = (int1)*(int2*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt4:
                i1 = (int1)*(int4*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt8:
                i1 = (int1)*(db_int8*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpReal4:
                i1 = (int1)*(real4*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpReal8:
                i1 = (int1)*(real8*)(src + fd->oldDbsOffs);
                break;
              default:
                i1 = 0;
            }
            *(int1*)(dst + fd->dbsOffs) = i1;
            break;

          case dbField::tpInt2:
            switch (fd->oldDbsType) {
              case dbField::tpBool:
                i2 = *(bool*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt1:
                i2 = *(int1*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt2:
                i2 = *(int2*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt4:
                i2 = (int2)*(int4*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt8:
                i2 = (int2)*(db_int8*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpReal4:
                i2 = (int2)*(real4*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpReal8:
                i2 = (int2)*(real8*)(src + fd->oldDbsOffs);
                break;
              default:
                i2 = 0;
            }
            *(int2*)(dst + fd->dbsOffs) = i2;
            break;

          case dbField::tpInt4:
            switch (fd->oldDbsType) {
              case dbField::tpBool:
                i4 = *(bool*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt1:
                i4 = *(int1*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt2:
                i4 = *(int2*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt4:
                i4 = *(int4*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt8:
                i4 = (int4)*(db_int8*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpReal4:
                i4 = (int4)*(real4*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpReal8:
                i4 = (int4)*(real8*)(src + fd->oldDbsOffs);
                break;
              default:
                i4 = 0;
            }
            *(int4*)(dst + fd->dbsOffs) = i4;
            break;

          case dbField::tpInt8:
            switch (fd->oldDbsType) {
              case dbField::tpBool:
                i8 = *(bool*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt1:
                i8 = *(int1*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt2:
                i8 = *(int2*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt4:
                i8 = *(int4*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt8:
                i8 = *(db_int8*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpReal4:
                i8 = (db_int8)*(real4*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpReal8:
                i8 = (db_int8)*(real8*)(src + fd->oldDbsOffs);
                break;
              default:
                i8 = 0;
            }
            *(db_int8*)(dst + fd->dbsOffs) = i8;
            break;

          case dbField::tpReal4:
            switch (fd->oldDbsType) {
              case dbField::tpBool:
                f4 = (real4)*(bool*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt1:
                f4 = (real4)*(int1*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt2:
                f4 = (real4)*(int2*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt4:
                f4 = (real4)*(int4*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt8:
                f4 = (real4)*(db_int8*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpReal4:
                f4 = *(real4*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpReal8:
                f4 = (real4)*(real8*)(src + fd->oldDbsOffs);
                break;
              default:
                f4 = 0;
            }
            *(real4*)(dst + fd->dbsOffs) = f4;
            break;

          case dbField::tpReal8:
            switch (fd->oldDbsType) {
              case dbField::tpBool:
                f8 = (real8)*(bool*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt1:
                f8 = (real8)*(int1*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt2:
                f8 = (real8)*(int2*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt4:
                f8 = (real8)*(int4*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpInt8:
                f8 = (real8)*(db_int8*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpReal4:
                f8 = *(real4*)(src + fd->oldDbsOffs);
                break;
              case dbField::tpReal8:
                f8 = *(real8*)(src + fd->oldDbsOffs);
                break;
              default:
                f8 = 0;
            }
            *(real8*)(dst + fd->dbsOffs) = f8;
            break;

          case dbField::tpRawBinary:
            if (fd->oldDbsType == dbField::tpRawBinary) {
                memcpy(dst + fd->dbsOffs, src + fd->oldDbsOffs,
                       size_t(fd->oldDbsSize) < fd->dbsSize
                       ? size_t(fd->oldDbsSize) : fd->dbsSize);
            }
            break;

          case dbField::tpString:
            if (fd->oldDbsType == dbField::tpUnknown) {
                ((dbVarying*)(dst + fd->dbsOffs))->size = 1;
                ((dbVarying*)(dst + fd->dbsOffs))->offs = offs;
                *(char*)(dst + offs++) = '\0';
            } else {
                size_t len =
                    ((dbVarying*)(src + fd->oldDbsOffs))->size;
                ((dbVarying*)(dst + fd->dbsOffs))->size = len;
                ((dbVarying*)(dst + fd->dbsOffs))->offs = offs;
                memcpy(dst + offs,
                       src + ((dbVarying*)(src+fd->oldDbsOffs))->offs, len);
                offs += len;
            }
            break;

          case dbField::tpArray:
            if (fd->oldDbsType == dbField::tpUnknown) {
                ((dbVarying*)(dst + fd->dbsOffs))->size = 0;
                ((dbVarying*)(dst + fd->dbsOffs))->offs = 0;
            } else {
                int len = ((dbVarying*)(src+fd->oldDbsOffs))->size;
                byte* srcElem = src + ((dbVarying*)(src+fd->oldDbsOffs))->offs;
                ((dbVarying*)(dst + fd->dbsOffs))->size = len;
                offs = DOALIGN(offs, fd->components->alignment);
                byte* dstElem = dst+offs;
                ((dbVarying*)(dst+fd->dbsOffs))->offs = offs;
                size_t offsElem = len*fd->components->dbsSize;
                offs += offsElem;
                while (--len >= 0) {
                    offsElem = fd->components->convertRecord(dstElem, srcElem,
                                                             offsElem);
                    offsElem -= fd->components->dbsSize;
                    dstElem += fd->components->dbsSize;
                    srcElem += fd->components->oldDbsSize;
                }
                offs += offsElem;
            }
            break;

          case dbField::tpStructure:
            offs = fd->components->convertRecord(dst, src, offs);
            break;

          case dbField::tpReference:
            if (fd->oldDbsType == dbField::tpUnknown) {
                *(oid_t*)(dst + fd->dbsOffs) = 0;
            } else {
                *(oid_t*)(dst + fd->dbsOffs) = *(oid_t*)(src + fd->oldDbsOffs);
            }
            break;
          case dbField::tpRectangle:
            if (fd->oldDbsType == dbField::tpUnknown) {
                memset(dst + fd->dbsOffs, 0, sizeof(rectangle));
            } else {
                *(rectangle*)(dst + fd->dbsOffs) = *(rectangle*)(src + fd->oldDbsOffs);
            }
            break;
          default:
            return offs;
        }
    } while ((fd = fd->next) != this);

    return offs;
}


int dbTableDescriptor::initialAutoincrementCount;


dbTableDescriptor::dbTableDescriptor(char const*        tableName,
                                     dbDatabase*        database,
                                     size_t             objSize,
                                     describeFunc       func,
                                     dbTableDescriptor* original)
{
    cloneOf = original;
    isStatic = true;
    if (original == NULL) {
        next = chain;
        chain = this;
    }
    name = (char*)tableName;
    dbSymbolTable::add(name, tkn_ident, FASTDB_CLONE_ANY_IDENTIFIER);
    describeComponentsFunc = func;
    columns = (*func)();
    nextFieldLink = &firstField;
    hashedFields = NULL;
    indexedFields = NULL;
    inverseFields = NULL;
    tableId = 0;
    nFields = 0;
    nColumns = 0;
    db = database;
    fixedDatabase = database != NULL;
    fixedSize = sizeof(dbRecord);
    int attr = dbFieldDescriptor::OneToOneMapping;
    appSize = 0;
    autoincrementCount = initialAutoincrementCount;
    size_t maxAlignment = calculateFieldsAttributes(columns, "",
                                                    sizeof(dbRecord),
                                                    HASHED|INDEXED, attr);
#if CHECK_RECORD_SIZE
    appSize = DOALIGN(appSize, maxAlignment);
    if (appSize < objSize) {
        fprintf(stderr, "Warning: may be not all fields of the class '%s' "
                "were described\n", name);
    }
#endif
    *nextFieldLink = NULL;
}


int dbTableDescriptor::calculateFieldsAttributes(dbFieldDescriptor* first,
                                                 char const*        prefix,
                                                 int                offs,
                                                 int                indexMask,
                                                 int&               attr)
{
    dbFieldDescriptor *field = first;
    size_t alignment = 1;
    do {
        if (field->method) {
            assert(((void)"Not empty record", field != first));
            do {
                assert(((void)"Methods should be specified after variables",
                        field->method != NULL));
                field->dbsOffs = first->dbsOffs;
                field->components = first;
                if (attr & dbFieldDescriptor::OneToOneMapping) {
                    field->method = field->method->optimize();
                }
            } while ((field = field->next) != first);
            break;
        }
        if (*prefix != '\0') {
            char* p = new char[strlen(prefix)+strlen(field->name)+1];
            sprintf(p, "%s%s", prefix, field->name);
            field->longName = p;
        } else {
            nColumns += 1;
            field->longName = new char[strlen(field->name)+1];
            strcpy(field->longName, field->name);
        }
        field->defTable = this;
        field->indexType &= indexMask|DB_FIELD_INHERITED_MASK;
        field->attr = (attr & dbFieldDescriptor::ComponentOfArray)
                    | dbFieldDescriptor::OneToOneMapping;
        if (field->inverseRefName) {
            assert(!(attr & dbFieldDescriptor::ComponentOfArray)
                   && (field->type == dbField::tpReference
                       || (field->type == dbField::tpArray
                           && field->components->type==dbField::tpReference)));
            field->nextInverseField = inverseFields;
            inverseFields = field;
        }
        *nextFieldLink = field;
        nextFieldLink = &field->nextField;
        field->fieldNo = nFields++;

        switch (field->type) {
          case dbField::tpArray:
            {
                size_t saveOffs = fixedSize;
                size_t saveAppSize = appSize;
                fixedSize = 0;
                attr = (attr | dbFieldDescriptor::HasArrayComponents)
                     & ~dbFieldDescriptor::OneToOneMapping;
                field->attr |= dbFieldDescriptor::ComponentOfArray;
                calculateFieldsAttributes(field->components, field->longName,
                                          0, 0, field->attr);
                if (field->components->dbsSize != field->components->appSize) {
                    field->attr &= ~dbFieldDescriptor::OneToOneMapping;
                }
                fixedSize = saveOffs;
                appSize = DOALIGN(saveAppSize, sizeof(void*))
                    + sizeof(void*)*3;
                break;
            }
          case dbField::tpStructure:
            {
                char* aggregateName = new char[strlen(field->longName) + 2];
                sprintf(aggregateName, "%s.", field->longName);
                size_t saveOffs = fixedSize;
                size_t saveAppSize = appSize;
                appSize = 0;
                size_t struct_alignment =
                    calculateFieldsAttributes(field->components,
                                              aggregateName,
                                              offs + field->appOffs,
                                              field->indexType,
                                              field->attr);
                field->alignment = struct_alignment;
                field->dbsOffs = field->components->dbsOffs;
                attr |= field->attr & dbFieldDescriptor::HasArrayComponents;
                attr &= field->attr | ~dbFieldDescriptor::OneToOneMapping;
                field->dbsSize = DOALIGN(fixedSize-saveOffs, struct_alignment);
                if ((field->attr & dbFieldDescriptor::HasArrayComponents)
                    && struct_alignment < sizeof(void*))
                {
                    struct_alignment = sizeof(void*);
                }
                appSize = DOALIGN(appSize, struct_alignment)
                        + DOALIGN(saveAppSize, struct_alignment);
                delete[] aggregateName;
                break;
            }
          case dbField::tpString:
            attr = (attr | dbFieldDescriptor::HasArrayComponents)
                & ~dbFieldDescriptor::OneToOneMapping;
            // no break
          default:
            appSize = DOALIGN(appSize, field->appSize) + field->appSize;
        }
        if (alignment < field->alignment) {
            alignment = field->alignment;
        }
        if (field->type != dbField::tpStructure) {
            field->dbsOffs = fixedSize = DOALIGN(fixedSize, field->alignment);
            fixedSize += field->dbsSize;
            if (field->dbsOffs != offs + field->appOffs) {
                attr &= ~dbFieldDescriptor::OneToOneMapping;
            }

            if (field->indexType & (HASHED|INDEXED)) {
                assert(!(field->attr & dbFieldDescriptor::ComponentOfArray));
                if (field->indexType & HASHED) {
                    field->nextHashedField = hashedFields;
                    hashedFields = field;
                }
                if (field->indexType & INDEXED) {
                    field->nextIndexedField = indexedFields;
                    indexedFields = field;
                }
            }
        }
    } while ((field = field->next) != first);

    return alignment;
}


int dbFieldDescriptor::sizeWithoutOneField(dbFieldDescriptor* field,
                                           byte* base, size_t& size)
{
    dbFieldDescriptor* fd = this;
    int offs, last = 0;
    do {
        if (fd != field) {
            if (fd->type == dbField::tpArray || fd->type == dbField::tpString){
                dbVarying* arr = (dbVarying*)(base + fd->dbsOffs);
                if (arr->offs > last) {
                    last = arr->offs;
                }
                int n = arr->size;
                size = DOALIGN(size, fd->components->alignment)
                     + fd->components->dbsSize * n;
                if (fd->attr & HasArrayComponents) {
                    byte* elem = base + arr->offs;
                    while (--n >= 0) {
                        offs = fd->components->sizeWithoutOneField(field,
                                                                   elem, size);
                        if (arr->offs + offs > last) {
                            last = arr->offs + offs;
                        }
                        elem += fd->components->dbsSize;
                    }
                }
            } else if (fd->attr & HasArrayComponents) {
                offs = fd->components->sizeWithoutOneField(field, base, size);
                if (offs > last) {
                    last = offs;
                }
            }
        }
    } while ((fd = fd->next) != this);

    return last;
}


size_t dbFieldDescriptor::copyRecordExceptOneField(dbFieldDescriptor* field,
                                                   byte* dst, byte* src,
                                                   size_t offs)
{
    dbFieldDescriptor* fd = this;
    do {
        if (fd != field) {
            if (fd->type == dbField::tpArray || fd->type == dbField::tpString){
                dbVarying* srcArr = (dbVarying*)(src + fd->dbsOffs);
                dbVarying* dstArr = (dbVarying*)(dst + fd->dbsOffs);
                int n = srcArr->size;
                byte* srcElem = src + srcArr->offs;
                offs = DOALIGN(offs, fd->components->alignment);
                byte* dstElem = dst + offs;
                dstArr->offs = offs;
                dstArr->size = n;
                size_t sizeElem = fd->components->dbsSize;
                size_t offsElem = sizeElem * n;
                offs += offsElem;
                if (fd->attr & HasArrayComponents) {
                    while (--n >= 0) {
                        offsElem = fd->components->
                            copyRecordExceptOneField(field, dstElem, srcElem,
                                                     offsElem);
                        offsElem -= sizeElem;
                        dstElem += sizeElem;
                        srcElem += sizeElem;
                    }
                    offs += offsElem;
                } else {
                    memcpy(dstElem, srcElem, offsElem);
                }
            } else if (fd->attr & HasArrayComponents) {
                offs = fd->components->copyRecordExceptOneField(field, dst,
                                                                src, offs);
            } else if (fd->method == NULL) {
                memcpy(dst+fd->dbsOffs, src+fd->dbsOffs, fd->dbsSize);
            }
        }
    } while ((fd = fd->next) != this);

    return offs;
}


void dbTableDescriptor::checkRelationship()
{
    dbFieldDescriptor* fd;
    for (fd = inverseFields; fd != NULL; fd = fd->nextInverseField) {
        dbTableDescriptor* refTable =
            fd->refTable ? fd->refTable : fd->components->refTable;
        fd->inverseRef = refTable->findSymbol(fd->inverseRefName);
        if (fd->inverseRef == NULL
             || fd->inverseRef->inverseRefName != fd->name)
        {
            char msg[256];
            if (fd->inverseRef == NULL) {
                sprintf(msg, "Failed to locate inverse reference field %s.%s for field %s.%s",
                        refTable->name, fd->inverseRefName, fd->defTable->name, fd->longName);
            } else {
                sprintf(msg, "Inverse references for field %s.%s is %s.%s, but its inverse reference is %s",
                        fd->defTable->name, fd->longName, refTable->name, fd->inverseRefName, fd->inverseRef->inverseRefName);
            }
            db->handleError(dbDatabase::InconsistentInverseReference, msg);
        }
    }
}

dbFieldDescriptor* dbTableDescriptor::find(char const* name)
{
    char* symnam = (char*)name;
    dbSymbolTable::add(symnam, tkn_ident);
    return findSymbol(symnam);
}

dbFieldDescriptor* dbTableDescriptor::findSymbol(const char* name)
{
    dbFieldDescriptor* first = columns;
    dbFieldDescriptor* field = first;
    do {
        if (field->name == name) {
            return field;
        }
    } while ((field = field->next) != first);
    return NULL;
}

dbFieldDescriptor& dbFieldDescriptor::adjustOffsets(long offs)
{
    if (offs != 0) {
        dbFieldDescriptor* fd = this;
        do {
            fd->appOffs += offs;
        } while ((fd = fd->next) != this);
    }
    return *this;
}




bool dbTableDescriptor::match(dbTable* table, bool confirmDeleteColumns)
{
    unsigned nFields = table->fields.size;
    unsigned nMatches = 0;
    bool formatNotChanged = ((size_t)nFields == this->nFields);

    for (dbFieldDescriptor* fd = firstField; fd != NULL; fd = fd->nextField) {
        dbField* field = (dbField*)((char*)table + table->fields.offs);
        fd->oldDbsType = dbField::tpUnknown;
        for (int n = nFields; --n >= 0; field++) {
            if (strcmp(fd->longName, (char*)field + field->name.offs) == 0) {
                assert(((void)"field can be converted to new format",
                        (fd->type == dbField::tpReference
                         && field->type == dbField::tpReference
                         && strcmp((char*)field + field->tableName.offs,
                                      fd->refTable->name) == 0)
                        || (fd->type <= dbField::tpReal8
                            && field->type <= dbField::tpReal8)
                        || (fd->type == dbField::tpString
                            && field->type == dbField::tpString)
                        || (fd->type == dbField::tpRectangle
                            && field->type == dbField::tpRectangle)
                        || (fd->type >= dbField::tpArray
                            && fd->type == field->type)));
                fd->oldDbsType = field->type;
                fd->oldDbsOffs = field->offset;
                fd->oldDbsSize = field->size;
                if (field->type != fd->type || field->offset != fd->dbsOffs) {
                    formatNotChanged = false;
                }
                nMatches += 1;
                //
                // Try to reuse indices
                //
                fd->hashTable = 0;
                fd->tTree = 0;

                if (field->type == fd->type) {
                    if ((fd->indexType & HASHED) != 0 && field->hashTable != 0) {
                        fd->hashTable = field->hashTable; // reuse index
                        field->hashTable = 0; // prevent index from destruction
                    }
                    if ((fd->indexType & INDEXED) != 0 && field->tTree != 0) {
                        fd->tTree = field->tTree; // reuse index
                        field->tTree = 0; // prevent index from destruction
                    }
                }
                break;
            }
        }
    }
    if (!confirmDeleteColumns) {
        assert(((void)"field can be removed only from empty table",
                nFields==nMatches));
    }
    return formatNotChanged;
}

void dbTableDescriptor::setFlags() {
    for (dbFieldDescriptor* fd = firstField; fd != NULL; fd = fd->nextField) {
        if (fd->tTree != 0) {
            fd->indexType |= INDEXED;
        } else if (fd->hashTable != 0) {
            fd->indexType |= HASHED;
        }
    }
}


bool dbTableDescriptor::equal(dbTable* table)
{
#ifdef AUTOINCREMENT_SUPPORT
    autoincrementCount = table->count;
#endif
    if (nColumns != table->nColumns ||
        nFields != table->fields.size || fixedSize != table->fixedSize)
    {
        return false;
    }
    dbField* field = (dbField*)((char*)table + table->fields.offs);

    for (dbFieldDescriptor* fd = firstField; fd != NULL; fd = fd->nextField) {
        if (strcmp(fd->longName, (char*)field + field->name.offs) != 0
            || (!fd->refTable && *((char*)field+field->tableName.offs) != '\0')
            || (fd->refTable && strcmp((char*)field + field->tableName.offs,
                                       fd->refTable->name) != 0)
            || (fd->inverseRefName == NULL
                && *((char*)field + field->inverse.offs) != '\0')
            || (fd->inverseRefName != NULL
                && strcmp((char*)field + field->inverse.offs,
                          fd->inverseRefName) != 0)
            || fd->dbsOffs != field->offset
#ifndef OLD_FIELD_DESCRIPTOR_FORMAT
            || fd->indexType != (int)field->flags
#endif
            || fd->type != field->type)
        {
            return false;
        }
        fd->tTree = field->tTree;
        fd->hashTable = field->hashTable;
        field += 1;
    }
    return true;
}


dbTableDescriptor::dbTableDescriptor(dbTable* table)
{
    next = chain;
    chain = this;
    cloneOf = NULL;
    isStatic = false;
    name = (char*)table + table->name.offs;
    dbSymbolTable::add(name, tkn_ident, true);
    nextFieldLink = &firstField;
    hashedFields = NULL;
    indexedFields = NULL;
    inverseFields = NULL;
    nFields = 0;
    nColumns = 0;
    fixedSize = table->fixedSize;
    int attr = 0;
    appSize = 0;
    columns = buildFieldsList(table, "", 0, attr);
    *nextFieldLink = NULL;
    db = NULL;
    fixedDatabase = false;
    tableId = 0;
#ifdef AUTOINCREMENT_SUPPORT
    autoincrementCount = table->count;
#endif
}

struct _abool { char n; bool v; };
struct _aint1 { char n; int1 v; };
struct _aint2 { char n; int2 v; };
struct _aint4 { char n; int4 v; };
struct _aint8 { char n; db_int8 v; };
struct _areal4 { char n; real4 v; };
struct _areal8 { char n; real8 v; };
struct _astring { char n; char *v; };
struct _arawbinary { char n; char v[1]; };
struct _areference { char n; oid_t v; };
struct _aarray { char n; void* v; };
struct _arectangle { char n; rectangle v; };

#define COMPUTE_ALIGNED_SIZE(type) offsetof(_a##type, v)


dbFieldDescriptor* dbTableDescriptor::buildFieldsList(dbTable*    table,
                                                      char const* prefix,
                                                      int         prefixLen,
                                                      int&        attr)
{
    dbFieldDescriptor* components = NULL;
    dbField* field = (dbField*)((char*)table+table->fields.offs) + nFields;

    while (nFields < table->fields.size
           && strncmp((char*)field + field->name.offs, prefix, prefixLen) == 0)
    {
        char* longName = (char*)field + field->name.offs;
        char* name = longName + prefixLen;
        if (*name == '.') {
            name += 1;
        } else if (prefixLen != 0 && *name != '[') {
            break;
        }
        dbSymbolTable::add(name, tkn_ident, true);
        dbFieldDescriptor* fd = new dbFieldDescriptor(name);
        fd->dbsOffs = field->offset;
        fd->alignment = fd->dbsSize = field->size;
        fd->longName = new char[strlen(longName)+1];
        strcpy(fd->longName, longName);
        fd->type = fd->appType = field->type;
        fd->indexType = field->flags;
        int appFieldSize, appAlignedFieldSize;

        switch (field->type) {
          case dbField::tpBool:
            appAlignedFieldSize = COMPUTE_ALIGNED_SIZE(bool);
            appFieldSize = sizeof(bool);
            break;
          case dbField::tpInt1:
            appAlignedFieldSize = COMPUTE_ALIGNED_SIZE(int1);
            appFieldSize = sizeof(int1);
            break;
          case dbField::tpInt2:
            appAlignedFieldSize = COMPUTE_ALIGNED_SIZE(int2);
            appFieldSize = sizeof(int2);
            break;
          case dbField::tpInt4:
            appAlignedFieldSize = COMPUTE_ALIGNED_SIZE(int4);
            appFieldSize = sizeof(int4);
            break;
          case dbField::tpInt8:
            appAlignedFieldSize = COMPUTE_ALIGNED_SIZE(int8);
            appFieldSize = sizeof(db_int8);
            break;
          case dbField::tpReal4:
            appAlignedFieldSize = COMPUTE_ALIGNED_SIZE(real4);
            appFieldSize = sizeof(real4);
            break;
          case dbField::tpReal8:
            appAlignedFieldSize = COMPUTE_ALIGNED_SIZE(real8);
            appFieldSize = sizeof(real8);
            break;
          case dbField::tpString:
            appAlignedFieldSize = COMPUTE_ALIGNED_SIZE(string);
            appFieldSize = sizeof(char *);
            break;
          case dbField::tpRawBinary:
            appAlignedFieldSize = COMPUTE_ALIGNED_SIZE(rawbinary);
            appFieldSize = field->size;
            break;
          case dbField::tpReference:
            appAlignedFieldSize = COMPUTE_ALIGNED_SIZE(reference);
            appFieldSize = sizeof(dbAnyReference);
            break;
          case dbField::tpArray:
            appAlignedFieldSize = COMPUTE_ALIGNED_SIZE(array);
            appFieldSize = sizeof(dbArray<char>);
            break;
          case dbField::tpRectangle:
            appAlignedFieldSize = COMPUTE_ALIGNED_SIZE(rectangle);
            appFieldSize = sizeof(rectangle);
            break;
          default:
            appAlignedFieldSize = 1;
            appFieldSize = 0;
        }
        fd->appOffs = appSize = DOALIGN(appSize, appAlignedFieldSize);
        appSize += fd->appSize = appFieldSize;

        if ((fd->hashTable = field->hashTable) != 0) {
            fd->nextHashedField = hashedFields;
            hashedFields = fd;
        }
        if ((fd->tTree = field->tTree) != 0) {
            fd->nextIndexedField = indexedFields;
            indexedFields = fd;
        }
        fd->fieldNo = nFields++;
        fd->defTable = this;
        fd->refTable = NULL;
        fd->refTableName = NULL;
        if (field->hashTable != 0) {
            fd->indexType |= HASHED;
        }
        if (field->tTree != 0) {
            fd->indexType |= INDEXED;
        }
        if (field->tableName.size > 1) {
            fd->refTableName = (char*)field + field->tableName.offs;
            dbSymbolTable::add(fd->refTableName, tkn_ident, true);
        }
        fd->inverseRefName = NULL;
        if (field->inverse.size > 1) {
            fd->nextInverseField = inverseFields;
            inverseFields = fd;
            fd->inverseRefName = (char*)field + field->inverse.offs;
            dbSymbolTable::add(fd->inverseRefName, tkn_ident, true);
        }
        fd->attr = (attr & dbFieldDescriptor::ComponentOfArray) | dbFieldDescriptor::OneToOneMapping;

        *nextFieldLink = fd;
        nextFieldLink = &fd->nextField;

        if (prefixLen == 0) {
            nColumns += 1;
        }
        if (components == NULL) {
            components = fd;
        } else {
            fd->next = components;
            fd->prev = components->prev;
            components->prev->next = fd;
            components->prev = fd;
        }
        if (fd->type == dbField::tpArray || fd->type == dbField::tpString) {
            attr = (attr | dbFieldDescriptor::HasArrayComponents)
                & ~dbFieldDescriptor::OneToOneMapping;
            fd->attr |= dbFieldDescriptor::ComponentOfArray;
            fd->alignment = 4;
        }
        if (fd->type == dbField::tpArray || fd->type == dbField::tpStructure) {
            int saveAppSize = appSize;
            appSize = 0;
            fd->components =
                buildFieldsList(table, longName, strlen(longName), fd->attr);
            attr |= fd->attr & dbFieldDescriptor::HasArrayComponents;
            attr &= fd->attr | ~dbFieldDescriptor::OneToOneMapping;
            field = (dbField*)((char*)table + table->fields.offs) + nFields;
            if (fd->type == dbField::tpStructure) {
                size_t alignment = 1;
                dbFieldDescriptor* component = fd->components;
                do {
                    if (component->alignment > alignment) {
                        alignment = component->alignment;
                    }
                } while ((component = component->next) != fd->components);
                fd->alignment = alignment;
                fd->appSize = appSize = DOALIGN(appSize, alignment);
                fd->appOffs = saveAppSize = DOALIGN(saveAppSize, alignment);
                appSize += saveAppSize;
            } else {
                appSize = saveAppSize;
                switch (fd->components->type) {
                  case dbField::tpString:
                    fd->arrayAllocator = &dbArray<char*>::arrayAllocator;
                    fd->attr &= ~dbFieldDescriptor::OneToOneMapping;
                    break;
                  case dbField::tpBool:
                    fd->arrayAllocator = &dbArray<bool>::arrayAllocator;
                    break;
                  case dbField::tpInt1:
                    fd->arrayAllocator = &dbArray<int1>::arrayAllocator;
                    break;
                  case dbField::tpInt2:
                    fd->arrayAllocator = &dbArray<int2>::arrayAllocator;
                    break;
                  case dbField::tpInt4:
                    fd->arrayAllocator = &dbArray<int4>::arrayAllocator;
                    break;
                  case dbField::tpInt8:
                    fd->arrayAllocator = &dbArray<int8>::arrayAllocator;
                    break;
                  case dbField::tpReal4:
                    fd->arrayAllocator = &dbArray<real4>::arrayAllocator;
                    break;
                  case dbField::tpReal8:
                    fd->arrayAllocator = &dbArray<real8>::arrayAllocator;
                    break;
                  case dbField::tpReference:
                    fd->arrayAllocator = &dbArray<dbAnyReference>::arrayAllocator;
                    break;
                  default:
                    fd->arrayAllocator = &dbAnyArray::arrayAllocator;
                    break;
                }
            }
        } else {
            if (fd->type == dbField::tpString) {
                fd->components = new dbFieldDescriptor("[]");
                fd->components->type = fd->components->appType = dbField::tpInt1;
                fd->components->dbsSize = fd->components->appSize = 1;
                fd->components->alignment = 1;
            }
            field += 1;
        }
    }
    return components;
}


size_t dbTableDescriptor::totalNamesLength()
{
    dbFieldDescriptor* fd;
    size_t len = strlen(name) + 1;
    for (fd = firstField; fd != NULL; fd = fd->nextField) {
        if (fd->name != NULL) {
            len += strlen(fd->longName) + 3;
            if (fd->inverseRefName != NULL) {
                len += strlen(fd->inverseRefName);
            }
            if (fd->refTable != NULL) {
                len += strlen(fd->refTable->name);
            }
        }
    }
    return len;
}


dbTableDescriptor* dbTableDescriptor::clone()
{
    return new dbTableDescriptor(name,
                                 DETACHED_TABLE,
                                 appSize,
                                 describeComponentsFunc,
                                 this);
}

dbTableDescriptor::~dbTableDescriptor()
{
    if (cloneOf == NULL) {
         dbTableDescriptor **tpp;
         for (tpp = &chain; *tpp != this; tpp = &(*tpp)->next);
         *tpp = next;
    }
    dbFieldDescriptor* last = columns->prev;
    while (last->method != NULL) {
        dbFieldDescriptor* prev = last->prev;
        delete last->method;
        delete last;
        if (last == columns) {
            break;
        }
        last = prev;
    }
    dbFieldDescriptor *field, *nextField;
    for (field = firstField; field != NULL; field = nextField) {
        nextField = field->nextField;
        delete field;
    }
}


void dbTableDescriptor::cleanup()
{
    dbTableDescriptor* next, *desc;
    for (desc = chain; desc != NULL; desc = next) {
        next = desc->next;
        if (!desc->isStatic) {
            delete desc;
        }
    }
}


void dbTableDescriptor::storeInDatabase(dbTable* table)
{
    int offs = sizeof(dbTable) + sizeof(dbField)*nFields;
    table->name.offs = offs;
    table->name.size = strlen(name)+1;
    strcpy((char*)table + offs, name);
    offs += table->name.size;
    table->fields.offs = sizeof(dbTable);
    table->fields.size = nFields;
    table->nRows = 0;
    table->nColumns = nColumns;
    table->fixedSize = fixedSize;
    table->firstRow = 0;
    table->lastRow = 0;
#ifdef AUTOINCREMENT_SUPPORT
    if (autoincrementCount < initialAutoincrementCount) {
        autoincrementCount = initialAutoincrementCount;
    }
    table->count = autoincrementCount;
#endif

    dbFieldDescriptor* fd;
    dbField* field = (dbField*)((char*)table + table->fields.offs);
    offs -= sizeof(dbTable);
    for (fd = firstField; fd != NULL; fd = fd->nextField) {
        field->name.offs = offs;
        field->name.size = strlen(fd->longName) + 1;
        strcpy((char*)field + offs, fd->longName);
        offs += field->name.size;
        field->tableName.offs = offs;
        if (fd->refTable != NULL) {
            field->tableName.size = strlen(fd->refTable->name) + 1;
            strcpy((char*)field + offs, fd->refTable->name);
        } else if (fd->refTableName != NULL) {
            field->tableName.size = strlen(fd->refTableName) + 1;
            strcpy((char*)field + offs, fd->refTableName);
        } else {
            field->tableName.size = 1;
            *((char*)field + offs) = '\0';
        }
        offs += field->tableName.size;

        field->inverse.offs = offs;
        if (fd->inverseRefName != NULL) {
            field->inverse.size = strlen(fd->inverseRefName) + 1;
            strcpy((char*)field + offs, fd->inverseRefName);
        } else {
            field->inverse.size = 1;
            *((char*)field + offs) = '\0';
        }
        offs += field->inverse.size;

        field->tTree = fd->tTree;
        field->hashTable = fd->hashTable;
        field->type = fd->type;
        field->size = fd->dbsSize;
        field->offset = fd->dbsOffs;
        field->flags = fd->indexType;
        field += 1;
        offs -= sizeof(dbField);
    }
}

void* dbAnyMethodTrampoline::operator new(size_t size EXTRA_DEBUG_NEW_PARAMS)
{
    return dbMalloc(size);
}

void  dbAnyMethodTrampoline::operator delete(void* p EXTRA_DEBUG_NEW_PARAMS)
{
    dbFree(p);
}

dbAnyMethodTrampoline::~dbAnyMethodTrampoline() {}

END_FASTDB_NAMESPACE
