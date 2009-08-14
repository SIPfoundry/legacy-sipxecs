//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef RESULTSET_H
#define RESULTSET_H

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "utl/UtlSList.h"
#include "utl/UtlHashMap.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// A set of rows from a conceptual table; each row is name/value pairs.
/**
 * A ResultSet is a list of hash tables, used primarily in the interfaces
 * to the sipdb in-memory databases.
 *
 * Care should be taken in managing memory - all objects inserted in the set
 * must be dynamically allocated (using new) and should be considered owned
 * by the set; see addValue.
 */
class ResultSet : public UtlSList
{
  public:
    /// Construct a container for rows.
    ResultSet();

    /// Destroy a set - implicitly destroys all contents.
    virtual ~ResultSet();

    /// Get the number of rows in the set.
    int getSize() const;

    /// Retrieve the Nth row in a set.
    OsStatus getIndex( const int& index    ///< the row to return.
                      ,UtlHashMap& rRecord ///< the returned row (must be empty when passed)
                      ) const;
    /**<
     * This makes a shallow copy, so the elements in the returned rRecord
     * must not be deleted (the values are still owned by the ResultSet).
     */

    /// Add a row to the set.
    void addValue(const UtlHashMap& record);
    /**<
     * This makes a shallow copy, so the elements in the record
     * must not be deleted (consider them owned by the ResultSet following the copy).
     * @code
     * ResultSet registrations;
     * UtlHashMap regRow;
     *
     * UtlString* uriKey = new UtlString("uri");
     * UtlString* uriValue = new UtlString(regdata[row].uri);
     * regRow.insertKeyAndValue(uriKey, uriValue);
     *
     * UtlString* callidKey = new UtlString("callid");
     * UtlString* callidValue = new UtlString(regdata[row].callid);
     * regRow.insertKeyAndValue(callidKey, callidValue);
     *
     * UtlString* contactKey = new UtlString("contact");
     * UtlString* contactValue = new UtlString(regdata[row].contact);
     * regRow.insertKeyAndValue(contactKey, contactValue);
     *
     * registrations.addValue(regRow);
     * @endcode
     * Note that the UtlHashMap record object is not dynamic; a new UtlHashMap is created in
     * and owned by the set, but the objects contained in record are not copied, so they are
     * now owned by the set.
     */

    void clear();
    /**<
     * @note
     * Do not create any additional use.  This was originally defined to remove all
     * entries.  It was probably a memory leak if there was anything in the set.
     * This is now used in only a few places where the set should already be empty,
     * and has been redefined to be just a check for emptiness.
     * @endnote
     */

    /// Deletes all objects in the set.
    void destroyAll();

  private:
    /// No copy constructor.
    ResultSet(const ResultSet& rhs);

    /// No assignment operator.
    ResultSet operator=(const ResultSet& rhs);

};

#endif //RESULTSET_H
