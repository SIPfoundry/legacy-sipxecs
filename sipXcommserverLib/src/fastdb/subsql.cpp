//-< SUBSQL.CPP >----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Interactive data manipulation language (subset of SQL)
//-------------------------------------------------------------------*--------*

#include "fastdb.h"
#include "compiler.h"
#include "wwwapi.h"
#include "subsql.h"
#include "symtab.h"
#include "hashtab.h"
#include "ttree.h"
#include "rtree.h"
#include <locale.h>
#include <stdio.h>
#include <ctype.h>

#if THREADS_SUPPORTED
#include "server.h"
#endif

BEGIN_FASTDB_NAMESPACE

static const char* typeMnem[] = {
        "Boolean",
        "Int1",
        "Int2",
        "Int4",
        "Int8",
        "Real4",
        "Real8",
        "String",
        "Reference",
        "Array",
        "MethodBool",
        "MethodInt1",
        "MethodInt2",
        "MethodInt4",
        "MethodInt8",
        "MethodReal4",
        "MethodReal8",
        "MethodString",
        "MethodReference",
        "Structure",
        "RawBinary",
        "StdString",
        "Rectangle",
        "Unknown" 
};

char* dbSubSql::prompt = ">> ";


dbSubSql::dbSubSql(dbAccessType accessType)
: dbDatabase(accessType)
{
    static struct { 
        char* name;
        int   tag;
    } keywords[] = { 
        {"alter",   tkn_alter},
        {"array",   tkn_array},
        {"autocommit",   tkn_autocommit},
        {"autoincrement",tkn_autoincrement},
        {"backup",  tkn_backup},        
        {"bool",    tkn_bool},
        {"commit",  tkn_commit},
        {"compactify",tkn_compactify},
        {"count",   tkn_count},
        {"create",  tkn_create},
        {"delete",  tkn_delete},
        {"describe",tkn_describe},
        {"drop",    tkn_drop},
        {"exit",    tkn_exit},
        {"export",  tkn_export},
        {"hash",    tkn_hash},
        {"help",    tkn_help},
        {"http",    tkn_http},
        {"import",  tkn_import},
        {"index",   tkn_index},
        {"int1",    tkn_int1},
        {"int2",    tkn_int2},
        {"int4",    tkn_int4},
        {"int8",    tkn_int8},
        {"inverse", tkn_inverse},
        {"memory",  tkn_memory},
        {"of",      tkn_of},
        {"off",     tkn_off},
        {"on",      tkn_on},
        {"open",    tkn_open},
        {"profile", tkn_profile},        
        {"reference",tkn_reference},
        {"real4",   tkn_real4},
        {"real8",   tkn_real8},
        {"rectangle", tkn_rectangle},
        {"rollback",tkn_rollback},
        {"server",  tkn_server},
        {"set",     tkn_set},
        {"stop",    tkn_stop},
        {"show",    tkn_show},
        {"to",      tkn_to},
        {"update",  tkn_update},
        {"values",  tkn_values},
        {"version", tkn_version}
    };
    for (unsigned i = 0; i < itemsof(keywords); i++) { 
        dbSymbolTable::add(keywords[i].name, keywords[i].tag, FASTDB_CLONE_ANY_IDENTIFIER);    
    }
    droppedTables = NULL;
    existedTables = NULL;
    opened = false;
    buflen = 1024;
    buf = new char[buflen];
    httpServerRunning = false;
    databaseName = NULL;
    historyUsed = historyCurr = 0;
    ungetToken = -1;
    dotIsPartOfIdentifier = false;
    autocommit = false;
    dateFormat = getenv("SUBSQL_DATE_FORMAT");
}

dbSubSql::~dbSubSql() 
{
    
    delete[] buf; 
}


//
// Find one string within another, ignoring case
//

inline char* stristr(const char* haystack, const char* needle)
{
    nat4 i, hayLen, ndlLen;

    ndlLen = strlen(needle);
    hayLen = strlen(haystack);

    if (ndlLen > hayLen) {
        return NULL;
    }

    for (i = 0; i <= (hayLen - ndlLen); i++) {
        if (strincmp(&haystack[i], needle, ndlLen) == 0) {
            return (char*)&haystack[i];
        }
    }
    return NULL;
}


bool contains(dbUserFunctionArgument& arg1, dbUserFunctionArgument& arg2) { 
    assert(arg1.type == dbUserFunctionArgument::atString && arg2.type == dbUserFunctionArgument::atString);
    return stristr(arg1.u.strValue, arg2.u.strValue) != NULL;
}

USER_FUNC(contains);

char* cmpOidAndStr(dbUserFunctionArgument& arg1, dbUserFunctionArgument& arg2) { 
    assert(arg1.type == dbUserFunctionArgument::atReference && arg2.type == dbUserFunctionArgument::atString);    
    printf("arg1.u.oidValue=%x, arg1.u.strValue=%s\n", arg1.u.oidValue, arg2.u.strValue);
    char* ptr = new char[16];
    sprintf(ptr, "%x", arg1.u.oidValue);
    return ptr;
}

USER_FUNC(cmpOidAndStr);

int dbSubSql::get() 
{
    int ch = getc(in);
    if (ch == '\n') { 
        pos = 0;
        line += 1;
    } else if (ch == '\t') {
        pos = DOALIGN(pos + 1, 8);
    } else {  
        pos += 1;
    }
    return ch;
}

void dbSubSql::unget(int ch) { 
    if (ch != EOF) { 
        if (ch != '\n') { 
            pos -= 1;
        } else { 
            line -= 1;
        }
        ungetc(ch, in);
    }
}

void dbSubSql::warning(char const* msg)
{
    fprintf(stderr, "%s at line %d position %d\n", msg, line, tknPos > 0 ? tknPos - 1 : 0);
}

void dbSubSql::error(char const* msg)
{
#ifdef THROW_EXCEPTION_ON_ERROR
   dbDatabaseThreadContext* ctx = threadContext.get();
    if (ctx != NULL) {
        ctx->interactive = true;
    }
    try { 
        handleError(QueryError, msg, tknPos > 0 ? tknPos - 1 : 0);
    } catch(dbException) {}
#else
    dbDatabaseThreadContext* ctx = threadContext.get();
    if (ctx != NULL) { 
        ctx->interactive = true;
        ctx->catched = true;
        if (setjmp(ctx->unwind) == 0) { 
            handleError(QueryError, msg, tknPos > 0 ? tknPos - 1 : 0);
        }
        ctx->catched = false;
    } else { 
        handleError(QueryError, msg, tknPos > 0 ? tknPos - 1 : 0);
    }
#endif
}


int dbSubSql::scan() 
{
    int i, ch, digits;
    
    if (ungetToken >= 0) { 
        int tkn = ungetToken;
        ungetToken = -1;
        return tkn;
    }
    bool dotIsIdentifierChar = dotIsPartOfIdentifier;
    dotIsPartOfIdentifier = false;
  nextToken:
    do { 
        if ((ch = get()) == EOF) { 
            return tkn_eof;
        }
    } while (isspace(ch));
    
    tknPos = pos;
    switch (ch) { 
      case '*':
        return tkn_all;
      case '(':
        return tkn_lpar;
      case ')':
        return tkn_rpar;
      case ',':
        return tkn_comma;
      case '.':
        return tkn_dot;
      case ';':
        return tkn_semi;
      case '=':
        return tkn_eq;
      case '\'':
        i = 0; 
        while (true) { 
            ch = get();
            if (ch == '\'') { 
                if ((ch = get()) != '\'') { 
                    unget(ch);
                    break;
                }
            } else if (ch == '\n' || ch == EOF) { 
                unget(ch);
                error("New line within character constant");
                return tkn_error;
            }
            if (i+1 == buflen) { 
                char* newbuf = new char[buflen*2];
                memcpy(newbuf, buf, buflen);
                delete[] buf;
                buf = newbuf;
                buflen *= 2;
            }
            buf[i++] = ch;
        }
        buf[i] = '\0';
        return tkn_sconst;
      case '-':
        if ((ch = get()) == '-') { 
            // ANSI comments
            while ((ch = get()) != EOF && ch != '\n');
            goto nextToken;
        }
        unget(ch);
        ch = '-';
        // no break
      case '0': case '1': case '2': case '3': case '4': 
      case '5': case '6': case '7': case '8': case '9':
      case '+':
        i = 0;
        do { 
            buf[i++] = ch;
            if (i == buflen) { 
                error("Numeric constant too long");
                return tkn_error;
            }
            ch = get();
        } while (ch != EOF 
                 && (isdigit(ch) || ch == '+' || ch == '-' || ch == 'e' || 
                     ch == 'E' || ch == '.'));
        unget(ch);
        buf[i] = '\0';
        if (sscanf(buf, INT8_FORMAT "%n", &ival, &digits) != 1) { 
            error("Bad integer constant");
            return tkn_error;
        }
        if (digits != i) { 
            if (sscanf(buf, "%lf%n", &fval, &digits) != 1 || digits != i) {
                error("Bad float constant");
                return tkn_error;
            }
            return tkn_fconst;
        } 
        return tkn_iconst;

      case '#':
        ival = 0;
        while (true) { 
            ch = get();
            if (ch >= '0' && ch <= '9') { 
                ival += (ival << 4) + ch-'0';
            } else if (ch >= 'a' && ch <= 'f') {
                ival += (ival << 4) + ch-'a'+10;
            } else if (ch >= 'A' && ch <= 'F') {
                ival += (ival << 4) + ch-'A'+10;
            } else { 
                unget(ch);
                return tkn_iconst;
            }
        }

      default:
        if (isalpha(ch) || ch == '$' || ch == '_') { 
            i = 0;
            do { 
                buf[i++] = ch;
                if (i == buflen) { 
                    error("Identifier too long");
                    return tkn_error;
                }
                ch = get();
            } while (ch != EOF && (isalnum(ch) || ch == '$' || ch == '_' || (ch == '.' && dotIsIdentifierChar)));
            unget(ch);
            buf[i] = '\0';
            name = buf;
            return dbSymbolTable::add(name, tkn_ident);
        } else { 
            error("Invalid symbol");
            return tkn_error;
        }
    }
}


bool dbSubSql::expect(char* expected, int token)
{
    int tkn = scan();
    if (tkn != token) { 
        if (tkn != tkn_error) { 
            char buf[256];
            sprintf(buf, "Token '%s' expected", expected);
            error(buf);
        }
        return false;
    }
    return true;
}

    
bool dbSubSql::updateTable(bool create)
{
    int tkn;
    dotIsPartOfIdentifier = true;
    if (!expect("table name", tkn_ident) || !expect("(", tkn_lpar)) { 
        return false;
    }
    char* name = this->name;
    int varyingLength = strlen(name)+1;

    static const struct { 
        int size;
        int alignment;
    } typeDesc[] = { 
        { sizeof(bool), sizeof(bool) }, 
        { sizeof(int1), sizeof(int1) }, 
        { sizeof(int2), sizeof(int2) }, 
        { sizeof(int4), sizeof(int4) }, 
        { sizeof(db_int8), sizeof(db_int8) }, 
        { sizeof(real4), sizeof(real4) }, 
        { sizeof(real8), sizeof(real8) }, 
        { sizeof(dbVarying), 4 }, 
        { sizeof(oid_t), sizeof(oid_t) }, 
        { sizeof(dbVarying), 4 },
        {0}, // tpMethodBool,
        {0}, // tpMethodInt1,
        {0}, // tpMethodInt2,
        {0}, // tpMethodInt4,
        {0}, // tpMethodInt8,
        {0}, // tpMethodReal4,
        {0}, // tpMethodReal8,
        {0}, // tpMethodString,
        {0}, // tpMethodReference,
        {0}, // tpStructure,
        {0}, // tpRawBinary,
        {0}, // tpStdString,
        { sizeof(rectangle), sizeof(coord_t) }, // tpRectangle,
        {0} // tpUnknown
    };

    const int maxFields = 256;
    tableField fields[maxFields];
    int nFields = 0;
    int nColumns = 0;
    tkn = tkn_comma;
    while (tkn == tkn_comma) { 
        if (nFields+1 == maxFields) { 
            error("Too many fields");
            break;
        }
        if (!expect("field name", tkn_ident)) { 
            break;
        }
        int nameLen = strlen(buf)+1;
        fields[nFields].name = new char[nameLen];
        strcpy(fields[nFields].name, buf);
        varyingLength += nameLen + 2;
        char* refTableName;
        char* inverseRefName;
        int type = parseType(refTableName, inverseRefName);
        fields[nFields++].type = type;
        if (type == dbField::tpUnknown) { 
            break;
        }
        nColumns += 1;
        if (type == dbField::tpArray) {
            if (nFields+1 == maxFields) { 
                error("Too many fields");
                break;
            }
            fields[nFields].name = new char[nameLen+2];
            sprintf(fields[nFields].name, "%s[]", fields[nFields-1].name);
            varyingLength += nameLen+2+2;
            type = parseType(refTableName, inverseRefName);
            if (type == dbField::tpUnknown) { 
                break;
            }
            if (type == dbField::tpArray) { 
                error("Arrays of arrays are not supported by CLI");
                break;
            }
            if (type == dbField::tpReference) {
                fields[nFields].refTableName = refTableName;
                varyingLength += strlen(refTableName);
                if (inverseRefName != NULL) { 
                    fields[nFields-1].inverseRefName = inverseRefName;
                    varyingLength += strlen(inverseRefName);
                }                   
            }
            fields[nFields++].type = type;
        } else if (type == dbField::tpReference) { 
            fields[nFields-1].refTableName = refTableName;
            varyingLength += strlen(refTableName);
            if (inverseRefName != NULL) { 
                fields[nFields-1].inverseRefName = inverseRefName;
                varyingLength += strlen(inverseRefName);
            }                   
        }
        tkn = scan(); 
    }
    if (tkn == tkn_rpar) { 
        beginTransaction(dbExclusiveLock);
        dbTableDescriptor* oldDesc = findTable(name);
        if (oldDesc != NULL) {
            if (create) { 
                error("Table already exists");
                return false;
            }
        } else { 
            if (!create) { 
                error("Table not found");
                return false;
            }
        }
        dbTable* table; 
        oid_t oid;
        if (create) { 
            modified = true;
            oid = allocateRow(dbMetaTableId, 
                              sizeof(dbTable) + sizeof(dbField)*nFields + varyingLength);
            table = (dbTable*)getRow(oid);    
        } else { 
            oid = oldDesc->tableId;
            table = (dbTable*)new char[sizeof(dbTable) + sizeof(dbField)*nFields + varyingLength];
        }
        int offs = sizeof(dbTable) + sizeof(dbField)*nFields;
        table->name.offs = offs;
        table->name.size = strlen(name)+1;
        strcpy((char*)table + offs, name);
        offs += table->name.size;
        size_t size = sizeof(dbRecord);
        table->fields.offs = sizeof(dbTable);
        dbField* field = (dbField*)((char*)table + table->fields.offs);
        offs -= sizeof(dbTable);
        bool arrayComponent = false;

        for (int i = 0; i < nFields; i++) { 
            field->name.offs = offs;
            field->name.size = strlen(fields[i].name) + 1;
            strcpy((char*)field + offs, fields[i].name);
            offs += field->name.size;
            
            field->tableName.offs = offs;
            if (fields[i].refTableName) { 
                field->tableName.size = strlen(fields[i].refTableName) + 1;
                strcpy((char*)field + offs, fields[i].refTableName);
                offs += field->tableName.size;
            } else { 
                field->tableName.size = 1;
                *((char*)field + offs++) = '\0';
            }

            field->inverse.offs = offs;
            if (fields[i].inverseRefName) { 
                field->inverse.size = strlen(fields[i].inverseRefName) + 1;
                strcpy((char*)field + offs, fields[i].inverseRefName);
                offs += field->inverse.size;
            } else { 
                field->inverse.size = 1;
                *((char*)field + offs++) = '\0';
            }
    
            field->flags = 0;
            field->type = fields[i].type;
            field->size = typeDesc[fields[i].type].size;
            if (!arrayComponent) { 
                size = DOALIGN(size, typeDesc[fields[i].type].alignment);
                field->offset = size;
                size += field->size;
            } else { 
                field->offset = 0;
            }
            field->hashTable = 0;
            field->tTree = 0;
            arrayComponent = field->type == dbField::tpArray; 
            field += 1;
            offs -= sizeof(dbField);
        }
        table->fields.size = nFields;
        table->fixedSize = size;
        table->nRows = 0;
        table->nColumns = nColumns;
        table->firstRow = 0;
        table->lastRow = 0;

        if (create) { 
            linkTable(new dbTableDescriptor(table), oid);
        } else { 
            dbTableDescriptor* newDesc = new dbTableDescriptor(table);      
            delete[] (char*)table;
            dbTable* oldTable = (dbTable*)getRow(oid);
            if (!newDesc->equal(oldTable)) {
                bool saveConfirmDeleteColumns = confirmDeleteColumns; 
                confirmDeleteColumns = true;
                modified = true;
                schemeVersion += 1;
                unlinkTable(oldDesc);
                if (oldTable->nRows == 0) {
                    updateTableDescriptor(newDesc, oid);
                } else {
                    reformatTable(oid, newDesc);
                }
                delete oldDesc;
                confirmDeleteColumns = saveConfirmDeleteColumns;
            }
        }
        if (!completeDescriptorsInitialization()) { 
            warning("Reference to undefined table");
        }
    }
    return tkn == tkn_rpar;
}

