//-< HASHTAB.CPP >---------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 19-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Extensible hash table implementation
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "fastdb.h"
#include "hashtab.h"
#include <ctype.h>

BEGIN_FASTDB_NAMESPACE

int const dbHashTable::keySize[] = {
    1,  // tpBool
    1,  // tpInt1
    2,  // tpInt2
    4,  // tpInt4
    8,  // tpInt8
    4,  // tpReal4
    8,  // tpReal8
    0,  // tpString,
    sizeof(oid_t), // tpReference
    -1, // tpArray,
    -1  // tpStructure,
};

static const size_t primeNumbers[] = {
    17,             /* 0 */
    37,             /* 1 */
    79,             /* 2 */
    163,            /* 3 */
    331,            /* 4 */
    673,            /* 5 */
    1361,           /* 6 */
    2729,           /* 7 */
    5471,           /* 8 */
    10949,          /* 9 */
    21911,          /* 10 */
    43853,          /* 11 */
    87719,          /* 12 */
    175447,         /* 13 */
    350899,         /* 14 */
    701819,         /* 15 */
    1403641,        /* 16 */
    2807303,        /* 17 */
    5614657,        /* 18 */
    11229331,       /* 19 */
    22458671,       /* 20 */
    44917381,       /* 21 */
    89834777,       /* 22 */
    179669557,      /* 23 */
    359339171,      /* 24 */
    718678369,      /* 25 */
    1437356741,     /* 26 */
    2147483647      /* 27 (largest signed int prime) */
};


oid_t dbHashTable::allocate(dbDatabase* db, size_t nRows)
{
    size_t size = dbInitHashTableSize;
    if (size < nRows) {
        size = nRows;
    }
    size_t i;
    for (i = 0; i < itemsof(primeNumbers)-1 && primeNumbers[i] < size; i++);
    size = primeNumbers[i];
    oid_t hashId = db->allocateObject(dbHashTableMarker);
    int nPages = (size+dbIdsPerPage-1) / dbIdsPerPage;
    oid_t pageId = db->allocateId(nPages);
    offs_t pos = db->allocate(nPages*dbPageSize);
    assert((pos & (dbPageSize-1)) == 0);
    memset(db->baseAddr+pos, 0, nPages*dbPageSize);
    dbHashTable* hash = (dbHashTable*)db->get(hashId);
    hash->size = size;
    hash->page = pageId;
    hash->used = 0;
    while (--nPages >= 0) {
        db->currIndex[pageId++] = pos + dbPageObjectMarker;
        pos += dbPageSize;
    }
    return hashId;
}


inline unsigned dbHashTable::stringHashFunction(byte* key, int keylen)
{
    unsigned h;
#ifdef IGNORE_CASE
    for (h = 0; --keylen >= 0;) {
        int code = *key++;
        h = h*31 + toupper(code);
    }
#else
    for (h = 0; --keylen >= 0; h = h*31 + *key++);
#endif
    return h;
}

static unsigned oldUniversalHashFunction(byte* key, int type, int keylen)
{
    unsigned h;
    for (h = 0; --keylen >= 0; h = h*31 + *key++);
    return h;
}

static unsigned newUnversalHashFunction(byte* key, int type, int keylen)
{
    unsigned h;
    key += keylen;
    for (h = 0; --keylen >= 0; h = (h << 8) + *--key);
    return h;
}

static unsigned oldSpecializedHashFunction(byte* key, int type, int keylen)
{
    unsigned h;

    switch (type) {
      case dbField::tpBool:
      case dbField::tpInt1:
        return *(db_nat1*)key;
      case dbField::tpInt2:
        return *(db_nat2*)key;
      case dbField::tpInt4:
      case dbField::tpReal4:
        return *(db_nat4*)key;
      case dbField::tpInt8:
      case dbField::tpReal8:
        return *(db_nat4*)key ^ *((db_nat4*)key+1);
      default:
        key += keylen;
        for (h = 0; --keylen >= 0; h = (h << 8) + *--key);
        return h;
    }
}

static unsigned newSpecializedHashFunction(byte* key, int type, int keylen)
{
    unsigned h;

    switch (type) {
      case dbField::tpBool:
      case dbField::tpInt1:
        return *(db_nat1*)key;
      case dbField::tpInt2:
        return *(db_nat2*)key;
      case dbField::tpInt4:
      case dbField::tpReal4:
        return *(db_nat4*)key;
      case dbField::tpInt8:
      case dbField::tpReal8:
        return *(db_nat4*)key ^ *((db_nat4*)key+1);
      default:
        for (h = 0; --keylen >= 0; h = h*31 + *key++);
        return h;
    }
}

