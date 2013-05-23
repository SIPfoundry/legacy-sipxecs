/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.bulk.ldap;

import java.io.Writer;
import java.util.List;

import org.sipfoundry.sipxconfig.bulk.UserPreview;

public interface LdapImportManager {
    void insert(int connectionId);

    /**
     * Retrieve an example of the user and its groups from currently configured LDAP
     *
     * @param user object to be filled with imported data
     * @param groupNames collection of group names created for this user
     */
    List<UserPreview> getExample(int connectionId);

    void dumpExample(List<UserPreview> list, Writer output, int connectionId);
}