int dbSubSql::parseType(char*& refTableName, char*& inverseFieldName)
{
    switch (scan()) { 
      case tkn_bool:
        return dbField::tpBool;
      case tkn_int1:
        return dbField::tpInt1;
      case tkn_int2:
        return dbField::tpInt2;
      case tkn_int4:
        return dbField::tpInt4;
      case tkn_int8:
        return dbField::tpInt8;
      case tkn_real4:
        return dbField::tpReal4;
      case tkn_real8:
        return dbField::tpReal8;
      case tkn_array:
        return expect("of", tkn_of) ? dbField::tpArray : dbField::tpUnknown;
      case tkn_string:
        return dbField::tpString;
      case tkn_reference:
        if (expect("to", tkn_to) && expect("referenced table name", tkn_ident)) {
            refTableName = new char[strlen(buf)+1];
            strcpy(refTableName, buf);
            int tkn = scan();
            if (tkn == tkn_inverse) {
                if (!expect("inverse reference field name", tkn_ident)) { 
                    return dbField::tpUnknown;
                }
                inverseFieldName = new char[strlen(buf)+1];
                strcpy(inverseFieldName, buf);
            } else { 
                inverseFieldName = NULL;
                ungetToken = tkn;
            }
            return dbField::tpReference;
        } else { 
            return dbField::tpUnknown;
        }
      case tkn_rectangle:
        return dbField::tpRectangle;
      default:
        error("Field type expected");
    }
    return dbField::tpUnknown;
}

int dbSubSql::readExpression()
{
    int i, ch;
    for (i = 0; (ch = get()) != ';' && ch != ',' && ch != EOF; i++) { 
        if (i+1 >= buflen) { 
            char* newbuf = new char[buflen*2];
            memcpy(newbuf, buf, buflen);
            delete[] buf;
            buf = newbuf;
            buflen *= 2;
        }
        buf[i] = ch;
    }
    buf[i] = '\0';
    return ch;
}

bool dbSubSql::readCondition()
{
    int i, ch;
    for (i = 0; (ch = get()) != ';' && ch !=  EOF; i++) { 
        if (i+1 >= buflen) { 
            char* newbuf = new char[buflen*2];
            memcpy(newbuf, buf, buflen);
            delete[] buf;
            buf = newbuf;
            buflen *= 2;
        }
        buf[i] = ch;
    }
    buf[i] = '\0';
    if (ch != ';') { 
        error("unexpected end of input");
        return false;
    } 
    return true;
}


void dbSubSql::dumpRecord(byte* base, dbFieldDescriptor* first)
{
    int i, n;
    byte* elem;
    dbFieldDescriptor* fd = first;
    do { 
        if (fd != first) { 
            printf(", ");
        }
        switch (fd->type) { 
          case dbField::tpBool:
            printf("%s", *(bool*)(base + fd->dbsOffs) 
                   ? "true" : "false");
            continue;
          case dbField::tpInt1:
            printf("%d", *(int1*)(base + fd->dbsOffs)); 
            continue;       
          case dbField::tpInt2:
            printf("%d", *(int2*)(base + fd->dbsOffs)); 
            continue;
          case dbField::tpInt4:
            printf("%d", *(int4*)(base + fd->dbsOffs)); 
            continue;
          case dbField::tpInt8:
            printf(INT8_FORMAT, *(db_int8*)(base + fd->dbsOffs)); 
            continue;
          case dbField::tpReal4:
            printf("%f", *(real4*)(base + fd->dbsOffs)); 
            continue;
          case dbField::tpReal8:
            printf("%f", *(real8*)(base + fd->dbsOffs)); 
            continue;
          case dbField::tpString:
            printf("'%s'", (char*)base+((dbVarying*)(base+fd->dbsOffs))->offs);
            continue;
          case dbField::tpReference:
            printf("#%lx", (unsigned long)*(oid_t*)(base + fd->dbsOffs)); 
            continue;
          case dbField::tpRectangle:
            {
                int i, sep = '(';
                rectangle& r = *(rectangle*)(base + fd->dbsOffs);
                for (i = 0; i < rectangle::dim*2; i++) { 
                    printf("%c%f", sep, (double)r.boundary[i]);
                    sep = ',';
                }
                printf(")");
            }
            continue;
          case dbField::tpRawBinary:
            n = fd->dbsSize;
            elem = base + fd->dbsOffs;
            printf("(");
            for (i = 0; i < n; i++) { 
                if (i != 0) { 
                    printf(", ");
                }
                printf("%02x", *elem++);
            }
            printf(")");
            continue;
          case dbField::tpArray:
            n = ((dbVarying*)(base + fd->dbsOffs))->size;
            elem = base + ((dbVarying*)(base + fd->dbsOffs))->offs;
            printf("(");
            for (i = 0; i < n; i++) { 
                if (i != 0) { 
                    printf(", ");
                }
                dumpRecord(elem, fd->components);
                elem += fd->components->dbsSize;
            }
            printf(")");
            continue;
          case dbField::tpStructure:
            if (dateFormat != NULL 
                && fd->components->next == fd->components 
                && strcmp(fd->components->name, "stamp") == 0) 
            { 
                char buf[64];
                printf(((dbDateTime*)(base + fd->components->dbsOffs))->asString(buf, sizeof buf, dateFormat));
                continue;
            }
            printf("(");
            dumpRecord(base, fd->components);
            printf(")");
        }
    } while ((fd = fd->next) != first);
}

int dbSubSql::calculateRecordSize(dbList* node, int offs, 
                                  dbFieldDescriptor* first)
{
    dbFieldDescriptor* fd = first;
    do { 
        if (node == NULL) { 
            return -1;
        }
        if (fd->type == dbField::tpArray) { 
            if (node->type != dbList::nTuple) { 
                return -1;
            }
            int nElems = node->aggregate.nComponents;
            offs = DOALIGN(offs, fd->components->alignment) 
                 + nElems*fd->components->dbsSize;
            if (fd->attr & dbFieldDescriptor::HasArrayComponents) {
                dbList* component = node->aggregate.components;
                while (--nElems >= 0) { 
                    int d = calculateRecordSize(component,offs,fd->components);
                    if (d < 0) return d;
                    offs = d;
                    component = component->next;
                }
            } 
        } else if (fd->type == dbField::tpString) { 
            if (node->type != dbList::nString) { 
                return -1;
            }
            offs += strlen(node->sval) + 1;
        } else if (fd->type == dbField::tpRectangle) {
            if (node->type != dbList::nTuple) { 
                return -1;
            }
            int nCoords = node->aggregate.nComponents;
            if (nCoords != rectangle::dim*2) {
                return -1;
            }
            dbList* component = node->aggregate.components;
            while (--nCoords >= 0) {
                if (component->type != dbList::nInteger && component->type != dbList::nReal) {
                    return -1;
                }
                component = component->next;
            }
        } else if (fd->type == dbField::tpRawBinary) { 
            if (node->type != dbList::nTuple) { 
                return -1;
            }
            int nElems = node->aggregate.nComponents;
            dbList* component = node->aggregate.components;
            if (size_t(nElems) > fd->dbsSize) { 
                return -1;
            }
            while (--nElems >= 0) { 
                if (component->type != dbList::nInteger
                    || (component->ival & ~0xFF) != 0) 
                { 
                    return -1;
                }
                component = component->next;
            }
#ifdef AUTOINCREMENT_SUPPORT
        } else if (node->type == dbList::nAutoinc) {        
            if (fd->type != dbField::tpInt4) {
                return -1;
            }
#endif
        } else { 
            if (!((node->type == dbList::nBool && fd->type == dbField::tpBool)
                  || (node->type == dbList::nInteger 
                      && (fd->type == dbField::tpInt1
                          || fd->type == dbField::tpInt2
                          || fd->type == dbField::tpInt4
                          || fd->type == dbField::tpInt8
                          || fd->type == dbField::tpReference))
                  || (node->type == dbList::nReal 
                      && (fd->type == dbField::tpReal4
                          || fd->type == dbField::tpReal8))
                  || (node->type == dbList::nTuple 
                      && fd->type == dbField::tpStructure)))
            {
                return -1;
            }
            if (fd->attr & dbFieldDescriptor::HasArrayComponents) {
                int d = calculateRecordSize(node->aggregate.components,
                                            offs, fd->components);
                if (d < 0) return d;
                offs = d;
            }
        }
        node = node->next;
    } while ((fd = fd->next) != first);
    return offs;    
}

bool dbSubSql::isValidOid(oid_t oid) 
{
    return oid == 0 || 
        (oid < currIndexSize && (currIndex[oid] & (dbFreeHandleMarker|dbInternalObjectMarker)) == 0);
}

