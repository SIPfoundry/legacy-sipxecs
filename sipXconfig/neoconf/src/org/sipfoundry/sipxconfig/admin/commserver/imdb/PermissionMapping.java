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

public class PermissionMapping extends DataSetRecord {
    public static final String PERMISSION = "permission";

    public PermissionMapping(String permissionName) {
        put(PERMISSION, permissionName);
    }
}