dbHashFunction dbHashTable::getHashFunction(int version)
{
    return (version < 288) ? &oldUniversalHashFunction
        : (version < 308) ? &newUnversalHashFunction
        : (version < 333) ? &oldSpecializedHashFunction : &newSpecializedHashFunction;
}

inline int keycmp(void* k1, void* k2, int type, int keylen)
{
    switch (type) {
      case dbField::tpBool:
      case dbField::tpInt1:
        return *(db_nat1*)k1 - *(db_nat1*)k2;
      case dbField::tpInt2:
        return *(db_nat2*)k1 - *(db_nat2*)k2;
      case dbField::tpInt4:
      case dbField::tpReal4:
        return *(db_nat4*)k1 - *(db_nat4*)k2;
      case dbField::tpInt8:
      case dbField::tpReal8:
        return *(db_nat8*)k1 == *(db_nat8*)k2 ? 0 : 1;
      default:
        return memcmp(k1, k2, keylen);
    }
}

void dbHashTable::insert(dbDatabase* db, oid_t hashId,
                         oid_t rowId, int type, int sizeofType, int offs, size_t nRows)
{
    dbHashTable* hash = (dbHashTable*)db->get(hashId);
    byte* record = db->get(rowId);
    byte* key = record + offs;
    unsigned hashkey;
    if (type == dbField::tpString) {
        int len = ((dbVarying*)key)->size - 1;
        key = record + ((dbVarying*)key)->offs;
        hashkey = stringHashFunction(key, len);
    } else {
        hashkey = db->hashFunction(key, type, sizeofType);
    }
    size_t size = hash->size;
    oid_t pageId = hash->page;
    if (size < nRows && hash->used*3/2 > size) {
        TRACE_MSG(("Reallocate hash table, used=%ld, size=%ld\n", hash->used, size));
        int nPages = (size+dbIdsPerPage-1) / dbIdsPerPage;
        size_t i;
        for (i = 0; i < itemsof(primeNumbers)-1 && primeNumbers[i] < size; i++);
        if (i < itemsof(primeNumbers)-1) {
            i += 1;
        }
        size = primeNumbers[i];
        int nNewPages = (size+dbIdsPerPage-1) / dbIdsPerPage;
        oid_t newPageId = db->allocateId(nNewPages);
        offs_t pos = db->allocate(nNewPages*dbPageSize);
        assert((pos & (dbPageSize-1)) == 0);
        memset(db->baseAddr + pos, 0, nNewPages*dbPageSize);
        hash = (dbHashTable*)db->put(hashId);
        hash->size = size;
        hash->page = newPageId;
        size_t used = 0;
        while (--nPages >= 0) {
            for (i = 0; i < dbIdsPerPage; i++) {
                oid_t itemId = ((oid_t*)db->get(pageId))[i];
                while (itemId != 0) {
                    dbHashTableItem* item = (dbHashTableItem*)db->get(itemId);
                    oid_t nextId = item->next;
                    unsigned h = item->hash % size;
                    oid_t* tab = (oid_t*)(db->baseAddr + pos);
                    if (item->next != tab[h]) {
                        item = (dbHashTableItem*)db->put(itemId);
                        tab = (oid_t*)(db->baseAddr + pos);
                        item->next = tab[h];
                    }
                    if (tab[h] == 0) {
                        used += 1;
                    }
                    tab[h] = itemId;
                    itemId = nextId;
                }
            }
            db->freeObject(pageId++);
        }
        ((dbHashTable*)db->get(hashId))->used = used;
        pageId = newPageId;
        while (--nNewPages >= 0) {
            db->currIndex[newPageId++] = pos + dbPageObjectMarker;
            pos += dbPageSize;
        }
    }
    oid_t itemId = db->allocateObject(dbHashTableItemMarker);
    unsigned h = hashkey % size;
    oid_t* ptr = (oid_t*)db->put(pageId + h/dbIdsPerPage) + h%dbIdsPerPage;
    dbHashTableItem* item = (dbHashTableItem*)db->get(itemId);
    item->record = rowId;
    item->hash = hashkey;
    item->next = *ptr;
    *ptr = itemId;
    if (item->next == 0) {
        ((dbHashTable*)db->get(hashId))->used += 1;
        db->file.markAsDirty(db->currIndex[hashId] & ~dbInternalObjectMarker, sizeof(dbHashTable));
    }
}