int dbSubSql::initializeRecordFields(dbList* node, byte* dst, int offs, 
                                     dbFieldDescriptor* first)
{
    dbFieldDescriptor* fd = first;
    dbList* component;
    byte* elem;
    coord_t* coord;
    int len, elemOffs, elemSize;

    do { 
        if (node->type == dbList::nString && fd->type != dbField::tpString) { 
            char* s = node->sval;
            long  ival;
            switch (fd->type) {
              case dbField::tpBool:
                *(bool*)(dst+fd->dbsOffs) = *s == '1' || *s == 't' || *s == 'T';
                break;
              case dbField::tpInt1:
                if (sscanf(s, "%ld", &ival) != 1) { 
                    return -1;
                }
                *(int1*)(dst+fd->dbsOffs) = (int1)ival;
              case dbField::tpInt2:
                if (sscanf(s, "%ld", &ival) != 1) { 
                    return -1;
                }
                *(int2*)(dst+fd->dbsOffs) = (int2)ival;
              case dbField::tpInt4:
                if (sscanf(s, "%ld", &ival) != 1) { 
                    return -1;
                }
                *(int4*)(dst+fd->dbsOffs) = (int4)ival;
              case dbField::tpInt8:
                if (sscanf(s, "%ld", &ival) != 1) { 
                    return -1;
                }
                *(db_int8*)(dst+fd->dbsOffs) = ival;
                break;
              case dbField::tpReal4:
                if (sscanf(s, "%f", (real4*)(dst+fd->dbsOffs)) != 1) { 
                    return -1;
                }
                break;
              case dbField::tpReal8:
                if (sscanf(s, "%lf", (real8*)(dst+fd->dbsOffs)) != 1) { 
                    return -1;
                }
                break;
            }
#ifdef AUTOINCREMENT_SUPPORT
        } else if (node->type == dbList::nAutoinc) {        
            if (fd->type == dbField::tpInt4) {
                *(int4*)(dst+fd->dbsOffs) = fd->defTable->autoincrementCount;
            } else { 
                return -1;
            }
#endif
        } else { 
            switch (fd->type) { 
              case dbField::tpBool:
                *(bool*)(dst+fd->dbsOffs) = node->bval;
                break;
              case dbField::tpInt1:
                *(int1*)(dst+fd->dbsOffs) = (int1)node->ival;
                break;
              case dbField::tpInt2:
                *(int2*)(dst+fd->dbsOffs) = (int2)node->ival;
                break;
              case dbField::tpInt4:
                *(int4*)(dst+fd->dbsOffs) = (int4)node->ival;
                break;
              case dbField::tpInt8:
                *(db_int8*)(dst+fd->dbsOffs) = node->ival;
                break;
              case dbField::tpReal4:
                *(real4*)(dst+fd->dbsOffs) = (real4)node->fval;
                break;
              case dbField::tpReal8:
                *(real8*)(dst+fd->dbsOffs) = node->fval;
                break;
              case dbField::tpReference:
                if (isValidOid((oid_t)node->ival)) {                
                    *(oid_t*)(dst+fd->dbsOffs) = (oid_t)node->ival;
                } else { 
                    return -1;
                }
                break;
              case dbField::tpString:
                ((dbVarying*)(dst+fd->dbsOffs))->offs = offs;
                len = strlen(node->sval) + 1;
                ((dbVarying*)(dst+fd->dbsOffs))->size = len;
                memcpy(dst + offs, node->sval, len);
                offs += len;
                break;
              case dbField::tpRawBinary:
                len = node->aggregate.nComponents;
                component = node->aggregate.components;
                elem = dst + fd->dbsOffs;
                while (--len >= 0) { 
                    *elem++ = (byte)component->ival;
                    component = component->next;
                }
                break;
              case dbField::tpRectangle:
                len = node->aggregate.nComponents;
                component = node->aggregate.components;
                coord = (coord_t*)(dst + fd->dbsOffs);
                assert(len == rectangle::dim*2);                    
                while (--len >= 0) {
                    *coord++ = (component->type == dbList::nInteger) 
                        ? (coord_t)component->ival : (coord_t)component->fval;
                    component = component->next;
                }
                break;
              case dbField::tpArray:
                len = node->aggregate.nComponents;
                elem = (byte*)DOALIGN(long(dst) + offs, fd->components->alignment);
                offs = elem - dst;
                ((dbVarying*)(dst+fd->dbsOffs))->offs = offs;
                ((dbVarying*)(dst+fd->dbsOffs))->size = len;
                elemSize = fd->components->dbsSize;  
                elemOffs = len*elemSize;
                offs += elemOffs;
                component = node->aggregate.components;
                while (--len >= 0) { 
                    elemOffs = initializeRecordFields(component, elem, elemOffs, 
                                                      fd->components);
                    elemOffs -= elemSize;
                    elem += elemSize;
                    component = component->next;
                }
                offs += elemOffs;
                break;
              case dbField::tpStructure:
                offs = initializeRecordFields(node->aggregate.components, 
                                              dst, offs, fd->components);
            }
        }
        node = node->next;
    } while ((fd = fd->next) != first);

    return offs;
}


