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

public class CallerAliasesMapping extends DataSetRecord {
    public static final String DOMAIN = "dm";
    public static final String ALIAS = "als";

    public CallerAliasesMapping(String domain, String alias) {
        put(DOMAIN, domain);
        put(ALIAS, alias);
    }

}
