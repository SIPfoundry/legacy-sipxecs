/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.Map;

public interface ImdbApi {

    boolean replace(String hostname, String tableName, Map<String, String>[] records);

    Map<String, String>[] retrieve(String hostname, String tableName);
}
