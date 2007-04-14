//-< CONTAINER.H >---------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     05-Nov-2002  K.A. Knizhnik  * / [] \ *
//                          Last update: 05-Nov-2002  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// T-Tree object container
//-------------------------------------------------------------------*--------*

#ifndef __CONTAINER_H__
#define __CONTAINER_H__

/**
 * Base class for all containers.
 * Container are implemented using T-Tree
 */
class FASTDB_DLL_ENTRY dbAnyContainer : public dbAnyReference {
  protected:
    dbFieldDescriptor* fd;

    void create(dbDatabase& db);
    void purge(dbDatabase& db);
    void free(dbDatabase& db);
    void add(dbDatabase& db, dbAnyReference const& ref);
    void remove(dbDatabase& db, dbAnyReference const& ref);
    int  search(dbAnyCursor& cursor, void const* from, void const* till);

    dbAnyContainer(char const* fieldName, dbTableDescriptor& desc);
};


/**
 * Template of container for particular table
 */
template<class T>
class dbContainer : public dbAnyContainer {
  public:
    /**
     * Search records matching search criteria (between, less or equal, great or equal)
     * @param cursor cursor to iterate through selected resords
     * @param from inclusive low bound for the search key, if <code>NULL</code> then there is no low bound
     * @param till inclusive high bound for the search key,  if <code>NULL</code> then there is no high bound
     * @return number of selected records
     */
    int search(dbCursor<T>& cursor, void const* from, void const* till) {
        return dbAnyContainer::search(cursor, from, till);
    }
    /**
     * Select records with sepcified value of the key
     * @param cursor cursor to iterate through selected resords
     * @param key searched value of the key
     * @return number of selected records
     */    
    int search(dbCursor<T>& cursor, void const* key) {
        return dbAnyContainer::search(cursor, key, key);
    }

    /**
     * Select all records in the container
     * @param cursor cursor to iterate through selected resords
     * @return number of selected records
     */
    int search(dbCursor<T>& cursor) {
        return dbAnyContainer::search(cursor, NULL, NULL);
    }

    /**
     * Create new container.
     */
    void create() {
        dbAnyContainer::create(T::dbDescriptor.db);
    }

    /**
     * Clear the container
     */
    void purge() {
        dbAnyContainer::purge(T::dbDescriptor.db);
    }

    /**
     * Deallocate container
     */
    void free() {
        dbAnyContainer::free(T::dbDescriptor.db);
    }

    /**
     * Add new record to container
     * @param ref reference to the record added to the container
     */
    void add(dbReference<T> const& ref) {
        dbAnyContainer::add(T::dbDescriptor.db, ref);
    }

    /**
     * Remove record from the container
     * @param ref reference to the record deleted from the container
     */
    void remove(dbReference<T> const& ref) {
        dbAnyContainer::remove(T::dbDescriptor.db, ref);
    }

    /**
     * Constructor of the contanier reference
     * @param fieldName name of the key field used by container
     */
    dbContainer(const char* fieldName) : dbAnyContainer(fieldName, T::dbDescriptor) {}
};

#endif