bool dbSubSql::insertRecord(dbList* list, dbTableDescriptor* desc)
{
    int size = calculateRecordSize(list, desc->fixedSize, desc->columns);
    if (size < 0) { 
        error("Incompatible types in insert statement");
        return false;
    }
    oid_t oid = allocateRow(desc->tableId, size);
    byte* dst = (byte*)getRow(oid);    
    dbTable* table = (dbTable*)getRow(desc->tableId);
#ifdef AUTOINCREMENT_SUPPORT
    desc->autoincrementCount = table->count;
#endif
    initializeRecordFields(list, dst, desc->fixedSize, desc->columns);

    int nRows = table->nRows;
    dbFieldDescriptor* fd;
    for (fd = desc->hashedFields; fd != NULL; fd = fd->nextHashedField){
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
    return true;
}

int dbSubSql::readValues(dbList** chain)
{
    int i, n = 0;
    int tkn;
    dbList* node;

    while (true) { 
        switch (scan()) { 
          case tkn_lpar:
            node = new dbList(dbList::nTuple);
            node->aggregate.components = NULL;
            i = readValues(&node->aggregate.components);
            if (i < 0) { 
                return -1;
            }
            node->aggregate.nComponents = i;
            break;
          case tkn_rpar:
            return -n; // valid only in case of empty list
          case tkn_iconst:
            node = new dbList(dbList::nInteger);
            node->ival = ival;
            break;
          case tkn_true:
            node = new dbList(dbList::nBool);
            node->bval = true;
            break;
          case tkn_false:
            node = new dbList(dbList::nBool);
            node->bval = false;
            break;          
          case tkn_fconst:
            node = new dbList(dbList::nReal);
            node->fval = fval;
            break;
          case tkn_sconst:
            node = new dbList(dbList::nString);
            node->sval = new char[strlen(buf)+1];
            strcpy(node->sval, buf);
            break;
          case tkn_autoincrement:
            node = new dbList(dbList::nAutoinc);
            break;
          case tkn_error:
            return -1;
          default:
            error("Syntax error in insert list");
            return -1;
        }
        *chain = node;
        chain = &node->next;
        n += 1;
        if ((tkn = scan()) == tkn_rpar) { 
            return n;
        }
        if (tkn != tkn_comma) {   
            error("',' expected");
            return -1;
        }
    }
}


dbFieldDescriptor* dbSubSql::readFieldName()
{
    int tkn;

    if (expect("table name", tkn_ident)) { 
        dbTableDescriptor* desc;
        dbFieldDescriptor* fd;
        if ((desc = findTable(name)) == NULL) { 
            error("No such table in database");
            return NULL;
        }       
        if (expect(".", tkn_dot) && expect("field name", tkn_ident)) { 
            if ((fd = desc->findSymbol(name)) == NULL) { 
                error("No such field in the table");
                return NULL;
            } else if (fd->type == dbField::tpArray) {  
                error("Array components can not be indexed");
                return NULL;
            }
        } else { 
            return NULL;
        }
        while ((tkn = scan()) != tkn_semi) { 
            if (tkn != tkn_dot) { 
                error("'.' expected");
                return NULL;
            }   
            if (expect("field name", tkn_ident)) { 
                if ((fd = fd->find(name)) == NULL) { 
                    error("No such field in the table");
                    return NULL;
                } else if (fd->type == dbField::tpArray) {      
                    error("Array components can not be indexed");
                    return NULL;
                }       
            } else { 
                return NULL;
            }
        }
        if (fd->type == dbField::tpStructure) { 
            error("Structures can not be indexed");
            return NULL;
        } 
        return fd;
    }
    return NULL;
}


bool dbSubSql::updateFields(dbAnyCursor* cursor, dbUpdateElement* elems)
{
     char buf[64], *src;
     dbInheritedAttribute iattr;
     dbSynthesizedAttribute sattr;
     iattr.db = this;
     iattr.oid = cursor->currId;
     iattr.table = (dbTable*)getRow(cursor->table->tableId);
     iattr.record = (byte*)getRow(cursor->currId);    
     iattr.paramBase = (size_t)cursor->paramBase;

     do { 
         dbExprNode* expr = elems->value;
         dbFieldDescriptor* fd = elems->field;
         execute(expr, iattr, sattr);
         byte* dst = cursor->record + fd->appOffs;

         switch (fd->type) {
           case dbField::tpBool:
             switch (expr->type) { 
               case tpInteger:
                 *(bool*)dst = sattr.ivalue != 0;
                 continue;
               case tpBoolean:
                 *(bool*)dst = sattr.bvalue;
                 continue;
               case tpReal:
                 *(bool*)dst = sattr.fvalue != 0;
                 continue;
               case tpString:
                 *(bool*)dst = *sattr.base == 'T' || *sattr.base == 't' || *sattr.base == '1';
                 continue;
             }
             break;
           case dbField::tpInt1:
             switch (expr->type) { 
               case tpInteger:
                 *(int1*)dst = (int1)sattr.ivalue;
                 continue;
               case tpBoolean:
                 *(int1*)dst = sattr.bvalue ? 1 : 0;
                 continue;
               case tpReal:
                 *(int1*)dst = (int1)sattr.fvalue;
                 continue;
               case tpString:
                 *(int1*)dst = (int1)atoi((char*)sattr.base);
                 continue;
             }
             break;
           case dbField::tpInt2:
             switch (expr->type) { 
               case tpInteger:
                 *(int2*)dst = (int2)sattr.ivalue;
                 continue;
               case tpBoolean:
                 *(int2*)dst = sattr.bvalue ? 1 : 0;
                 continue;
               case tpReal:
                 *(int2*)dst = (int2)sattr.fvalue;
                 continue;
               case tpString:
                 *(int2*)dst = (int2)atoi((char*)sattr.base);
                 continue;
             }
             break;
           case dbField::tpInt4:
             switch (expr->type) { 
               case tpInteger:
                 *(int4*)dst = (int4)sattr.ivalue;
                 continue;
               case tpBoolean:
                 *(int4*)dst = sattr.bvalue ? 1 : 0;
                 continue;
               case tpReal:
                 *(int4*)dst = (int4)sattr.fvalue;
                 continue;
               case tpString:
                 *(int4*)dst = (int1)atoi((char*)sattr.base);
                 continue;
             }
             break;
           case dbField::tpInt8:
             switch (expr->type) { 
               case tpInteger:
                 *(db_int8*)dst = sattr.ivalue;
                 continue;
               case tpBoolean:
                 *(db_int8*)dst = sattr.bvalue ? 1 : 0;
                 continue;
               case tpReal:
                 *(db_int8*)dst = (db_int8)sattr.fvalue;
                 continue;
               case tpString:
                 *(db_int8*)dst = (db_int8)atoi((char*)sattr.base);
                 continue;
             }
             break;
           case dbField::tpReal4:
             switch (expr->type) { 
               case tpInteger:
                 *(real4*)dst = (real4)sattr.ivalue;
                 continue;
               case tpBoolean:
                 *(real4*)dst = (real4)(sattr.bvalue ? 1 : 0);
                 continue;
               case tpReal:
                 *(real4*)dst = (real4)sattr.fvalue;
                 continue;
               case tpString:
                 *(real4*)dst = (real4)atof((char*)sattr.base);
                 continue;
             }
             break;
           case dbField::tpReal8:
             switch (expr->type) { 
               case tpInteger:
                 *(real8*)dst = (real8)sattr.ivalue;
                 continue;
               case tpBoolean:
                 *(real8*)dst = sattr.bvalue ? 1.0 : 0.0;
                 continue;
               case tpReal:
                 *(real8*)dst = sattr.fvalue;
                 continue;
               case tpString:
                 *(real8*)dst = atof((char*)sattr.base);
                 continue;
             }
             break;
           case dbField::tpString:
             src = buf;
             switch (expr->type) { 
               case tpInteger:
                 sprintf(buf, INT8_FORMAT, sattr.ivalue);
                 break;
               case tpBoolean:
                 strcpy(buf, sattr.bvalue ? "t" : "f");
                 break;
               case tpReal:
                 sprintf(buf, "%f", sattr.fvalue);
                 break;
               case tpString:
                 src = (char*)sattr.base;
                 break;
             }
             *(char**)dst = new char[strlen(src)+1];
             strcpy(*(char**)dst, src);
             elems->strValue = *(char**)dst;
             continue;
           case dbField::tpReference:
             if (expr->type == tpInteger) { 
                 *(oid_t*)dst = sattr.oid;
                 continue;
             }
         }
         assert(false);
     } while ((elems = elems->next) != NULL);

     return true;
}

void dbSubSql::deleteColumns(dbFieldDescriptor* columns)
{
    if (columns != NULL) { 
        dbFieldDescriptor *next, *fd = columns;
        do {
            next = fd->next;
            fd->type = dbField::tpUnknown;
            fd->longName = NULL;
            delete fd;
            fd = next;
        } while (next != columns);
    }                    
}

void dbSubSql::profile()
{
    printf("TABLES:\n");
    printf("   Fixed   Fields  Columns     Rows    Total  Aligned  TableName\n");
    printf("----------------------------------------------------------------\n");
    beginTransaction(dbSharedLock);
    for (dbTableDescriptor* desc=tables; desc != NULL; desc=desc->nextDbTable)
    { 
        dbRecord* rec;
        dbTable* table = (dbTable*)getRow(desc->tableId);
        size_t totalSize = 0;
        size_t totalAlignedSize = 0;
        for (oid_t oid = table->firstRow; oid != 0; oid = rec->next) {
            rec = getRow(oid);
            totalSize += rec->size;
            totalAlignedSize += DOALIGN(rec->size, dbAllocationQuantum);
        }            
        printf("%8ld %8ld %8ld %8ld %8ld %8ld  %s\n",
               (long)desc->fixedSize,(long)desc->nFields, (long)desc->nColumns, 
               (long)table->nRows,  (long)totalSize, (long)totalAlignedSize,
               desc->name);
    }
}        


bool dbSubSql::parse() 
{
    dbTableDescriptor* desc;
    dbFieldDescriptor* fd;
    int tkn;
    bool count, outputOid, compactify;
    dbFieldDescriptor* columns;

    line = 1;
    pos = 0;

    while (true) { 
        if (in == stdin) { 
            printf(prompt);
            tkn = scan();
            pos += strlen(prompt);
        } else { 
            tkn = scan();
        }           

        switch (tkn) { 
          case tkn_update:
            if (!opened) { 
                error("Database not opened");
                continue;
            }
            if (accessType == dbReadOnly) { 
                error("Operation is not possible in read-only mode");
                continue;
            }
            dotIsPartOfIdentifier = true;
            if (expect("table name", tkn_ident)) {
                if ((desc = findTable(name)) == NULL) { 
                    error("No such table in database");
                    continue;
                }
                if (!expect("set", tkn_set)) { 
                    continue;
                }

                dbDatabaseThreadContext* ctx = threadContext.get();
                byte *record = dbMalloc(desc->appSize);
                memset(record, 0, desc->appSize);
                ctx->interactive = true;
                ctx->catched = true;
                dbUpdateElement* elems = NULL;
                if (!expect("field name", tkn_ident)) { 
                    goto updateCleanup;
                }
        
#ifdef THROW_EXCEPTION_ON_ERROR
                try {
#else
                if (setjmp(ctx->unwind) == 0) {
#endif
                    
                    char* condition = NULL;
                    int startPos = pos;
                    while (true) { 
                        dbUpdateElement* elem = new dbUpdateElement;
                        dbFieldDescriptor* fd = desc->findSymbol(name);
                        if (fd == NULL) { 
                            error("No such field in the table");
                            goto updateCleanup;
                        }
                        if (fd->type > dbField::tpReference) { 
                            error("Field can not be updated");
                            goto updateCleanup;
                        }
                        elem->field = fd;
                        elem->next = elems;
                        elems = elem;
                        if (!expect("=", tkn_eq)) { 
                            goto updateCleanup;
                        }
                        startPos = pos;
                        int ch = readExpression();
                        if (ch == EOF) { 
                            error("unexpected end of input");
                            goto updateCleanup;
                        } 
                        condition = strstr(buf, "where");
                        if (condition != NULL) {
                            *condition = '\0';
                        }
                        dbExprNode* expr = ctx->compiler.compileExpression(desc, buf, startPos);
                        if (expr == NULL) { 
                            goto updateCleanup;
                        }
                        if (expr->type > tpString) { 
                            error("Invalid expression type");
                            goto updateCleanup;
                        }
                        elem->value = expr;
                        if (condition == NULL && ch == ',') { 
                            if (!expect("field name", tkn_ident)) { 
                                goto updateCleanup;
                            }
                        } else { 
                            break;
                        }
                    }
                    dbAnyCursor cursor(*desc, dbCursorForUpdate, record);
                    cursor.reset();
                        
                    if (condition != NULL) { 
                        query.pos = startPos + (condition - buf) + 5;
                        query = (char const*)(condition + 5);
                        select(&cursor, query);
                        if (query.compileError()) { 
                            goto updateCleanup;
                        }                                   
                    } else { 
                        select(&cursor);
                    }
                    if (cursor.gotoFirst()) { 
                        do { 
                            cursor.fetch();
                            if (!updateFields(&cursor, elems)) { 
                                goto updateCleanup;
                            }   
                            cursor.update();
                        } while (cursor.gotoNext());
                    }
                    printf("\n\t%d records updated\n", cursor.getNumberOfRecords());
#ifdef THROW_EXCEPTION_ON_ERROR
                } catch(dbException const&) {}
#else
                } else { 
                    if (query.mutexLocked) { 
                        query.mutexLocked = false;
                        query.mutex.unlock();
                    }
                }
#endif
              updateCleanup:
                query.reset();
                while (elems != NULL) { 
                    dbUpdateElement* elem = elems;
                    elems = elems->next;
                    delete elem;
                }
                dbExprNodeAllocator::instance.reset();
                ctx->catched = false;
                dbFree(record);
            }
            break;

          case tkn_select:
            if (!opened) { 
                error("Database not opened");
                continue;
            }
            outputOid = true;
            count = false;
            if ((tkn = scan()) == tkn_all) { 
                outputOid = false;
                tkn = scan();
            } else if (tkn == tkn_count) { 
                if (!expect("'('", tkn_lpar)
                    || !expect("'*'", tkn_all)
                    || !expect("')'", tkn_rpar))
                {
                    continue;
                }
                count = true;
                tkn = scan();
            }
            columns = NULL;
            if (tkn != tkn_from) {
                while (true) { 
                    if (tkn != tkn_ident) { 
                        error("Field name or 'from' expected");
                    }
                    dbFieldDescriptor* column = new dbFieldDescriptor(name);
                    if (columns != NULL) { 
                        column->next = columns;
                        column->prev = columns->prev;
                        column->prev->next = column;
                        columns->prev = column;
                    } else { 
                        columns = column;
                        column->prev = column->next = column;
                    }
                    tkn = scan();
                    if (tkn != tkn_comma) { 
                        break;
                    }
                    tkn = scan();
                }
            }
            if (tkn != tkn_from) {
                deleteColumns(columns);
                error("FROM expected");
                continue;
            }
            dotIsPartOfIdentifier = true;
            if (scan() != tkn_ident) {
                deleteColumns(columns);
                error("Table name expected");
                continue;
            }
            if ((desc = findTable(name)) != NULL) {     
                dbAnyCursor cursor(*desc, dbCursorViewOnly, NULL);
                query.pos = pos;
                dbDatabaseThreadContext* ctx = threadContext.get();
                ctx->interactive = true;
                ctx->catched = true;
#ifdef THROW_EXCEPTION_ON_ERROR
                try {
#else
                if (setjmp(ctx->unwind) == 0) {
#endif
                    if (readCondition()) { 
                        query = (char const*)buf;
                        cursor.reset();
                        select(&cursor, query);
                        if (query.compileError()) { 
                            deleteColumns(columns);
                            dbExprNodeAllocator::instance.reset();
                            ctx->catched = false;
                            break;
                        }                
                    } else { 
                        deleteColumns(columns);
                        ctx->catched = false;
                        break;
                    }
                    if (count) { 
                        printf("%d records selected\n",
                               cursor.getNumberOfRecords());
                    } else { 
                        if (cursor.gotoFirst()) { 
                            dbFieldDescriptor* columnList;
                            if (columns != NULL) { 
                                columnList = columns;
                                dbFieldDescriptor* cc = columns; 
                                do { 
                                    dbFieldDescriptor* next = cc->next;
                                    dbFieldDescriptor* fd = desc->columns;
                                    do { 
                                        if (cc->name == fd->name) { 
                                            *cc = *fd;
                                            cc->next = next;
                                            goto Found;
                                        }
                                    } while ((fd = fd->next) != desc->columns);                                
                                    char buf[256];
                                    sprintf(buf, "Column '%s' is not found\n", cc->name);
                                    error(buf);
                                  Found:
                                    printf("%s ", fd->name);
                                    cc = next;
                                } while (cc != columns);
                            } else {                  
                                columnList = desc->columns;
                                dbFieldDescriptor* fd = columnList;
                                do { 
                                    printf("%s ", fd->name);
                                } while ((fd = fd->next) != columnList);
                            }
                            if (outputOid) { 
                                printf("\n#%lx: (", (unsigned long)cursor.currId);
                            } else { 
                                printf("\n(");
                            }
                            dumpRecord((byte*)getRow(cursor.currId), columnList); 
                            printf(")");
                            while (cursor.gotoNext()) { 
                                if (outputOid) { 
                                    printf(",\n#%lx: (", (unsigned long)cursor.currId);
                                } else { 
                                    printf(",\n(");
                                }
                                dumpRecord((byte*)getRow(cursor.currId), columnList); 
                                printf(")");
                            }
                            printf("\n\t%d records selected\n", 
                                   cursor.getNumberOfRecords());
                        } else { 
                            fprintf(stderr, "No records selected\n");
                        }
                    }
#ifdef THROW_EXCEPTION_ON_ERROR
                } catch(dbException const&) {}
#else
                } else { 
                    if (query.mutexLocked) { 
                        query.mutexLocked = false;
                        query.mutex.unlock();
                    }
                }
#endif
                deleteColumns(columns); 
                ctx->catched = false;
            } else { 
                error("No such table in database");
            }
            break;
                
          case tkn_open:
            if (expect("database name", tkn_sconst)) { 
                char* databaseName = new char[strlen(buf)+1];
                strcpy(databaseName, buf);
                char* fileName = NULL;
                if ((tkn = scan()) == tkn_sconst) { 
                    fileName = buf;
                } else if (tkn != tkn_semi) { 
                    delete[] databaseName;
                    error("database file name expected");
                    continue;
                }
                if (opened) { 
                    close();
                    while (droppedTables != NULL) { 
                        dbTableDescriptor* next = droppedTables->nextDbTable;
                        delete droppedTables; 
                        droppedTables = next;
                    }
                    opened = false;
                    dbTableDescriptor::cleanup();
                }
                time_t transactionCommitDelay = 0;
                char* delay = getenv("FASTDB_COMMIT_DELAY");
                if (delay != NULL) { 
                    transactionCommitDelay = atoi(delay);
                }
                if (!open(databaseName, fileName, INFINITE, transactionCommitDelay)) { 
                    fprintf(stderr, "Database not opened\n");
                } else { 
                    opened = true;
                    loadMetaTable();
                    existedTables = tables;
                    char* backupName = getenv("FASTDB_BACKUP_NAME");
                    if (backupName != NULL) { 
                        char* backupPeriod = getenv("FASTDB_BACKUP_PERIOD");
                        time_t period = 60*60*24; // one day
                        if (backupPeriod != NULL) { 
                            period = atoi(backupPeriod);
                        }
                        printf("Schedule backup to file %s each %u seconds\n", 
                               backupName, (unsigned)period);   
                        scheduleBackup(backupName, period);                     
                    }
                }
                delete[] databaseName;
            }
            break;
                    
          case tkn_drop:
            if (!opened) { 
                error("Database not opened");
                continue;
            }
            if (accessType == dbReadOnly) { 
                error("Operation is not possible in read-only mode");
                continue;
            }
            if (monitor->users != 1) { 
                error("Can not perform operation with active appliations");
                continue;
            }
            switch (scan()) { 
              case tkn_table:
                dotIsPartOfIdentifier = true;
                if (expect("table name", tkn_ident)) { 
                    desc = findTable(name);
                    if (desc == NULL) { 
                        error("No such table in database");
                    } else { 
                        dropTable(desc);
                        if (desc == existedTables) { 
                            existedTables = desc->nextDbTable;
                        }
                        unlinkTable(desc);
                        desc->nextDbTable = droppedTables;
                        droppedTables = desc;
                    }
                }
                break;
              case tkn_hash:
                fd = readFieldName();
                if (fd != NULL) { 
                    if (fd->hashTable == 0) { 
                        error("This field is not hashed");
                    } else { 
                        dropHashTable(fd);
                    }
                }
                break;
              case tkn_index:
                fd = readFieldName();
                if (fd != NULL) { 
                    if (fd->tTree == 0) { 
                        error("There is no index for this field");
                    } else { 
                        dropIndex(fd);
                    }
                }
                break;
              default:
                error("Expecting 'table', 'hash' or 'index' keyword");
                continue;
            }
            break;

          case tkn_backup:
            if (!opened) { 
                error("Database not opened");
                continue;
            }
            compactify = false;
            if ((tkn = scan()) == tkn_compactify) {
                compactify = true;
                tkn = scan();
            }
            if (tkn != tkn_sconst) { 
                 error("Backup file name expected");
            } else { 
                if (!backup(buf, compactify)) { 
                    printf("Backup failed\n");
                } else { 
                    while (droppedTables != NULL) { 
                        dbTableDescriptor* next = droppedTables->nextDbTable;
                        delete droppedTables; 
                        droppedTables = next;
                    }
                    commit();
                    existedTables = tables;
                }
            }
            continue;
            
          case tkn_create:
            if (!opened) { 
                error("Database not opened");
                continue;
            }
            if (accessType == dbReadOnly) { 
                error("Operation is not possible in read-only mode");
                continue;
            }
            if (monitor->users != 1) { 
                error("Can not perform operation with active appliations\n");
                continue;
            }
            switch (scan()) { 
              case tkn_hash:
                if (!expect("on", tkn_on)) { 
                    continue;
                }
                fd = readFieldName();
                if (fd != NULL) { 
                    if (fd->hashTable != 0) { 
                        error("This field is already hashed");
                    } else { 
                        createHashTable(fd);
                    }
                }
                break;
              case tkn_index:
                if (!expect("on", tkn_on)) { 
                    continue;
                }
                fd = readFieldName();
                if (fd != NULL) { 
                    if (fd->tTree != 0) { 
                        error("Index already exists");
                    } else { 
                        createIndex(fd);
                    }
                }
                break;
                
              case tkn_table:
                updateTable(true);
                break;

              default:
                error("Expecting 'table', 'hash' or 'index' keyword");
                continue;
            }
            break;

          case tkn_alter:
            if (!opened) {
                error("Database not opened");
                continue;
            }
            if (accessType == dbReadOnly) { 
                error("Operation is not possible in read-only mode");
                continue;
            }
            if (monitor->users != 1) { 
                error("Can not perform operation with active appliations\n");
                continue;
            }
            switch (scan()) {
              case tkn_table:
                updateTable(false);
                break;
              default:
                error("Expecting 'table' keyword");
                continue;
            }
            break;

          case tkn_insert:
            if (!opened) { 
                error("Database not opened");
                continue;
            }
            if (accessType == dbReadOnly) { 
                error("Operation is not possible in read-only mode");
                continue;
            }
            if (expect("into", tkn_into)) {
                dotIsPartOfIdentifier = true;
                if (expect("table name", tkn_ident)) {
                    if ((desc = findTable(name)) == NULL) { 
                        error("No such table in database");
                        continue;
                    }
                    if (!expect("values", tkn_values)) { 
                        continue;
                    }
                    beginTransaction(dbExclusiveLock);
                    modified = true;
                    while (expect("(", tkn_lpar)) { 
                        dbList* list = NULL;
                        int n = readValues(&list);
                        if (n <= 0 || !insertRecord(list, desc)) { 
                            if (n == 0) { 
                                error("Empty fields list");
                            }
                            tkn = tkn_semi; // just avoid extra error messages
                        } else { 
                            tkn = scan();
                        }
                        while (list != NULL) { 
                            dbList* tail = list->next;
                            delete list;
                            list = tail;
                        }
                        if (tkn == tkn_semi) { 
                            break;
                        } else if (tkn != tkn_comma) { 
                            error("';' or ',' expected");
                        }
                    }
                }
            }
            break;
          case tkn_delete:
            if (!opened) { 
                error("Database not opened");
                continue;
            }
            if (accessType == dbReadOnly) { 
                error("Operation is not possible in read-only mode");
                continue;
            }
            if (expect("FROM", tkn_from)) { 
                dotIsPartOfIdentifier = true;
                if (expect("table name", tkn_ident)) {
                    if ((desc = findTable(name)) == NULL) {
                        error("No such table in database");
                        continue;
                    } 
                    dbAnyCursor cursor(*desc, dbCursorForUpdate, NULL);
                    dbDatabaseThreadContext* ctx = threadContext.get();
                    ctx->interactive = true;
                    ctx->catched = true;
        
#ifdef THROW_EXCEPTION_ON_ERROR
                    try {
#else
                        if (setjmp(ctx->unwind) == 0) {
#endif
                            if (readCondition()) {
                                query = (char const*)buf;
                                cursor.reset();
                                select(&cursor, query);
                                if (query.compileError()) {
                                    dbExprNodeAllocator::instance.reset();
                                    ctx->catched = false;
                                    break;
                                }
                            } else {
                                ctx->catched = false;
                                break;
                            }
                            int n_deleted = cursor.getNumberOfRecords();
                            cursor.removeAllSelected();
                            printf("\n\t%d records deleted\n", n_deleted);
#ifdef THROW_EXCEPTION_ON_ERROR
                        } catch(dbException const&) {}
#else
                    } else { 
                        if (query.mutexLocked) { 
                            query.mutexLocked = false;
                            query.mutex.unlock();
                        }
                    }
#endif
                    ctx->catched = false;
                }
            }
            break;

          case tkn_commit:
            if (!opened) { 
                error("Database not opened");
            } else { 
                while (droppedTables != NULL) { 
                    dbTableDescriptor* next = droppedTables->nextDbTable;
                    delete droppedTables; 
                    droppedTables = next;
                }
                commit();
                existedTables = tables;
            }
            continue;

          case tkn_rollback:
            if (!opened) { 
                error("Database not opened");
            } else { 
                while (droppedTables != NULL) { 
                    dbTableDescriptor* next = droppedTables->nextDbTable;
                    linkTable(droppedTables, droppedTables->tableId); 
                    droppedTables = next;
                }
                rollback();
                while (tables != existedTables) { 
                    dbTableDescriptor* table = tables;
                    unlinkTable(table);
                    delete table;
                }
            } 
            continue;

          case tkn_memory:
            if (!opened) { 
                error("Database not opened");
            } else { 
                dbMemoryStatistic stat;
                beginTransaction(dbSharedLock);
                getMemoryStatistic(stat);
                printf("Used memory: %ld\nFree memory: %ld\nNumber of holes: %ld\nMaximal hole size: %ld\nMinimal hole size: %ld\nAverage hole size: %ld\n\n", 
                       (long)stat.used,
                       (long)stat.free,
                       (long)stat.nHoles,
                       (long)stat.maxHoleSize,
                       (long)stat.minHoleSize,
                       (long)(stat.nHoles != 0 ? (stat.free / stat.nHoles) : 0));
                for (int i = 0; i < dbDatabaseOffsetBits; i++) { 
                    if (stat.nHolesOfSize[i] != 0) { 
                        printf("Number of holes of size [%ld...%ld): %ld\n", 1L << i, 1L << (i+1), (long)stat.nHolesOfSize[i]);
                    }
                }
            }
            break;

          case tkn_profile:
            if (!opened) { 
                error("Database not opened");
            } else { 
                profile();
            }
            break;

          case tkn_show:
            if (!opened) { 
                error("Database not opened");
            } else { 
                beginTransaction(dbSharedLock);
                printf("FastDB   version  :  %d.%02d\n"
                       "Database version  :  %d.%02d\n"
                       "Database file size: %lu Kb\n"
                       "Object index size : %lu handles\n"
                       "Used part of index: %lu handles\n"
                       "Number of users   : %d\n"
                       "Number of readers : %d\n" 
                       "Number of writers : %d\n" 
                       "Number of blocked readers : %d\n" 
                       "Number of blocked writers : %d\n", 
                       FASTDB_MAJOR_VERSION, FASTDB_MINOR_VERSION, 
                       header->majorVersion, header->minorVersion, 
                       (unsigned long)(header->size/1024),
                       (unsigned long)header->root[1-header->curr].indexSize, 
                       (unsigned long)header->root[1-header->curr].indexUsed,
                       monitor->users,
                       monitor->nReaders,
                       monitor->nWriters,
                       monitor->nWaitReaders,
                       monitor->nWaitWriters + monitor->waitForUpgrade);
                printf("\nTABLES:\n");
                printf("OID       FixedSize   Fields  Columns  TableName\n");
                printf("---------------------------------------------------------\n");
                for (dbTableDescriptor* desc=tables; desc != NULL; desc=desc->nextDbTable)
                { 
                    printf("0x%06x  %8ld %8ld %8ld   %s\n",
                           (int)desc->tableId, (long)desc->fixedSize,
                           (long)desc->nFields, (long)desc->nColumns, desc->name);
                }
            } 
            break;
                       
          case tkn_describe:
            if (!opened) {
                error("Database not opened");
                continue;
            }   
            dotIsPartOfIdentifier = true;
            if (expect("table name", tkn_ident)) {
                if ((desc = findTable(name)) == NULL) {
                    error("No such table in database");
                    continue;
                }
                printf("\nOID=0x%06x, TableName=%s\n", (int)desc->tableId, desc->name);
                printf("No Index FieldType        RefTableName     FieldName        InverseFieldName Flg\n");
                printf("--------------------------------------------------------------------------------\n");
                dbFieldDescriptor* fd = desc->columns; 
                for (int i = desc->nColumns; --i >= 0;) { 
                    printf("%-2d %-5s %-16s %-16s %-16s %-16s %x\n", 
                           fd->fieldNo,
                           fd->hashTable != 0 && fd->tTree != 0 ? "H+T"
                           : fd->hashTable != 0 ? "H" : fd->tTree != 0 ? "T" : "-",
                           typeMnem[fd->type],
                           fd->refTableName != NULL
                               ? fd->refTableName 
                               : (fd->type == dbField::tpArray && fd->components->refTableName != NULL)
                                  ? fd->components->refTableName
                                  : "(null)", 
                           fd->name, 
                           (fd->inverseRefName != NULL ? fd->inverseRefName : "(null)"),
                           fd->indexType);
                    fd = fd->next;
                }
            }
            continue;

          case tkn_export:
            if (!opened) {
                error("Database not opened");
                continue;
            }   
            if (expect("xml file name", tkn_sconst)) { 
                FILE* f;
                if (strcmp(buf, "-") == 0) { 
                    f = stdout;
                } else { 
                    f = fopen(buf, "w");
                }
                if (f != NULL) { 
                    exportDatabase(f);
                    fclose(f);
                } else { 
                    error("Failed to open output file");
                }
            }
            break;

          case tkn_import:
            if (!opened) {
                error("Database not opened");
                continue;
            }   
            if (accessType == dbReadOnly) { 
                error("Operation is not possible in read-only mode");
                continue;
            }
            if (expect("xml file name", tkn_sconst)) { 
                FILE* f;
                if (strcmp(buf, "-") == 0) { 
                    f = stdin;
                } else { 
                    f = fopen(buf, "r");
                }
                if (f != NULL) { 
                    if (!importDatabase(f)) { 
                        error("Import from XML file failed: incorrect file format");
                    }
                    fclose(f);
                } else { 
                    error("Failed to open input file");
                }
            }
            break;

          case tkn_autocommit:
            switch (scan()) {
              case tkn_on:
                autocommit = true;
                break;
               case tkn_off:
                autocommit = false;
                break;
              default:
                error("ON or OFF expected");
            }
            continue;

          case tkn_help:
            fprintf(stderr, "SubSQL commands:\n\n\
open 'database-name' ( 'database-file-name' ) ';'\n\
select ('*') from <table-name> where <condition> ';'\n\
update <table-name> set <field-name> '=' <expression> {',' <field-name> '=' <expression>} where <condition> ';'\n\
create table <table-name> '('<field-name> <field-type> {',' <field-name> <field-type>}')' ';' \n\
alter table <table-name> '('<field-name> <field-type> {',' <field-name> <field-type>}')' ';' \n\
delete from <table-name>\n\
drop table <table-name>\n\
drop index <table-name> {'.' <field-name>} ';'\n\
create index on <table-name> {'.' <field-name>} ';'\n\
drop hash <table-name> {'.' <field-name>};\n\
create hash on <table-name> {'.' <field-name>}field> ';'\n\
insert into <table-name> values '(' <value>{',' <value>} ')' ';'\n\
backup [compactify] 'file-name'\n\
start server URL number-of-threads\n\
stop server URL\n\
start http server URL\n\
stop http server\n\
describe <table-name>\n\
import 'xml-file-name'\n\
export 'xml-file-name'\n\
show\n\
profile\n\
commit\n\
rollback\n\
autocommit (on|off)\n\
exit\n\
help\n\n");
            continue;
          case tkn_start:
            if (!opened) { 
                error("Database not opened");
            } else { 
                commit(); // allow server threads to process
                existedTables = tables;
                tkn = scan();
                if (tkn == tkn_http) { 
                    if (expect("server", tkn_server)
                        && expect("HTTP server URL", tkn_sconst))
                    {
#if !THREADS_SUPPORTED
                        error("Database was build without pthread support");
#else
                        startHttpServer(buf);
#endif
                    }
                } else if (tkn == tkn_server && expect("server URL", tkn_sconst)) { 
#if !THREADS_SUPPORTED
                    error("Database was build without pthread support");
#else
                    dbServer* server = dbServer::find(buf);
                    if (server == NULL) { 
                        char* serverURL = new char[strlen(buf)+1];
                        strcpy(serverURL, buf);
                        if (expect("number of threads", tkn_iconst)) { 
                            server = new dbServer(this, serverURL, (int)ival);
                            printf("Server started for URL %s\n", serverURL);
                        }
                        delete[] serverURL;
                    }
                    if (server != NULL) { 
                        server->start();
                    }
#endif
                } else { 
                    error("Token 'server' expected");
                }
            }
            continue;
          case tkn_stop:
            tkn = scan();
            if (tkn == tkn_http) { 
                if (expect("server", tkn_server) && expect("HTTP server URL", tkn_sconst))
                {
#if !THREADS_SUPPORTED
                    error("Database was build without pthread support");
#else
                    stopHttpServer(buf);
#endif
                }
            } else if (tkn == tkn_server) { 
                if (expect("server URL", tkn_sconst))
                {
#if !THREADS_SUPPORTED
                    error("Database was build without pthread support");
#else
                    dbServer* server = dbServer::find(buf);
                    if (server != NULL) { 
                        server->stop();
                        printf("Server stopped for URL %s\n", buf);
                    } else { 
                        fprintf(stderr, "No server was started for URL %s\n", buf);
                    }
#endif
                }
            } else { 
                error("Token 'server' expected");
            }
            continue;
          case tkn_semi:
            putchar('\n');
            // no break
          case tkn_error:
            continue;
          case tkn_exit:
            return false;
          case tkn_version:
            printf("FastDB version %d.%02d\n", FASTDB_MAJOR_VERSION, FASTDB_MINOR_VERSION);
            continue;
          case tkn_eof:
            return true;
          default:
            error("Unexpected token");
            continue;
        }
        if (autocommit || !modified) { 
            while (droppedTables != NULL) { 
                dbTableDescriptor* next = droppedTables->nextDbTable;
                delete droppedTables; 
                droppedTables = next;
            }
            commit();
            existedTables = tables;
        }
    }
}

            

void exportString(FILE* out, char* src, int len)
{
    fprintf(out, "\"");
    while (--len > 0) { 
        byte b = (byte)*src++;
        switch (b) { 
          case '&':
            fprintf(out, "&amp;");
            break;
          case '<':
            fprintf(out, "&lt;");
            break;
          case '>':
            fprintf(out, "&gt;");
            break;
          case '"':
            fprintf(out, "&quot;");
            break;
          case '\'':
            fprintf(out, "&apos;");
            break;
          default:
            fprintf(out, "%c", b);
        }
    }
    fprintf(out, "\"");
}

void exportBinary(FILE* out, byte* src, int len)
{
    fprintf(out, "\"");
    while (--len >= 0) { 
        fprintf(out, "%02X", *src++);
    }
    fprintf(out, "\"");
}

void exportRecord(dbFieldDescriptor* fieldList, FILE* out, byte* src, int indent)
{
    int i;
    dbFieldDescriptor* fd = fieldList;
    do {
        byte* ptr = src + fd->dbsOffs;
        for (i = indent; --i >= 0;) { 
            fprintf(out, " ");
        }
        char* fieldName = fd->name;
        if (strcmp(fieldName, "[]") == 0) { 
            fieldName = "array-element";
        }
        fprintf(out, "<%s>", fieldName);
        switch (fd->type) {
          case dbField::tpBool:
            fprintf(out, "%d", *(bool*)ptr);
            break;
          case dbField::tpInt1:
            fprintf(out, "%d", *(int1*)ptr);
            break;
          case dbField::tpInt2:
            fprintf(out, "%d", *(int2*)ptr);
            break;
          case dbField::tpInt4:
            fprintf(out, "%d", *(int4*)ptr);
            break;
          case dbField::tpInt8:
            fprintf(out, INT8_FORMAT, *(db_int8*)ptr);
            break;
          case dbField::tpReal4:
            fprintf(out, "%.8G", *(real4*)ptr);
            break;
          case dbField::tpReal8:
            fprintf(out, "%.16G", *(real8*)ptr);
            break;
          case dbField::tpRawBinary:
            exportBinary(out, src+fd->dbsOffs, fd->dbsSize);
            break;
          case dbField::tpString:
            exportString(out, (char*)(src + ((dbVarying*)ptr)->offs), ((dbVarying*)ptr)->size);
            break;
          case dbField::tpArray:
            {
                int nElems = ((dbVarying*)ptr)->size;
                byte* srcElem = src + ((dbVarying*)ptr)->offs;
                dbFieldDescriptor* element = fd->components;
                fprintf(out, "\n");
                while (--nElems >= 0) {
                    exportRecord(element, out, srcElem, indent+1);
                    srcElem += element->dbsSize;
                }
                for (i = indent; --i >= 0;) { 
                    fprintf(out, " ");
                }
                break;
            }
          case dbField::tpReference:
            fprintf(out, "<ref id=\"%ld\"/>", (long)*(oid_t*)ptr);
            break;
          case dbField::tpRectangle:
            { 
                rectangle& r = *(rectangle*)ptr;
                fprintf(out, "<rectangle><vertex");
                for (i = 0; i < rectangle::dim; i++) { 
                    fprintf(out, " c%d=\"%d\"", i, r.boundary[i]);
                }
                fprintf(out, "/><vertex");
                for (i = 0; i < rectangle::dim; i++) { 
                    fprintf(out, " c%d=\"%d\")", i, r.boundary[rectangle::dim+i]);
                }
                fprintf(out, "/></rectangle>");
            }
            break;
          case dbField::tpStructure:
            fprintf(out, "\n");
            exportRecord(fd->components, out, src, indent+1);
            for (i = indent; --i >= 0;) { 
                fprintf(out, " ");
            }
            break;
        }
        fprintf(out, "</%s>\n", fieldName);
    } while ((fd = fd->next) != fieldList);
}

void dbSubSql::exportScheme(FILE* out) 
{
    fprintf(out, "<!DOCTYPE database [\n");
    if (tables == NULL) { 
        fprintf(out, "<!ELEMENT database EMPTY>\n");
    } else { 
        fprintf(out, "<!ELEMENT database (%s*", tables->name);
        dbTableDescriptor* desc = tables; 
        while ((desc = desc->nextDbTable) != NULL) { 
            if (desc->tableId != dbMetaTableId) {
                fprintf(out, ", %s*", desc->name);
            }
        }
        fprintf(out, ")>\n");
    }
    for (dbTableDescriptor* desc = tables; desc != NULL; desc = desc->nextDbTable) { 
        if (desc->tableId != dbMetaTableId) {
            exportClass(out, desc->name, desc->columns);
            fprintf(out, "<!ATTLIST %s id CDATA #REQUIRED>\n", desc->name);
        }
    }
    fprintf(out, "<!ELEMENT array-element ANY>\n<!ELEMENT ref EMPTY>\n<!ATTLIST ref id CDATA #REQUIRED>\n]>\n");
}

void dbSubSql::exportClass(FILE* out, char* name, dbFieldDescriptor* fieldList) 
{
    fprintf(out, "<!ELEMENT %s (%s", name, fieldList->name);
    dbFieldDescriptor* fd = fieldList;
    while ((fd = fd->next) != fieldList) {
        fprintf(out, ", %s", fd->name);
    }
    fprintf(out, ")>\n");
    fd = fieldList;
    do { 
        switch (fd->type) {
          case dbField::tpStructure:
            exportClass(out, fd->name, fd->components);
            break;
          case dbField::tpArray:
            fprintf(out, "<!ELEMENT %s (array-element*)>\n", fd->name);
            if (fd->components->type == dbField::tpStructure) { 
                exportClass(out, fd->name, fd->components->components);
            }
            break;
          default:
            fprintf(out, "<!ELEMENT %s (#PCDATA)>\n", fd->name);
            
        } 
    } while ((fd = fd->next) != fieldList);
}


void dbSubSql::exportDatabase(FILE* out) 
{
    fprintf(out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    beginTransaction(dbSharedLock);
    exportScheme(out);
    fprintf(out, "<database>\n");
    for (dbTableDescriptor* desc=tables; desc != NULL; desc=desc->nextDbTable) { 
        if (desc->tableId != dbMetaTableId) {
            dbTable* table = (dbTable*)getRow(desc->tableId);    
            oid_t oid = table->firstRow; 
            int n = table->nRows;
            int percent = 0;
            for (int i = 0; oid != 0; i++) { 
                dbRecord* rec = getRow(oid);
                fprintf(out, " <%s id=\"%lu\">\n", desc->name, (unsigned long)oid); 
                exportRecord(desc->columns, out, (byte*)rec, 2);
                fprintf(out, " </%s>\n", desc->name); 
                oid = rec->next;
                int p = (i+1)*100/n;
                if (p != percent) { 
                    fprintf(stderr, "Exporting table %s: %d%%\r", desc->name, p);
                    fflush(stderr);
                    percent = p;
                }
            }
            fprintf(stderr, "Exporting table %s: 100%%   \n", desc->name);
        }
    }
    fprintf(out, "</database>\n");
}


inline int dbXmlScanner::get()
{
    int ch = getc(in);
    if (ch == '\n') {
        pos = 0;
        line += 1;
    } else if (ch == '\t') {
        pos = DOALIGN(pos + 1, 8);
    } else {
        pos += 1;
    }
    return ch;
}

inline void dbXmlScanner::unget(int ch) {
    if (ch != EOF) {
        if (ch != '\n') {
            pos -= 1;
        } else {
            line -= 1;
        }
        ungetc(ch, in);
    }
}


dbXmlScanner::token dbXmlScanner::scan()
{
    int ch, i, pos;
    bool floatingPoint;

  retry:
    do {
        if ((ch = get()) == EOF) {
            return xml_eof;
        }
    } while (ch <= ' ');
    
    switch (ch) { 
      case '<':
        ch = get();
        if (ch == '?') { 
            while ((ch = get()) != '?') { 
                if (ch == EOF) { 
                    return xml_error;
                }
            }
            if ((ch = get()) != '>') { 
                return xml_error;
            }
            goto retry;
        } else if (ch == '!') {
            int nParen = 1;
            do { 
                switch (get()) {
                  case '<':
                    nParen += 1;
                    break;
                  case '>':
                    nParen -= 1;
                    break;
                  case EOF:
                    return xml_error;
                }
            } while (nParen != 0);
            goto retry;
        } 
        if (ch != '/') { 
            unget(ch);
            return xml_lt;
        }
        return xml_lts;
      case '>':
        return xml_gt;
      case '/':
        ch = get();
        if (ch != '>') { 
            unget(ch);
            return xml_error;
        }
        return xml_gts;
      case '=':
        return xml_eq;
      case '"':
        i = 0;
        while (true) { 
            ch = get();
            switch (ch) { 
              case EOF:
                return xml_error;
              case '&':
                switch (get()) { 
                  case 'a':
                    ch = get();
                    if (ch == 'm' && get() == 'p' && get() == ';') { 
                        ch = '&';
                    } else if (ch == 'p' && get() == 'o' && get() == 's' && get() == ';') { 
                        ch = '\'';
                    } else { 
                        return xml_error;
                    }
                    break;
                  case 'l':
                    if (get() != 't' || get() != ';') { 
                        return xml_error;
                    }
                    ch = '<';
                    break;
                  case 'g':
                    if (get() != 't' || get() != ';') { 
                        return xml_error;
                    }
                    ch = '>';
                    break;
                  case 'q':
                    if (get() != 'u' || get() != 'o' || get() != 't' || get() != ';') { 
                        return xml_error;
                    }
                    ch = '"';
                    break;
                  default:
                    return xml_error;
                }
                break;
              case '"':
                slen = i;
                sconst[i] = 0;
                return xml_sconst;
            }
            if ((size_t)i+1 >= size) { 
                char* newBuf = new char[size *= 2];
                memcpy(newBuf, sconst, i);
                delete[] sconst;
                sconst = newBuf;
            }
            sconst[i++] = (char)ch;
        } 
      case '-': case '+':
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        i = 0;
        floatingPoint = false;
        while (true) { 
            if ((size_t)i == size) { 
                return xml_error;
            }
            if (!isdigit(ch) && ch != '-' && ch != '+' && ch != '.' && ch != 'E') { 
                unget(ch);
                sconst[i] = '\0';
                if (floatingPoint) { 
                   return sscanf(sconst, "%lf%n", &fconst, &pos) == 1 && pos == i
                       ? xml_fconst : xml_error;
                } else { 
                    return sscanf(sconst, INT8_FORMAT "%n", &iconst, &pos) == 1 && pos == i
                       ? xml_iconst : xml_error;
                }
            }
            sconst[i++] = (char)ch;
            if (ch == '.') { 
                floatingPoint = true;
            }
            ch = get();
        }
      default:
        i = 0;
        while (isalnum(ch) || ch == '-' || ch == ':' || ch == '_') { 
            if (i == MaxIdentSize) { 
                return xml_error;
            }
            ident[i++] = (char)ch;
            ch = get();
        }
        unget(ch);
        if (i == MaxIdentSize || i == 0) { 
            return xml_error;            
        }
        ident[i] = '\0';
        return xml_ident;
    }
}

#define EXPECT(x) scanner.expect(__LINE__, x)

bool skipElement(dbXmlScanner& scanner) 
{
    int depth = 1;
    do {  
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_lt:
            depth += 1;
            continue;
          case dbXmlScanner::xml_lts:
            depth -= 1;
            if (depth < 0 || !EXPECT(dbXmlScanner::xml_ident) || !EXPECT(dbXmlScanner::xml_gt))
            { 
                return false;
            }
            break;
          case dbXmlScanner::xml_gts:
            depth -= 1;
            break;
          default:
            continue;            
        }
    } while (depth != 0);

    return true;
}

bool dbSubSql::importRecord(char* terminator, dbFieldDescriptor* fieldList, byte* rec, dbXmlScanner& scanner) 
{
    dbXmlScanner::token tkn;

    while ((tkn = scanner.scan()) != dbXmlScanner::xml_lts) { 
        if (tkn != dbXmlScanner::xml_lt || !EXPECT(dbXmlScanner::xml_ident)
            || !EXPECT(dbXmlScanner::xml_gt)) 
        { 
            return false;
        }
        char* fieldName = scanner.getIdentifier();
        dbSymbolTable::add(fieldName, tkn_ident, FASTDB_CLONE_ANY_IDENTIFIER);
        dbFieldDescriptor* fd = fieldList;
        while (true) {
            if (fd->name == fieldName) {
                if (!importField(fd->name, fd, rec, scanner)) { 
                    return false;
                }
                break;
            }
            if ((fd = fd->next) == fieldList) { 
                if (!skipElement(scanner)) { 
                    return false;
                }   
                break;
            }
        } 
    }
    return EXPECT(terminator) && EXPECT(dbXmlScanner::xml_gt);
} 

#define HEX_DIGIT(ch) ((ch) >= 'a' ? ((ch) - 'a' + 10) : (ch) >= 'A' ? (((ch) - 'A' + 10)) : ((ch) - '0'))

bool dbSubSql::importField(char* terminator, dbFieldDescriptor* fd, byte* rec, dbXmlScanner& scanner) 
{
    dbXmlScanner::token tkn;
    int i;
    long id;
    byte* dst = rec + fd->appOffs;
    
    switch (fd->type) { 
      case dbField::tpStructure:
        return importRecord(terminator, fd->components, dst, scanner);
      case dbField::tpArray:
      { 
          int arrSize = 8;
          int elemSize = fd->components->appSize;
          byte* arr = (byte*)tmpAlloc.alloc(elemSize*arrSize);
          memset(arr, 0, elemSize*arrSize);
          for (i = 0; (tkn = scanner.scan()) == dbXmlScanner::xml_lt; i++) { 
              if (!EXPECT("array-element")
                  || !EXPECT(dbXmlScanner::xml_gt))
              {
                  return false;
              }
              if (i == arrSize) { 
                  arrSize *= 2;
                  byte* newArr = (byte*)tmpAlloc.alloc(elemSize*arrSize);
                  memcpy(newArr, arr, i*elemSize);
                  memset(newArr + i*elemSize, 0, i*elemSize);
                  arr = newArr;
              }
              importField("array-element", fd->components, arr + i*elemSize, scanner);
          }
          dbAnyArray::arrayAllocator((dbAnyArray*)dst, arr, i); 
          return tkn == dbXmlScanner::xml_lts
              && EXPECT(terminator)
              && EXPECT(dbXmlScanner::xml_gt);
      }
      case dbField::tpReference:
        if (!EXPECT(dbXmlScanner::xml_lt)
            || !EXPECT("ref")
            || !EXPECT("id")
            || !EXPECT(dbXmlScanner::xml_eq)
            || !EXPECT(dbXmlScanner::xml_sconst)
            || sscanf(scanner.getString(), "%ld", &id) != 1
            || !EXPECT(dbXmlScanner::xml_gts))
        { 
            return false;
        }
        *(oid_t*)dst = mapId(id);
        break;
      case dbField::tpRectangle:
        if (!EXPECT("rectangle")
            || !EXPECT(dbXmlScanner::xml_gt)
            || !EXPECT(dbXmlScanner::xml_lt)
            || !EXPECT("vertex"))
        { 
            return false;
        } else {
            rectangle& r = *(rectangle*)dst;
            for (i = 0; i < rectangle::dim; i++) { 
                if (!EXPECT(dbXmlScanner::xml_ident)
                    || !EXPECT(dbXmlScanner::xml_eq)
                    || !EXPECT(dbXmlScanner::xml_sconst)
                    || sscanf(scanner.getString(), "%d", &r.boundary[i]) != 1)
                {
                    return false;
                }
            }
            if (!EXPECT(dbXmlScanner::xml_gts)
                || !EXPECT(dbXmlScanner::xml_lt)
                || !EXPECT("vertex"))
            {
                return false;
            }
            for (i = 0; i < rectangle::dim; i++) { 
                if (!EXPECT(dbXmlScanner::xml_ident)
                    || !EXPECT(dbXmlScanner::xml_eq)
                    || !EXPECT(dbXmlScanner::xml_sconst)
                    || sscanf(scanner.getString(), "%d", &r.boundary[rectangle::dim+i]) != 1)
                {
                    return false;
                }
            }
            if (!EXPECT(dbXmlScanner::xml_gts)
                || !EXPECT(dbXmlScanner::xml_lts)
                || !EXPECT("rectangle")
                || !EXPECT(dbXmlScanner::xml_gt))
            {
                return false;
            }
            break;
        }
      case dbField::tpBool:
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_iconst:
            *(bool*)dst = scanner.getInt() != 0;
            break;
          case dbXmlScanner::xml_fconst:
            *(bool*)dst = scanner.getReal() != 0.0;
            break;
          default:
            fprintf(stderr, "Failed to convert field '%s'\n", fd->name);
        }
        break;            
      case dbField::tpInt1:
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_iconst:
            *(int1*)dst = (int1)scanner.getInt();
            break;
          case dbXmlScanner::xml_fconst:
            *(int1*)dst = (int1)scanner.getReal();
            break;
          default:
            fprintf(stderr, "Failed to convert field '%s'\n", fd->name);
        }
        break;            
      case dbField::tpInt2:
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_iconst:
            *(int2*)dst = (int2)scanner.getInt();
            break;
          case dbXmlScanner::xml_fconst:
            *(int2*)dst = (int2)scanner.getReal();
            break;
          default:
            fprintf(stderr, "Failed to convert field '%s'\n", fd->name);
        }
        break;            
      case dbField::tpInt4:
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_iconst:
            *(int4*)dst = (int4)scanner.getInt();
            break;
          case dbXmlScanner::xml_fconst:
            *(int4*)dst = (int4)scanner.getReal();
            break;
          default:
            fprintf(stderr, "Failed to convert field '%s'\n", fd->name);
        }
        break;            
      case dbField::tpInt8:
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_iconst:
            *(db_int8*)dst = scanner.getInt();
            break;
          case dbXmlScanner::xml_fconst:
            *(db_int8*)dst = (int8)scanner.getReal();
            break;
          default:
            fprintf(stderr, "Failed to convert field '%s'\n", fd->name);
        }
        break;            
      case dbField::tpReal4:
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_iconst:
            *(real4*)dst = (real4)scanner.getInt();
            break;
          case dbXmlScanner::xml_fconst:
            *(real4*)dst = (real4)scanner.getReal();
            break;
          default:
            fprintf(stderr, "Failed to convert field '%s'\n", fd->name);
        }
        break;            
      case dbField::tpReal8:
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_iconst:
            *(real8*)dst = (real8)scanner.getInt();
            break;
          case dbXmlScanner::xml_fconst:
            *(real8*)dst = scanner.getReal();
            break;
          default:
            fprintf(stderr, "Failed to convert field '%s'\n", fd->name);
        }
        break;            
      case dbField::tpString:
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_iconst:
          case dbXmlScanner::xml_sconst:
          case dbXmlScanner::xml_fconst:
          { 
              char* str = (char*)tmpAlloc.alloc((scanner.getStringLength()+1));
              memcpy(str, scanner.getString(), scanner.getStringLength());
              str[scanner.getStringLength()] = '\0';
              *(char**)dst = str;
              break;
          }
          default:
            fprintf(stderr, "Failed to convert field '%s'\n", fd->name);
        }
        break;            
      case dbField::tpRawBinary:
      {
          if (scanner.scan() != dbXmlScanner::xml_sconst) { 
              fprintf(stderr, "Failed to convert field '%s'\n", fd->name);
              break;            
          }
          char* src = scanner.getString();
          int len = scanner.getStringLength() >> 1;
          if (fd->appSize != (size_t)len) { 
              fprintf(stderr, "Length of raw binary field '%s' was changed\n", fd->name);
          } else { 
              while (--len >= 0) { 
                  *dst++ = (HEX_DIGIT(src[0]) << 4) | HEX_DIGIT(src[1]);
                  src += 2;
              }
          }
          break;
      }
