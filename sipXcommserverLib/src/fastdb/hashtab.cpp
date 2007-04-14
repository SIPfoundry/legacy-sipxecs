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

#include <ctype.h>
#include "fastdb.h"
#include "hashtab.h"

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

oid_t dbHashTable::allocate(dbDatabase* db, size_t nRows)
{
    size_t size = dbInitHashTableSize;
    while (size <= nRows) { 
	size = (size+1)*2 - 1;
    }
    oid_t hashId = db->allocateObject(dbHashTableMarker);
    int nPages = (size+1) / dbIdsPerPage;
    oid_t pageId = db->allocateId(nPages);
    offs_t pos = db->allocate((size+1)*sizeof(oid_t));
    assert((pos & (dbPageSize-1)) == 0);
    memset(db->baseAddr+pos, 0, (size+1)*sizeof(oid_t));
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
 

inline unsigned dbHashTable::strHashCode(byte* key, int keylen) 
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

inline unsigned dbHashTable::hashCode(byte* key, int keylen) 
{
    unsigned h;
    for (h = 0; --keylen >= 0; h = h*31 + *key++);
    return h;
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
	hashkey = strHashCode(key, len);
    } else {  
	hashkey = hashCode(key, sizeofType);
    }
    size_t size = hash->size;
    oid_t pageId = hash->page;
    if (size < nRows && hash->used*2/3 > size) {
	int nPages = (size+1) / dbIdsPerPage;
	size = (size+1)*2-1;
	oid_t newPageId = db->allocateId((size+1) / dbIdsPerPage);
	offs_t pos = db->allocate((size+1)*sizeof(oid_t));
	assert((pos & (dbPageSize-1)) == 0);
	memset(db->baseAddr + pos, 0, (size+1)*sizeof(oid_t));
	hash = (dbHashTable*)db->put(hashId);
	hash->size = size;
	hash->page = newPageId;
	size_t used = 0;
	while (--nPages >= 0) { 
	    for (size_t i = 0; i < dbIdsPerPage; i++) { 
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
	for (nPages = (size+1)/dbIdsPerPage; --nPages >= 0; pos += dbPageSize){
	    db->currIndex[newPageId++] = pos + dbPageObjectMarker;
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
	((dbHashTable*)db->put(hashId))->used += 1;
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
	hashkey = strHashCode(key, len);
    } else {  
	hashkey = hashCode(key, sizeofType);
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
	hashkey = strHashCode((byte*)sc.firstKey, keylen);
    } else {  
	keylen = sc.sizeofType;
	hashkey = hashCode((byte*)sc.firstKey, keylen);
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
		 && strcasecmp(sc.firstKey, 
                               (char*)rec+((dbVarying*)(rec+sc.offs))->offs) == 0)
#else
		 && memcmp(sc.firstKey, rec+((dbVarying*)(rec+sc.offs))->offs, 
			   keylen) == 0)
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
    int nPages = (hash->size+1) / dbIdsPerPage;
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
    int nPages = (hash->size+1) / dbIdsPerPage;
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







