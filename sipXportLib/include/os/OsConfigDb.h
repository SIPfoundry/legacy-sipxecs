//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsConfigDb_h_
#define _OsConfigDb_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsRWMutex.h"
#include "utl/UtlContainable.h"
#include "utl/UtlSortedList.h"
#include "utl/UtlString.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsConfigEncryption;
class UtlSList;

/**
 * Class for holding a name/value pair.
 */
class DbEntry : public UtlContainable
{
 public:

    DbEntry(const UtlString &key);

    DbEntry(const UtlString &key, const UtlString &value);

    ~DbEntry();

    virtual UtlContainableType getContainableType() const;

    static const UtlContainableType TYPE;

    virtual unsigned int hash() const;

    int compareTo(const UtlContainable *b) const;

    UtlString key;

    UtlString value;
};


/**
 * Configuration database containing key/value pairs with ability to
 * read and write to disk.
 */
class OsConfigDb
{
   friend class OsConfigDbTest;

public:

    OsConfigDb();

    virtual ~OsConfigDb();

    virtual OsStatus loadFromFile(FILE* fp);

    /**
     *  Load the configuration database from a file
     */
    virtual OsStatus loadFromFile(const char *filename);

    /**
     * Load the configuation database from a string buffer(Buffer
     * CANNOT be encrypted) with the following format:
     *
     * s : s\n
     * s :
     * s\n...
     */
    virtual OsStatus loadFromBuffer(const char *buf);

    /**
     * Store the config database to a file
     */
    virtual OsStatus storeToFile(const char *filename);

    /**
     * Remove the key/value pair associated with rKey.
     *
     * return OS_SUCCESS if the key was found in the database,
     * return OS_NOT_FOUND otherwise
     */
    OsStatus remove(const UtlString& rKey);

    /**
     * Remove all the key/value pairs starting with the designated prefix
     *
     * return OS_SUCCESS if one key or more keys were found in the database,
     * return OS_NOT_FOUND otherwise
     */
    OsStatus removeByPrefix(const UtlString& rPrefix);

    /**
     * Insert the key/value pair into the config database If the
     * database already contains an entry for this key, then set the
     * value for the existing entry to rNewValue.
     */
    void set(const UtlString& rKey, const UtlString& rNewValue);

    /**
     * Insert the key/value pair into the config database If the
     * database already contains an entry for this key, then set the
     * value for the existing entry to iNewValue.
     */
    void set(const UtlString& rKey, const int iNewValue);

    /**
     * Sets rValue to the value in the database associated with rKey.
     * If rKey is found in the database, returns OS_SUCCESS.  Otherwise,
     * returns OS_NOT_FOUND and sets rValue to the empty string.
     */
    virtual OsStatus get(const UtlString& rKey, UtlString& rValue) const;

    /**
     * Sets rValue to the value in the database associated with rKey.
     * If rKey is found in the database, returns OS_SUCCESS.  Otherwise,
     * returns OS_NOT_FOUND and sets rValue to -1.
     */
    virtual OsStatus get(const UtlString& rKey, int& rValue) const;

    /**
     * Returns the boolean value in the database associated with rKey.
     * Strings starting with T, t, Y, y, or 1 are considered true,
     * and strings starting with F, f, N, n, or 0 are considered false.
     * If rKey is not found in the database or the value does not
     * start with an acceptable character, returns defaultValue.
     */
    virtual UtlBoolean getBoolean(const UtlString& rKey,
                                  UtlBoolean defaultValue);

    /**
     * Filename, URI, or what helps identify the contents of this config
     */
    virtual const char *getIdentityLabel() const;

    /**
     * Filename, URI, or what helps identify the contents of this config
     */
    virtual void setIdentityLabel(const char *idLabel);

    /**
     * Current encryption support. NULL when there's no encryption
     * support, !NULL then there's a possiblity that actual contents of
     * config will be encrypted or decrypted from/to io source.
     */
    OsConfigEncryption *getEncryption() const;

    /**
     * Set the default encryption support for all OsConfig instances
    */
    static void setStaticEncryption(OsConfigEncryption *encryption);

    /**
     * Get the encryption support for call instances
     */
    static OsConfigEncryption *getStaticEncryption();

    /**
     * force capitalization of all keys, most profiles want this off
     * even keys are typcialy stored as capitalized
     */
    void setCapitalizeName(UtlBoolean capitalize);

    /**
     * Store all contents into a buffer, call calculateBufferSize to
     * get safe size. Call strlen to get actual size
     */
    void storeToBuffer(char *buff) const;

    /**
     * Return gauronteed to be large enough, (unless values are
     * changed) when storing into a buffer
     */
    int calculateBufferSize() const;