#ifdef USE_STD_STRING
      case dbField::tpStdString:
          if (scanner.scan() != dbXmlScanner::xml_sconst) { 
              fprintf(stderr, "Failed to convert field '%s'\n", fd->name);
              break;            
          }
          ((std::string*)dst)->assign(scanner.getString(), scanner.getStringLength());
          break;
#endif
    }    
    return EXPECT(dbXmlScanner::xml_lts)
        && EXPECT(terminator)
        && EXPECT(dbXmlScanner::xml_gt); 
}

oid_t dbSubSql::mapId(long id)
{
    oid_t oid;
    if (id == 0) { 
        return 0;
    }
    if ((oid_t)id >= oidMapSize) { 
        oid_t* newOidMap = new oid_t[id*2];
        memcpy(newOidMap, oidMap, oidMapSize*sizeof(oid_t));
        memset(newOidMap + oidMapSize, 0, (id*2-oidMapSize)*sizeof(oid_t));
        oidMapSize = id*2;
        oidMap = newOidMap;
    }
    oid = oidMap[id];
    if (oid == 0) { 
        oid = allocateId();
        oidMap[id] = oid;
    }
    return oid;
}

void dbSubSql::insertRecord(dbTableDescriptor* desc, oid_t oid, void const* record) 
{
    dbFieldDescriptor* fd;
    byte* src = (byte*)record;
    size_t size = desc->columns->calculateRecordSize(src, desc->fixedSize);
    allocateRow(desc->tableId, oid, size);
    dbTable* table = (dbTable*)getRow(desc->tableId);
#ifdef AUTOINCREMENT_SUPPORT
    desc->autoincrementCount = table->count;
#endif
    dbRecord* dst = getRow(oid);
    desc->columns->storeRecordFields((byte*)dst, src, desc->fixedSize, dbFieldDescriptor::Import);
    
#ifdef AUTOINCREMENT_SUPPORT
    if (desc->autoincrementCount > table->count) {
        table->count = desc->autoincrementCount;
    }
#endif
    int nRows = table->nRows;
    for (fd = desc->hashedFields; fd != NULL; fd = fd->nextHashedField) {
        dbHashTable::insert(this, fd->hashTable, oid, fd->type, fd->dbsSize, fd->dbsOffs, nRows);
    }
    for (fd = desc->indexedFields; fd != NULL; fd = fd->nextIndexedField) {
        if (fd->type == dbField::tpRectangle) { 
            dbRtree::insert(this, fd->tTree, oid, fd->dbsOffs);
        } else { 
            dbTtree::insert(this, fd->tTree, oid, fd->type, fd->dbsSize, fd->comparator, fd->dbsOffs);
        }
    }
}

