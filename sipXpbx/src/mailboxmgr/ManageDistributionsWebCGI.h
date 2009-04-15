//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef MANAGEDISTRIBUTIONSWEBCGI_H
#define MANAGEDISTRIBUTIONSWEBCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "mailboxmgr/VXMLCGICommand.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlHashMap;

class ManageDistributionsWebCGI : public VXMLCGICommand
{
public:
    /**
     * Ctor
     */
    ManageDistributionsWebCGI ( const UtlString& mailboxIdentity,
                                const UtlString& action,
                                const UtlString& status,
                                const UtlString& index,
                                const UtlString& distribution
                              );

    /**
     * Virtual Dtor
     */
    virtual ~ManageDistributionsWebCGI();

    /** This does the work */
    virtual OsStatus execute ( UtlString* out = NULL );

    /**
     *  Code for displaying the main webpage listing all the distribution lists,
     *  link for editing/deleting existing distribution and
     *  adding new distributions.
     *
     */
    OsStatus getManageDistributionsUI(UtlString* out) ;

    /**
     *  Constructs the HTML code for displaying an individual row on the main page
     *  for the specified distribution.
     *
     *  @param  index           Content for the field 'Index'
     *  @param  distribution    Content for the field 'Distribution'.
     *  @param  htmlCode        Html code for displaying the row. Filled on return
     *
     *  @return OS_SUCCESS or OS_FAILED
     */
    OsStatus getDistributionUIHtmlCode(  const UtlString& index,
                                         UtlSortedList& distribution,
                                         UtlString& htmlCode) const ;

    /**
     *  Constructs the HTML page for displaying a form for adding new / editing
     *  existing distribution.
     *
     *  @param  out     Contains the HTML code. Filled on return.
     *  @return OS_SUCCESS or OS_FAILED
     */
    OsStatus getAddEditDistributionsUI(UtlString* out) const;

    /**
     *  Calls the appropriate method in MailboxManager for
     *  adding / editing / deleting a distribution.
     *  Constructs HTML code for redirecting to the appropriate page
     *  and sends appropriate status based on the outcome of the operation requested.
     *
     *  @param  out     Contains the HTML code. Filled on return.
     *  @return OS_SUCCESS or OS_FAILED
     */
    OsStatus addEditDeleteDistribution(UtlString* out) const ;

    /** Utility method for saving the distribution.
     *
     *  @return     OS_SUCCESS      Address is valid
     *  @return     OS_INVALID      Address is invalid
     */
    OsStatus saveDistribution() const ;

    /** Utility method for validating the address
     *
     *  @return     OS_SUCCESS      Address is valid
     *  @return     OS_INVALID      Address is invalid
     */
    OsStatus validateAddress(const UtlString& address) const ;

protected:

private:
    UtlString m_mailboxIdentity;
    const UtlString m_action ;
    const UtlString m_status ;
    const UtlString m_index ;
    const UtlString m_distribution ;

    UtlString m_cgiUrl ;
    UtlString m_filename ;
};

#endif //MANAGEDISTRIBUTIONSWEBCGI_H