void dbHashTable::remove(dbDatabase* db, oid_t hashId,
                         oid_t rowId, int type, int sizeofType, int offs)
{
    dbHashTable* hash = (dbHashTable*)db->get(hashId);
    byte* record = (byte*)db->getRow(rowId);
    byte* key = record + offs;
    unsigned hashkey;
    if (type == dbField::tpString) {
        int len = ((dbVarying*)key)->size - 1;
        key = record + ((dbVarying*)key)->offs;
        hashkey = stringHashFunction(key, len);
    } else {
        hashkey = db->hashFunction(key, type, sizeofType);
    }
    unsigned h = hashkey % hash->size;
    oid_t pageId = hash->page + h / dbIdsPerPage;
    int i = h % dbIdsPerPage;
    oid_t itemId = ((oid_t*)db->get(pageId))[i];
    oid_t prevItemId = 0;
    while (true) {
        assert(itemId != 0);
        dbHashTableItem* item = (dbHashTableItem*)db->get(itemId);
        if (item->record == rowId) {
            oid_t next = item->next;
            if (prevItemId == 0) {
                if (next == 0) {
                    hash->used -= 1; // consistency can be violated
                    db->file.markAsDirty(db->currIndex[hashId] & ~dbInternalObjectMarker,
                                         sizeof(dbHashTable));
                }
                *((oid_t*)db->put(pageId) + i) = next;
            } else {
                item = (dbHashTableItem*)db->put(prevItemId);
                item->next = next;
            }
            db->freeObject(itemId);
            return;
        }
        prevItemId = itemId;
        itemId = item->next;
    }
}

void dbHashTable::find(dbDatabase* db, oid_t hashId, dbSearchContext& sc)
{
    dbHashTable* hash = (dbHashTable*)db->get(hashId);
    unsigned hashkey;
    unsigned keylen;
    if (hash->size == 0) {
        return;
    }
    if (sc.type == dbField::tpString) {
        keylen = strlen(sc.firstKey);
        hashkey = stringHashFunction((byte*)sc.firstKey, keylen);
    } else {
        keylen = sc.sizeofType;
        hashkey = db->hashFunction((byte*)sc.firstKey, sc.type, keylen);
    }
    unsigned h = hashkey % hash->size;
    oid_t itemId =
        ((oid_t*)db->get(hash->page + h/dbIdsPerPage))[h % dbIdsPerPage];
    dbTable* table = (dbTable*)db->getRow(sc.cursor->table->tableId);
    while (itemId != 0) {
        dbHashTableItem* item = (dbHashTableItem*)db->get(itemId);
        sc.probes += 1;
        if (item->hash == hashkey) {
            byte* rec = (byte*)db->getRow(item->record);
            if ((sc.type == dbField::tpString
                 && keylen == ((dbVarying*)(rec + sc.offs))->size - 1
#ifdef IGNORE_CASE
                 && stricmp(sc.firstKey,
                            (char*)rec+((dbVarying*)(rec+sc.offs))->offs) == 0)
#else
                 && keycmp(sc.firstKey, rec+((dbVarying*)(rec+sc.offs))->offs,
                           sc.type, keylen) == 0)
#endif
                || (sc.type != dbField::tpString
                    && sc.comparator(sc.firstKey, rec + sc.offs, keylen) == 0))
            {
                if (!sc.condition
                    || db->evaluate(sc.condition, item->record, table, sc.cursor))
                {
                    if (!sc.cursor->add(item->record)) {
                        return;
                    }
                }
            }
        }
        itemId = item->next;
    }
}



void dbHashTable::purge(dbDatabase* db, oid_t hashId)
{
    dbHashTable* hash = (dbHashTable*)db->put(hashId);
    oid_t pageId = hash->page;
    int nPages = (hash->size + dbIdsPerPage  - 1) / dbIdsPerPage;
    hash->used = 0;
    while (--nPages >= 0) {
        for (size_t i = 0; i < dbIdsPerPage; i++) {
            oid_t itemId = ((oid_t*)db->get(pageId))[i];
            while (itemId != 0) {
                oid_t nextId = ((dbHashTableItem*)db->get(itemId))->next;
                db->freeObject(itemId);
                itemId = nextId;
            }
        }
        memset(db->put(pageId++), 0, dbPageSize);
    }
}

void dbHashTable::drop(dbDatabase* db, oid_t hashId)
{
    dbHashTable* hash = (dbHashTable*)db->get(hashId);
    oid_t pageId = hash->page;
    int nPages = (hash->size + dbIdsPerPage - 1) / dbIdsPerPage;
    while (--nPages >= 0) {
        for (size_t i = 0; i < dbIdsPerPage; i++) {
            oid_t itemId = ((oid_t*)db->get(pageId))[i];
            while (itemId != 0) {
                oid_t nextId = ((dbHashTableItem*)db->get(itemId))->next;
                db->freeObject(itemId);
                itemId = nextId;
            }
        }
        db->freeObject(pageId++);
    }
    db->freeObject(hashId);
}

END_FASTDB_NAMESPACE