bool dbSubSql::importDatabase(FILE* in)
{
    dbXmlScanner scanner(in);
    dbXmlScanner::token tkn;
    
    if (!EXPECT(dbXmlScanner::xml_lt) ||
        !EXPECT("database") || 
        !EXPECT(dbXmlScanner::xml_gt))
    {
        return false;
    }
    beginTransaction(dbExclusiveLock);

    oidMapSize = dbDefaultInitIndexSize;
    oidMap = new oid_t[oidMapSize];
    memset(oidMap, 0, oidMapSize*sizeof(oid_t));

    while ((tkn = scanner.scan()) != dbXmlScanner::xml_lts) { 
        if (tkn != dbXmlScanner::xml_lt || !EXPECT(dbXmlScanner::xml_ident)) { 
            return false;
        }
        dbTableDescriptor* desc = findTableByName(scanner.getIdentifier());
        if (desc == NULL) { 
            fprintf(stderr, "Table '%s' not found\n", scanner.getIdentifier());
        }
        if (!EXPECT("id")
            || !EXPECT(dbXmlScanner::xml_eq)
            || !EXPECT(dbXmlScanner::xml_sconst)
            || !EXPECT(dbXmlScanner::xml_gt)) 
        {
            return false;
        }
        if (desc != NULL) { 
            long id;
            if (sscanf(scanner.getString(), "%ld", &id) != 1 || id == 0) { 
                return false;
            }
            oid_t oid = mapId(id);
            byte *record = (byte*)tmpAlloc.alloc(desc->appSize);  
            memset(record, 0, desc->appSize);
            if (!importRecord(desc->name, desc->columns, record, scanner)) {                 
                tmpAlloc.reset();
                return false;
            }
            insertRecord(desc, oid, record);
            tmpAlloc.reset();
        } else { // skip record
            if (!skipElement(scanner)) { 
                return false;
            }
        }    
    }
    return EXPECT("database") && EXPECT(dbXmlScanner::xml_gt);
}


