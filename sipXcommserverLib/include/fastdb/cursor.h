//-< CURSOR.H >------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Table cursor
//-------------------------------------------------------------------*--------*

#ifndef __CURSOR_H__
#define __CURSOR_H__

class dbOrderByNode;

class FASTDB_DLL_ENTRY dbSelection { 
  public:
    enum { quantum = 1024 };
    class segment { 
      public:
	segment* prev;
	segment* next;
	size_t   nRows;
	oid_t    rows[quantum];

	segment(segment* after) { 
	    prev = after;
	    next = NULL;
	    nRows = 0;
	}	
    };
    segment*  first;
    segment*  last;
    segment*  curr;
    size_t    nRows;
    size_t    pos;

    segment*  createNewSegment(segment* after);

    void add(oid_t oid) {
        if (last == NULL) { 
            first = last = createNewSegment(NULL);
        } else if (last->nRows == quantum) { 
            last = last->next = createNewSegment(last);
        }
        last->rows[last->nRows++] = oid;
        nRows += 1;
    }
   
    void sort(dbDatabase* db, dbOrderByNode* order);
    static int compare(dbRecord* a, dbRecord* b, dbOrderByNode* order);

    void toArray(oid_t* oids);

    dbSelection() { 
	nRows = 0;
	pos = 0;
	first = curr = last = NULL;
    }
    void reverse();
    void reset();
};

enum dbCursorType { 
    dbCursorViewOnly,
    dbCursorForUpdate
};

/**
 * Base class for all cursors
 */
class FASTDB_DLL_ENTRY dbAnyCursor : public dbL2List { 
    friend class dbAnyContainer;
    friend class dbDatabase;
    friend class dbHashTable;
    friend class dbTtreeNode;
    friend class dbSubSql;
    friend class dbStatement;
    friend class dbServer;
    friend class dbCLI;
  public:
    /**
     * Get number of selected records
     * @return number of selected records
     */
    int getNumberOfRecords() { return selection.nRows; }

    /**
     * Remove current record
     */
    void remove();
    
    /**
     * Checks whether selection is empty
     * @return true if there is no current record
     */
    bool isEmpty() { return currId == 0; }

    /**
     * Checks whether limit for number of selected reacord is reached
     * @return true if limit is reached
     */
    bool isLimitReached() { return selection.nRows >= limit; }

    /**
     * Extract OIDs of selected recrods in array
     * @param arr if <code>arr</code> is not null, then this array is used as destination (it should
     *   be at least selection.nRows long)<BR>
     *  If <code>arr</code> is null, then new array is created by  new oid_t[] and returned by this method
     * @return if <code>arr</code> is not null, then <code>arr</code>, otherwise array created by this method
     */
    oid_t* toArrayOfOid(oid_t* arr); 

    /**
     * Execute query.
     * @param query selection criteria
     * @param aType cursor type: <code>dbCursorForUpdate, dbCursorViewOnly</code>
     * @param paramStruct pointer to structure with parameters. If you want to create reentrant precompiled query, i.e.
     * query which can be used concurrently by different threadsm you should avoid to use static variables in 
     * such query, and instead of it place paramters into some structure, specify in query relative offsets to the parameters,
     * fill local structure and pass pointer to it to select method.
     * @return number of selected records
     */
    int select(dbQuery& query, dbCursorType aType, void* paramStruct = NULL) {
	type = aType;
	reset();
	paramBase = paramStruct;
	db->select(this, query);
	paramBase = NULL;
	if (gotoFirst() && prefetch) { 
	    fetch();
	}
	return selection.nRows;
    } 
    
    /**
     * Execute query with default cursor type.
     * @param query selection criteria
     * @param paramStruct pointer to structure with parameters.
     * @return number of selected records
     */    
    int select(dbQuery& query, void* paramStruct = NULL) { 
	return select(query, defaultType, paramStruct);
    }
     