    /**
     * Return TRUE if the database is empty, otherwise FALSE
    */
    virtual UtlBoolean isEmpty(void) const;

    /**
     * Return the number of entries in the config database
     */
    virtual int numEntries(void) const;

    /**
     * Get a hash of name value pairs with the given key prefix
     */
    virtual OsStatus getSubHash(const UtlString& rHashSubKey, /**< the prefix for keys to name value pairs
                                                               * which are copied into the given rSubDb. The key in the
                                                               * sub-OsConfigDb have the prefix removed.
                                                               */
                                OsConfigDb& rSubDb) const;

    /**
     * Relative to <i>rKey</i>, return the key and value associated
     * with next (lexicographically ordered) key/value pair stored in
     * the database.  If rKey is the empty string, key and value
     * associated with the first entry in the database will be
     * returned.
     *
     * @return OS_SUCCESS if there is a "next" entry;
     * @return OS_NOT_FOUND if rKey is not found in the database and is not the
     *             empty string
     * @return OS_NO_MORE_DATA if there is no "next" entry.
     */
    virtual OsStatus getNext(const UtlString& rKey,
                             UtlString& rNextKey, UtlString& rNextValue) const;

    /**
     * Stores a list of strings to the configuration datadase using the
     * designated prefix as the base for the list items.  The prefix is used
     * to build unique configuration keys.  For example, if you use specify
     * a prefix of "MYLIST" and supply a list containing ("item 1", "item 2",
     * and "item 3"), you will end up with the following:
     *
     * MYLIST.COUNT : 3
     * MYLIST.1 : item 1
     * MYLIST.2 : item 2
     * MYLIST.3 : item 3
     *
     * Warning: All items with a key of "[rPrefix]." are removed as a side effect.
     *
     * @param rPrefix Configuration name prefix
     * @param rList List of UtlString values.
     */
    virtual void addList(const UtlString& rPrefix,
                         UtlSList& rList);

    /**
     * Loads a list of strings from the configuration datadase using the
     * designated prefix as the base for the list items.  The number of
     * list items is returned.
     *
     * @param rPrefix Configuration name prefix
     * @param rList List of UtlString values.
     *
     * @see addList
     */
    virtual int loadList(const UtlString& rPrefix,
                         UtlSList& rList) const;


    /**
     * Helper method to obtain a port value from the configuration database.
     * Results are as follows:
     * <pre>
     *   PORT_DEFAULT : Let a port be selected automatically.
     *                  Represented as "DEFAULT".
     *   PORT_NONE :    Disabled (either specified as such, the key
     *                  was not found, or the value was blank)
     *                  Represented as "NONE".
     *   other :        The port number that was specified
     *                  Represented as a decimal integer.
     * </pre>
     *
     * @param szKey Key file to lookup.
     */
    int getPort(const char* szKey) const;

 protected:

    /** reader/writer lock for synchronization */
    mutable OsRWMutex mRWMutex;

    /** sorted storage of key/values */
    UtlSortedList mDb;

    /** ID, used to distiguish which files should be encrypted */
    UtlString mIdentityLabel;

    /**
     * Force capitalization on all keys. Most profile do not want this
     * on even though most of their keys are already captilized
     */
    UtlBoolean mCapitalizeName;

    OsStatus loadFromEncryptedFile(const char *filename);

    OsStatus loadFromUnencryptedFile(FILE* fp);

    OsStatus loadFromEncryptedBuffer(char *buf, int bufLen);

    OsStatus loadFromUnencryptedBuffer(const char *buf);

    OsStatus storeToEncryptedFile(const char *filename);

    OsStatus storeBufferToFile(const char *filename, const char *buff, unsigned long buffLen);

    void dump();

        virtual OsStatus storeToFile(FILE* fp);

    /**
     * Parse "key : value" and add to dictionary.
     */
    void insertEntry(const char* line);

    /**
     * Helper method for inserting a key/value pair into the dictionary
     * The write lock for the database should be taken before calling this
     * method. If the database already contains an entry for this key, then
     * set the value for the existing entry to rNewValue.
     */
    void insertEntry(const UtlString& rKey, const UtlString& rNewValue);

    /**
     * Copy constructor (not implemented for this class)
     */
    OsConfigDb(const OsConfigDb& rOsConfigDb);

    /**
     * Assignment operator (not implemented for this class)
     */
    OsConfigDb& operator=(const OsConfigDb& rhs);

    /**
     * Utility func to remove all chars = c from given string
     */
    static void removeChars(UtlString *s, char c);
};


/* ============================ INLINE METHODS ============================ */

#endif  // _OsConfigDb_h_