void dbSubSql::handleError(dbErrorClass error, char const* msg, int arg)
{
    dbDatabaseThreadContext* ctx = threadContext.get();
    if (ctx == NULL || ctx->interactive) { 
        const int screenWidth = 80;
        int col;
        switch (error) { 
          case QueryError:
            col = arg % screenWidth;
            if (in == stdin) { 
                while (--col >= 0) putc('-', stderr);
                fprintf(stderr, "^\n%s\n", msg);
            } else { 
                fprintf(stderr, "%s at line %d position %d\n", msg, line, arg);
            }
            break;
          case ArithmeticError:
            fprintf(stderr, "%s\n", msg);
            break;
          case IndexOutOfRangeError:
            fprintf(stderr, "Index %d is out of range\n", arg);
            break;
          case NullReferenceError:
            fprintf(stderr, "Null object reference is accessed\n");
            break;
          case DatabaseOpenError:
            return;
          default:
            dbDatabase::handleError(error, msg, arg);
        }
        //
        // Recovery
        //
        if (in == stdin) { 
            int ch;
            while ((ch = get()) != '\n' && ch != EOF); 
        } else { 
            fseek(in, 0, SEEK_END);
        }
    }
#ifdef THROW_EXCEPTION_ON_ERROR
    if (msg == NULL) { 
        msg = errorMessage[error];
    }
    throw dbException(error, msg, arg);
#else
    if (ctx != NULL) { 
        if (ctx->catched) {     
            longjmp(ctx->unwind, error);
        } else { 
            abort();
        }
    }
#endif
}

void dbSubSql::run(int firstParam, int argc, char* argv[])
{
    for (int i = firstParam; i < argc; i++) { 
        if (strcmp(argv[i], "-") == 0) {
            break;
        } 
        in = fopen(argv[i], "r");
        if (in == NULL) { 
            fprintf(stderr, "Failed to open '%s' file\n", argv[i]);
        } else { 
            if (!parse()) { 
                if (opened) { 
                    close();
                }
#if THREADS_SUPPORTED
                dbServer::cleanup();
#endif
                return;
            }
        }
    }
    printf("SubSQL interactive utility for FastDB v. %d.%.02d\n"
           "Type 'help' for more information\n", 
           FASTDB_MAJOR_VERSION, FASTDB_MINOR_VERSION);
    in = stdin;
    parse();
    if (opened) { 
        close();
    }
#if THREADS_SUPPORTED
    dbServer::cleanup();
#endif
}

#define HTML_HEAD "Content-type: text/html\r\n\r\n\
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\"><HTML><HEAD>"

#define BODY "<BODY BGCOLOR=\"#c0c0c0\">"

#define EMPTY_LIST "<OPTION>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</OPTION></SELECT><BR>"


void httpQueryError(WWWconnection& con, char const* msg, char const* table)
{
    con << TAG <<
        HTML_HEAD "<TITLE>BUGDB error</TITLE></HEAD>"
        BODY
        "<CENTER><FONT SIZE=+2 COLOR=\"#FF0000\">"
        << msg << "</FONT></CENTER><P><FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=socket VALUE=\"" << con.getAddress() << "\">"
        "<INPUT TYPE=hidden NAME=table VALUE=\"" << table << "\">"
        "<INPUT TYPE=hidden NAME=page VALUE=queryPage>"
        "<CENTER><INPUT TYPE=submit VALUE=\"Ok\"></CENTER></FORM></BODY></HTML>";
}


void httpError(WWWconnection& con, char const* msg)
{
    con << TAG <<
        HTML_HEAD "<TITLE>BUGDB error</TITLE></HEAD>"
        BODY
        "<CENTER><FONT SIZE=+2 COLOR=\"#FF0000\">"
        << msg << "</FONT></CENTER><P><FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << "\">"
        "<INPUT TYPE=hidden NAME=\"page\" VALUE=defaultPage>"
        "<CENTER><INPUT TYPE=submit VALUE=\"Ok\"></CENTER></FORM></BODY></HTML>";
}


bool defaultPage(WWWconnection& con)
{
    ((dbSubSql*)con.userData)->defaultPage(con);
    return true;
}


void dbSubSql::defaultPage(WWWconnection& con)
{
    con << TAG <<
        HTML_HEAD "<TITLE>Database browser</TITLE></HEAD>"
        BODY
        "<TABLE><TR><TH align=left>Database name</TH><TD>" << databaseName << "</TD></TR>"
        "<TR><TH align=left>FastDB version</TH><TD>" << FASTDB_MAJOR_VERSION << "." << FASTDB_MINOR_VERSION << "</TD></TR>"
        "<TR><TH align=left>Database version</TH><TD>" << header->majorVersion << "." << header->minorVersion << "</TD></TR>"
        "<TR><TH align=left>Database size</TH><TD>" << (int)(getDatabaseSize() / 1024) << "Kb</TD></TR>"
        "</TABLE><P>"
        "<H2>Tables:</H2><FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=socket VALUE=\"" << con.getAddress() << "\">"
        "<INPUT TYPE=hidden NAME=page VALUE=queryPage>"
        "<SELECT SIZE=10 NAME=table>";
    if (tables != NULL && tables->nextDbTable != NULL) { 
        for (dbTableDescriptor* desc=tables; desc != NULL; desc=desc->nextDbTable)
        { 
            if (strcmp(desc->name, "Metatable") != 0) { 
                con << TAG << "<OPTION VALUE=\"" << desc->name << "\">" << desc->name << "</OPTION>";
            }
        }
    } else { 
        con << TAG << EMPTY_LIST;
    }
    con << TAG << "</SELECT><P><INPUT TYPE=submit VALUE=Query></FORM></BODY></HTML>";
}
    
bool queryPage(WWWconnection& con)
{
    ((dbSubSql*)con.userData)->queryPage(con);
    return true;
}

char const* indexType(dbFieldDescriptor* fd) { 
    return (fd->tTree != 0) 
        ? ((fd->hashTable != 0) ? "HT" : "T") 
        : ((fd->hashTable != 0) ? "H" : " ");
}