    /**
     * Execute query.
     * @param condition selection criteria
     * @param aType cursor type: <code>dbCursorForUpdate, dbCursorViewOnly</code>
     * @param paramStruct pointer to structure with parameters.
     * @return number of selected records
     */
    int select(char const* condition, dbCursorType aType, void* paramStruct = NULL) { 
	dbQuery query(condition);
	return select(query, aType, paramStruct);
    } 

    /**
     * Execute query with default cursor type.
     * @param condition selection criteria
     * @param paramStruct pointer to structure with parameters.
     * @return number of selected records
     */    
    int select(char const* condition, void* paramStruct = NULL) { 
	return select(condition, defaultType, paramStruct);
    }

    /**
     * Select all records from the table
     * @param aType cursor type: <code>dbCursorForUpdate, dbCursorViewOnly</code>
     * @return number of selected records
     */    
    int select(dbCursorType aType) { 
	type = aType;
	reset();
	db->select(this); 
	if (gotoFirst() && prefetch) { 
	    fetch();
	}
	return selection.nRows;
    } 

    /**
     * Select all records from the table with default cursor type
     * @return number of selected records
     */    
    int select() {
	return select(defaultType);
    }

    /**
     * Select all records from the table with specfied value of the key
     * @param key name of the key field
     * @param value searched value of the key
     * @return number of selected records
     */    
    int selectByKey(char const* key, void const* value);

    /**
     * Select all records from the table with specfied range of the key values
     * @param key name of the key field
     * @param minValue inclusive low bound for key values, if <code>NULL</code> then there is no low bound
     * @param maxValue inclusive high bound for key values, if <code>NULL</code> then there is no high bound
     * @return number of selected records
     */    
    int selectByKeyRange(char const* key, void const* minValue, void const* maxValue);

    /**
     * Update current record. You should changed value of current record before and then call
     * update method to save changes to the database
     */
    void update() { 
	assert(type == dbCursorForUpdate && currId != 0);
	updateInProgress = true;
	db->update(currId, table, record);
	updateInProgress = false;
    }

    /**
     * Remove all records in the table
     */
    void removeAll() {
	assert(db != NULL);
	db->deleteTable(table);
	reset();
    }

    /**
     * Remove all selected records
     */
    void removeAllSelected();

    /**
     * Specify maximal number of records to be selected
     */
    void setSelectionLimit(size_t lim) { limit = lim; }
    
    /**
     * Remove selection limit
     */
    void unsetSelectionLimit() { limit = dbDefaultSelectionLimit; }

    /**
     * Set prefetch mode. By default, current record is fetch as soon as it is becomes current.
     * But sometimesyou need only OIDs of selected records. In this case setting prefetchMode to false can help.
     * @param mode if <code>false</code> then current record is not fetched. You should explicitly call <code>fetch</code>
     * method if you want to fetch it.
     */
    void setPrefetchMode(bool mode) { prefetch = mode; }

    /**
     * Reset cursor
     */
    void reset();

    /**
     * Check whether current record is the last one in the selection
     * @return true if next() method will return <code>NULL</code>
     */
    bool isLast(); 

    /**
     * Check whether current record is the first one in the selection
     * @return true if prev() method will return <code>NULL</code>
     */
    bool isFirst(); 

    /**
     * Freeze cursor. This method makes it possible to save current state of cursor, close transaction to allow
     * other threads to proceed, and then later restore state of the cursor using unfreeze method and continue 
     * traversal through selected records.
     */     
    void freeze();

    /**
     * Unfreeze cursor. This method starts new transaction and restore state of the cursor
     */
    void unfreeze();

    /**
     * Skip specified number of records
     * @param n if positive then skip <code>n</code> records forward, if negative then skip <code>-n</code> 
     * records backward
     * @return <code>true</code> if specified number of records was successfully skipped, <code>false</code> if
     * there is no next (<code>n &gt; 0</code>) or previous (<code>n &lt; 0</code>) record in the selction.
     */
    bool skip(int n);


    /**
     * Position cursor on the record with the specified OID
     * @param oid object identifier of record
     * @return poistion of the record in the selection or -1 if record with such OID is not in selection
     */
    int seek(oid_t oid);

