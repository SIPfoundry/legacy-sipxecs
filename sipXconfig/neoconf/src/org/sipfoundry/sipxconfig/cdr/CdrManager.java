/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cdr;

import java.io.IOException;
import java.io.Writer;
import java.util.Date;
import java.util.List;

import org.sipfoundry.sipxconfig.common.User;

public interface CdrManager {
    final String CONTEXT_BEAN_NAME = "cdrManager";

    /**
     * Retrieve CDRS between from and to dates
     *
     * @param from date of first CDR retrieved, pass null for oldest
     * @param to date of the last CDR retrieved, pass null for latest
     * @param search specification - enumeration representing columns and string to search for
     * @return list of CDR objects
     */
    List<Cdr> getCdrs(Date from, Date to, CdrSearch search, User user);
    List<Cdr> getCdrs(Date from, Date to, CdrSearch search, User user, int limit, int offset);


    /**
     * @param from date of first CDR retrieved, pass null for oldest
     * @param to date of the last CDR retrieved, pass null for latest
     * @param search specification - enumeration representing columns and string to search for
     * @return number of CDRs that fullfil passed criteria
     */
    int getCdrCount(Date from, Date to, CdrSearch search, User user);

    /**
     * Dumps CDRs in comma separated values format.
     *
     * @param writer CSV stream destination
     * @param from date of first CDR retrieved, pass null for oldest
     * @param to date of the last CDR retrieved, pass null for latest
     * @param search specification - enumeration representing columns and string to search for
     */
    void dumpCdrs(Writer writer, Date from, Date to, CdrSearch search, User user) throws IOException;

    /**
     * Dump CDRs in a JSON format used by Exhibit platform.
     *
     * @param out destination for JSON stream
     */
    void dumpCdrsJson(Writer out) throws IOException;


    /**
     * Returns the list of active calls as CDRs
     */
    List<Cdr> getActiveCalls();
}
