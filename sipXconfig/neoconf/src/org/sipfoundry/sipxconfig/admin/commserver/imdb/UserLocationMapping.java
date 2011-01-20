/*
 *
 *
 * Copyright (C) 2011 eZuce inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;


public class UserLocationMapping extends DataSetRecord {

    public UserLocationMapping(String url, String siteName) {
        put("identity", url);
        put("location", siteName);
    }

}