    /**
     * Get table for which cursor is opened
     */
    dbTableDescriptor* getTable() { return table; }


    /**
     * Set table for the cursor
     * @param aTable table which records will be iterated
     */
    void setTable(dbTableDescriptor* aTable) { 
	table = aTable;
	db = aTable->db;
    }

    /**
     * Set destination for selected record
     * rec - buffer to which fields of current record will be fetched
     */
    void setRecord(void* rec) { 
	record = (byte*)rec;
    }

    /**
     * Get pointer to the location where fields of the current record are fetched
     * @return pointer to the memory location set by cursor constructor or setRecord method
     */
    void* getRecord() { 
        return record;
    }

  protected: 
    dbDatabase*        db;
    dbTableDescriptor* table;
    dbCursorType       type;
    dbCursorType       defaultType;
    dbSelection        selection;
    bool               allRecords;
    oid_t              firstId;
    oid_t              lastId;
    oid_t              currId;
    byte*              record;
    size_t             limit;

    int4*              bitmap; // bitmap to avoid duplicates
    size_t             bitmapSize;
    bool               eliminateDuplicates;
    bool               prefetch;
    bool               removed; // current record was removed
    bool               updateInProgress;

    void*              paramBase;
    
    void checkForDuplicates();

    bool isMarked(oid_t oid) { 
	return bitmap != NULL && (bitmap[oid >> 5] & (1 << (oid & 31))) != 0;
    }

    void mark(oid_t oid) { 
	if (bitmap != NULL) { 
	    bitmap[oid >> 5] |= 1 << (oid & 31);
	}
    }    

    bool add(oid_t oid) { 
	if (selection.nRows < limit) { 
	    if (eliminateDuplicates) { 
		if (bitmap[oid >> 5] & (1 << (oid & 31))) { 
		    return true;
		}
		bitmap[oid >> 5] |= 1 << (oid & 31);
	    } 
	    selection.add(oid);
	    return selection.nRows < limit;
	} 
	return false;
    }

    bool gotoNext();
    bool gotoPrev(); 
    bool gotoFirst();
    bool gotoLast();
    
    void setCurrent(dbAnyReference const& ref);

    void fetch() { 
	assert(!(db->currIndex[currId] 
		 & (dbInternalObjectMarker|dbFreeHandleMarker)));
	table->columns->fetchRecordFields(record, 
					  (byte*)db->getRow(currId));
    }

    void adjustReferences(size_t base, size_t size, long shift) { 
	if (currId != 0) { 
	    table->columns->adjustReferences(record, base, size, shift);
	}
    }

    dbAnyCursor(dbTableDescriptor& aTable, dbCursorType aType, byte* rec)
    : table(&aTable),type(aType),defaultType(aType),
      allRecords(false),currId(0),record(rec)
    {
	db = aTable.db;
	limit = dbDefaultSelectionLimit;
	updateInProgress = false;
	prefetch = true;
	removed = false;
	bitmap = NULL; 
	bitmapSize = 0;
	eliminateDuplicates = false;
	paramBase = NULL;
    }
  public:
    dbAnyCursor() 
    : table(NULL),type(dbCursorViewOnly),defaultType(dbCursorViewOnly),
	  allRecords(false),currId(0),record(NULL)
    {
	limit = dbDefaultSelectionLimit;
	updateInProgress = false;
	prefetch = false;
	removed = false;
	bitmap = NULL;
	bitmapSize = 0;
	eliminateDuplicates = false;
	db = NULL;
	paramBase = NULL;
    }
    ~dbAnyCursor();
};


/**
 * Cursor template parameterized by table class
 */
template<class T>
class dbCursor : public dbAnyCursor { 
  protected:
    T record;
    
  public:
    /**
     * Cursor constructor
     * @param type cursor type (dbCursorViewOnly by default)
     */
    dbCursor(dbCursorType type = dbCursorViewOnly) 
        : dbAnyCursor(T::dbDescriptor, type, (byte*)&record) {}

