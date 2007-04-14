//-< CONTAINER.CPP >---------------------------------------------------*--------*
// GigaBASE                  Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     05-Nov-2002  K.A. Knizhnik  * / [] \ *
//                          Last update: 05-Nov-2002  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// T-Tree object container
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "fastdb.h"
#include "compiler.h"
#include "ttree.h"
#include "symtab.h"

void dbAnyContainer::create(dbDatabase& db)
{
    oid = dbTtree::allocate(&db);
}

void dbAnyContainer::add(dbDatabase& db, dbAnyReference const& ref)
{
    dbTtree::insert(&db, oid, ref.getOid(), fd->type, fd->dbsSize, fd->comparator, fd->dbsOffs);
}

void dbAnyContainer::remove(dbDatabase& db, dbAnyReference const& ref)
{
    dbTtree::insert(&db, oid, ref.getOid(), fd->type, fd->dbsSize, fd->comparator, fd->dbsOffs);
}

void dbAnyContainer::purge(dbDatabase& db)
{
    dbTtree::purge(&db, oid);
}

void dbAnyContainer::free(dbDatabase& db)
{
    dbTtree::purge(&db, oid);
}

int dbAnyContainer::search(dbAnyCursor& cursor, void const* from, void const* till)
{
    dbDatabase* db = cursor.table->db;
    db->beginTransaction(cursor.type == dbCursorForUpdate ? dbDatabase::dbExclusiveLock : dbDatabase::dbSharedLock);
    dbDatabaseThreadContext* ctx = db->threadContext.get();
    ctx->cursors.link(&cursor);
    cursor.reset();
    if (from == NULL && till == NULL) {
        dbTtree::traverseForward(db, oid, &cursor);
    } else {
        dbSearchContext sc;
        sc.db = db;
        sc.condition = NULL;
        sc.firstKey = (char*)from;
        sc.firstKeyInclusion = 1;
        sc.lastKey = (char*)till;
        sc.lastKeyInclusion = 1;
        sc.comparator = fd->comparator;
        sc.type = fd->type;
        sc.cursor = &cursor;
        sc.sizeofType = fd->dbsSize;
        dbTtree::find(db, oid, sc);
    }
    return cursor.getNumberOfRecords();
}

dbAnyContainer::dbAnyContainer(char const* name, dbTableDescriptor& desc)
{
    fd = desc.find(name);
}