void dbSubSql::queryPage(WWWconnection& con)
{
    char* tableName = con.get("table");
    if (tableName == NULL) { 
        httpError(con, "Table not selected");
        return;
    }
    dbTableDescriptor* desc = findTableByName(tableName);
    if (desc == NULL) {
        httpError(con, "No such table");
        return;
    }
    char* history = con.get("history");
    if (history == NULL) { 
        history = "";
    }
    con << TAG <<
        HTML_HEAD "<TITLE>Table query</TITLE></HEAD>"
        BODY
        "<H2>Table " << tableName << "</H2>"
        "<TABLE BORDER><TR><TH>Field name</TH><TH>Field type</TH><TH>Referenced table</TH><TH>Inverse reference</TH><TH>Index</TH></TR>";
    dbFieldDescriptor* fd = desc->columns; 
    for (int i = desc->nColumns; --i >= 0;) { 
        con << TAG << "<TR><TD>" << fd->name << "</TD><TD>" << typeMnem[fd->type] << "</TD><TD>" 
            << (fd->refTableName != NULL 
                ? fd->refTableName 
                : (fd->type == dbField::tpArray && fd->components->refTableName != NULL)
                  ? fd->components->refTableName
                  : " ")  << "</TD><TD>" 
            << (fd->inverseRefName ? fd->inverseRefName : " ") 
            << "</TD><TD align=center>"
            << indexType(fd) << "</TD></TR>";
        fd = fd->next;
    }
    con << TAG << "</TABLE><P><TABLE>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=socket VALUE=\"" << con.getAddress() << "\">"
        "<INPUT TYPE=hidden NAME=page VALUE=selectionPage>"
        "<INPUT TYPE=hidden NAME=table VALUE=\"" << tableName << "\">"
        "<TR><TD>SELECT FROM <B>" << tableName << "</B> WHERE</TD>"
        "<TD><INPUT TYPE=text NAME=query VALUE=\""
        << history << "\" SIZE=40></TD>"
        "<TD><INPUT type=submit value=Select></TD></TR></FORM>";
    if (historyUsed != 0) { 
        con << TAG << "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
            "<INPUT TYPE=HIDDEN NAME=socket VALUE=\"" << con.getAddress() << "\">"
            "<INPUT TYPE=hidden NAME=page VALUE=queryPage>"
            "<INPUT TYPE=hidden NAME=table VALUE=\"" << tableName << "\">"
            "<TR><TD align=right>Query history</TD>"
            "<TD><SELECT SIZE=1 NAME=history>";
        for (unsigned i = historyCurr, j = historyUsed; j != 0; j -= 1) { 
            char* h = queryHistory[--i % MAX_HISTORY_SIZE];
            con << TAG << "<OPTION VALUE=\"" << h << "\">" << h << "</OPTION>";
        }
        con << TAG << "</TD><TD><INPUT type=submit value=Edit></TD></TR></FORM>";
    }
    con << TAG << "</TABLE></FORM>"
        "<P><FORM METHOD=POST ACTION=\"" << con.getStub() << "\">" 
        "<INPUT TYPE=HIDDEN NAME=socket VALUE=\"" << con.getAddress() << "\">"
        "<INPUT TYPE=hidden NAME=page VALUE=defaultPage>"
        "<INPUT TYPE=submit VALUE=\"Another table\"></FORM></BODY></HTML>";
}

    
enum ComponentType { 
    RecordComponent,
    ArrayComponent, 
    StructureComponent
};

void httpDumpRecord(WWWconnection& con, byte* base, dbFieldDescriptor* first, ComponentType componentType)
{
    int i, n;
    byte* elem;
    dbFieldDescriptor* fd = first;
    do {
        if (componentType == StructureComponent) { 
            con << TAG << "<TR><TH>" << fd->name << "</TH><TD>";
        } else if (componentType == RecordComponent) { 
            con << TAG << "<TD>";
        }
        switch (fd->type) {
          case dbField::tpBool:
            con << TAG << (*(bool*)(base + fd->dbsOffs) ? "true" : "false");
            break;
          case dbField::tpInt1:
            con << TAG << *(int1*)(base + fd->dbsOffs);
            break;
          case dbField::tpInt2:
            con << TAG << *(int2*)(base + fd->dbsOffs);
            break;
          case dbField::tpInt4:
            con << TAG << *(int4*)(base + fd->dbsOffs);
            break;
          case dbField::tpInt8:
            con << TAG << *(int8*)(base + fd->dbsOffs);
            break;
          case dbField::tpReal4:
            con << TAG << *(real4*)(base + fd->dbsOffs);
            break;
          case dbField::tpReal8:
            con << TAG << *(real8*)(base + fd->dbsOffs);
            break;
          case dbField::tpRectangle:
            {
                rectangle& r = *(rectangle*)(base + fd->dbsOffs);
                con << TAG << "<TABLE BORDER><TR>";                
                for (i = 0; i < rectangle::dim; i++) { 
                    con << TAG << "<TD>" << r.boundary[i] << "</TD>";
                }
                con << TAG << "</TR><TR>";                
                for (i = 0; i < rectangle::dim; i++) { 
                    con << TAG << "<TD>" << r.boundary[rectangle::dim+i] << "</TD>";
                }
                con << TAG << "</TR></TABLE>";
            }
            break;
          case dbField::tpString:
            con << TAG << "\"" << ((char*)base+((dbVarying*)(base+fd->dbsOffs))->offs) << "\"";
            break;
          case dbField::tpReference:
            {
                oid_t oid = *(oid_t*)(base + fd->dbsOffs);
                if (oid == 0) { 
                    con << TAG << "null";
                } else { 
                    con << TAG << "<A HREF=\"" <<  con.getStub() << "?socket="
                        << con.getAddress() << "&page=selectionPage&table=" << URL << fd->refTableName <<  "&query=" 
                        << URL << "current=" << oid << TAG << "\">@" << oid << "</A>";
                }
            }
            break;
          case dbField::tpRawBinary:
            n = fd->dbsSize;
            elem = base + fd->dbsOffs;
            con << TAG << "\"";
            for (i = 0; i < n; i++) {
                char buf[8];
                sprintf(buf, "\\x%02x", *elem++);
                con << TAG << buf;
            }
            con << TAG << "\"";
            break;
          case dbField::tpArray:
            n = ((dbVarying*)(base + fd->dbsOffs))->size;
            elem = base + ((dbVarying*)(base + fd->dbsOffs))->offs;
            con << TAG << "<OL>";
            for (i = 0; i < n; i++) {
                con << TAG << "<LI>";                
                httpDumpRecord(con, elem, fd->components, ArrayComponent);
                elem += fd->components->dbsSize;
            }
            con << TAG << "</OL>";
            break;
          case dbField::tpStructure:
            con << TAG << "<TABLE BORDER>"; 
            httpDumpRecord(con, base, fd->components,RecordComponent);
            con << TAG << "</TABLE>"; 
        }
        if (componentType == StructureComponent) { 
            con << TAG << "</TD></TR>";
        } else if (componentType == RecordComponent) { 
            con << TAG << "</TD>";
        }
    } while ((fd = fd->next) != first);
}

bool selectionPage(WWWconnection& con)
{
    ((dbSubSql*)con.userData)->selectionPage(con);
    return true;
}

void dbSubSql::selectionPage(WWWconnection& con)
{
    char const* tableName = con.get("table");
    char const* condition = con.get("query");
    dbTableDescriptor* desc = findTableByName(tableName);
    if (desc == NULL) {
        httpError(con, "No such table");
        return;
    }
    if (condition == NULL) {
        httpError(con, "Condition was not specified");
        return;
    }
    if (strlen(condition) > 0 
        && (historyUsed == 0 
            || strcmp(condition, queryHistory[unsigned(historyCurr-1)%MAX_HISTORY_SIZE]) != 0))
    {
        char* h = new char[strlen(condition)+1];
        strcpy(h, condition);
        if (historyCurr == historyUsed) { 
            historyUsed += 1;
        } else { 
            delete[] queryHistory[historyCurr];
        }
        queryHistory[historyCurr] = h;
        if (++historyCurr == MAX_HISTORY_SIZE) { 
            historyCurr = 0;
        }
    }
    dbAnyCursor cursor(*desc, dbCursorViewOnly, NULL);
    query.pos = pos;
    dbDatabaseThreadContext* ctx = threadContext.get();
    ctx->interactive = false;
    ctx->catched = true;
#ifdef THROW_EXCEPTION_ON_ERROR
    try {
#else
    if (setjmp(ctx->unwind) == 0) {
#endif
        query = condition;
        cursor.reset();
        select(&cursor, query);
        if (query.compileError()) {
            dbExprNodeAllocator::instance.reset();
            ctx->catched = false;
            httpQueryError(con, "query syntax error", tableName);
            return;
        }
        con << TAG <<
            HTML_HEAD "<TITLE>Selection</TITLE></HEAD>" 
            BODY
            "<H2>Selection from table " << tableName << "</H2>"
            "<TABLE BORDER><TR><TH>OID</TH>";
        dbFieldDescriptor* fd = desc->columns;
        do {
            con << TAG << "<TH>" << fd->name << "</TH>";
        } while ((fd = fd->next) != desc->columns);
        con << TAG << "</TR>";

        int nSelected = 0;
        if (cursor.gotoFirst()) {
            do {
                nSelected += 1;
                con << TAG << "<TR><TD>@" << cursor.currId << "</TD>";
                httpDumpRecord(con, (byte*)getRow(cursor.currId),
                               cursor.table->columns, RecordComponent);
                con << TAG << "</TR>";
            } while (cursor.gotoNext());
            con << TAG << "</TABLE>";
            if (nSelected > 1) {
                con << TAG << "<P>" << nSelected << " records selected";
            }
        } else { 
            con << TAG << "</TABLE><P>No records selected";
        }
#ifdef THROW_EXCEPTION_ON_ERROR
    } catch(dbException const& x) {
        httpQueryError(con, x.getMsg(), tableName);
        ctx->catched = false;
        commit(); // release locks
        return;
    }
#else
    } else { 
        httpQueryError(con, "Query error", tableName);
        if (query.mutexLocked) { 
            query.mutexLocked = false;
            query.mutex.unlock();
        }
        ctx->catched = false;
        commit(); // release locks
        return;
    }
#endif
    ctx->catched = false;
    commit(); // release locks
    
    con << TAG << 
        "<P><FORM METHOD=POST ACTION=\"" << con.getStub() << "\">" 
        "<INPUT TYPE=HIDDEN NAME=socket VALUE=\"" << con.getAddress() << "\">"
        "<INPUT TYPE=hidden NAME=table VALUE=\"" << tableName << "\">"
        "<INPUT TYPE=hidden NAME=page VALUE=queryPage>"
        "<INPUT TYPE=submit VALUE=\"New query\"></FORM></BODY></HTML>";
}
    

WWWapi::dispatcher dispatchTable[] = {
    {"defaultPage", defaultPage},
    {"queryPage", queryPage},
    {"selectionPage", selectionPage}
};




void dbSubSql::startHttpServer(char const* address) 
{
    if (httpServerRunning) { 
        error("HTTP server already started");
    } else { 
        httpServer = new HTTPapi(*this, itemsof(dispatchTable), dispatchTable);
        if (!httpServer->open(address, socket_t::sock_global_domain)) {
            delete httpServer;
            error("Failed to open HTTP session");
        } else { 
            httpServerRunning = true;
            httpServerThread.create(httpServerThreadProc, this);
        }
    }
}

void dbSubSql::stopHttpServer(char const* address) 
{
    if (!httpServerRunning) { 
        error("HTTP server was not started");
    } else {
        httpServerRunning = false;
        httpServer->cancel();        
    }
}

void thread_proc dbSubSql::httpServerThreadProc(void* arg) 
{
    ((dbSubSql*)arg)->httpServerLoop();
}
    
void  dbSubSql::httpServerLoop() 
{
    WWWconnection con;
    con.userData = this;
    attach();
    while (httpServer->connect(con) && httpServerRunning && httpServer->serve(con));
    delete httpServer;
    detach();
    httpServerRunning = false;
}

END_FASTDB_NAMESPACE

USE_FASTDB_NAMESPACE


#ifdef _WINCE

#define MAX_ARGS 32

#ifdef WINONLY
int WINAPI WinMain(
				   HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nCmdShow)
{
	char* line = m_cmdline;
#else
int WINAPI WinMain(
				   HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPTSTR lpCmdLine,
				   int nCmdShow)
{
	size_t len = wcslen(lpCmdLine);
	char* line   = new char[len+1];
	wcstombs(line, lpCmdLine, len);	
#endif
	int argc = 1;
	char* argv[MAX_ARGS];
	argv[0] = NULL;
	if (*line != '\0') {
		while (true) { 
			if (*line == '\"') { 
				argv[argc++] = ++line;
				while (*++line != '\"' && *line != '\0');
			} else { 
				argv[argc++] = line;
				while (*++line != ' ' && *line != '\0');
			}
			if (*line == '\0') { 
				break;
			}
			*line++ = '\0';
		}
	}
	argv[argc] = NULL;
#else
int main(int argc, char* argv[]) 
{
#endif    
    char* subsqlAccessType = getenv("SUBSQL_ACCESS_TYPE");    
    dbDatabase::dbAccessType accessType = dbDatabase::dbAllAccess;
    int firstParam = 1;
    if (argc > 1 && strcmp(argv[1], "-?") == 0 || strcmp(argv[1], "-h") == 0) { 
        printf("SubSQL interactive utility for FastDB v. %d.%.02d\n",
               FASTDB_MAJOR_VERSION, FASTDB_MINOR_VERSION);
        printf("Usage: subsql (-access [read-only|concurrent-read|all-access]) (SQL-FILE)*\n");
        return 1;
    }
    char* locale = getenv("SUBSQL_LOCALE");
    if (locale != NULL) { 
        setlocale(LC_COLLATE, locale);
    }

    if  (argc > firstParam+1 && strcmp(argv[firstParam], "-access") == 0) { 
        subsqlAccessType = argv[firstParam+1];
        firstParam += 2;
    }         
    if (subsqlAccessType != NULL) { 
        if (strcmp(subsqlAccessType, "read-only") == 0) { 
            accessType = dbDatabase::dbReadOnly;
        } else if (strcmp(subsqlAccessType, "concurrent-read") == 0) { 
            accessType = dbDatabase::dbConcurrentRead;
        } else if (strcmp(subsqlAccessType, "concurrent-update") == 0) { 
            accessType = dbDatabase::dbConcurrentUpdate;
        } else if (strcmp(subsqlAccessType, "normal") && strcmp(subsqlAccessType, "all-access") 
                   && strcmp(subsqlAccessType, "read-write") && strcmp(subsqlAccessType, "update"))
        {
            fprintf(stderr, "Invalid access type: '%s'\n", subsqlAccessType);
            return 1;
        }
    }
    dbSubSql db(accessType);
    db.run(firstParam, argc, argv);
    return 0;
}
    