    /**
     * Cursor constructor with explicit specification of database.
     * This cursor should be used for unassigned tables. 
     * @param aDB database in which table lokkup is performed
     * @param type cursor type (dbCursorViewOnly by default)
     */
    dbCursor(dbDatabase* aDb, dbCursorType type = dbCursorViewOnly)
	: dbAnyCursor(T::dbDescriptor, type, (byte*)&record)
    {
	db = aDb;
	dbTableDescriptor* theTable = db->lookupTable(table);
	if (theTable != NULL) { 
	    table = theTable;
	}
    }

    /**
     * Get pointer to the current record
     * @return pointer to the current record or <code>NULL</code> if there is no current record
     */
    T* get() { 
	return currId == 0 ? (T*)NULL : &record; 
    }
    
    /**
     * Get next record
     * @return pointer to the next record or <code>NULL</code> if there is no next record
     */     
    T* next() { 
	if (gotoNext()) { 
	    fetch();
	    return &record;
	}
	return NULL;
    }

    /**
     * Get previous record
     * @return pointer to the previous record or <code>NULL</code> if there is no previous record
     */     
    T* prev() {	
	if (gotoPrev()) { 
	    fetch();
	    return &record;
	}
	return NULL;
    }

    /**
     * Get pointer to the first record
     * @return pointer to the first record or <code>NULL</code> if no records were selected
     */
    T* first() { 
	if (gotoFirst()) {
	    fetch();
	    return &record;
	}
	return NULL;
    }

    /**
     * Get pointer to the last record
     * @return pointer to the last record or <code>NULL</code> if no records were selected
     */
    T* last() { 
	if (gotoLast()) {
	    fetch();
	    return &record;
	}
	return NULL;
    }    
    
    /**
     * Position cursor on the record with the specified OID
     * @param oid object identifier of record
     * @return poistion of the record in the selection or -1 if record with such OID is not in selection
     */
    int seek(dbReference<T> const& ref) { 
        return dbAnyCursor::seek(ref.getOid());
    }

    /**
     * Overloaded operator for accessing components of the current record
     * @return pointer to the current record
     */
    T* operator ->() { 
	assert(currId != 0);
	return &record;
    }

    /**
     * Select record by reference
     * @param ref reference to the record
     * @return pointer to the referenced record
     */
    T* at(dbReference<T> const& ref) { 
	setCurrent(ref);
	return &record;
    }
    
    /**
     * Convert selection to array of reference
     * @param arr [OUT] array of refeences in which references to selected recrods will be placed
     */
    void toArray(dbArray< dbReference<T> >& arr) { 
	arr.resize(selection.nRows);
	toArrayOfOid((oid_t*)arr.base());
    }

    /**
     * Get current object idenitifer
     * @param reference to the current record
     */
    dbReference<T> currentId() { 
	return dbReference<T>(currId);
    }

    /**
     * Method nextAvailable allows to iterate through the records in uniform way even when some records 
     * are removed. For example:
     * <PRE>
     * if (cursor.select(q) > 0) { 
     *     do { 
     *         if (x) { 
     *             cursor.remove();
     *         } else { 
     *             cursor.update();
     *         }
     *     } while (cursor.nextAvaiable());
     *  }
     *</PRE>
     * @return pointer to the current record
     */     
    T* nextAvailable() { 
	if (!removed) { 
	    return next(); 
	} else { 
	    removed = false;
	    return get();
	}
    }
};

class dbParallelQueryContext { 
  public:
    dbDatabase* const      db;
    dbCompiledQuery* const query;
    oid_t                  firstRow;
    dbTable*               table;
    dbAnyCursor*           cursor;
    dbSelection            selection[dbMaxParallelSearchThreads];

    void search(int i); 

    dbParallelQueryContext(dbDatabase* aDb, dbTable* aTable, 
			   dbCompiledQuery* aQuery, dbAnyCursor* aCursor)
      : db(aDb), query(aQuery), firstRow(aTable->firstRow), table(aTable), cursor(aCursor) {}
};


extern char* strupper(char* s);

extern char* strlower(char* s);

#endif
