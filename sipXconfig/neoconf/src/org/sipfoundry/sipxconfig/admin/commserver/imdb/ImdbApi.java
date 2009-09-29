/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.List;
import java.util.Map;

public interface ImdbApi {

    boolean replace(String hostname, String tableName, Map<String, ? >[] records);

    List<Map<String, ? >> read(String hostname, String tableName);
}
