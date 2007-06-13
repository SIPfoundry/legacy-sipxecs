/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.permission;

import java.util.Collection;

import org.sipfoundry.sipxconfig.common.DataObjectSource;
import org.sipfoundry.sipxconfig.setting.Setting;

public interface PermissionManager extends DataObjectSource<Permission> {
    public static final String CONTEXT_BEAN_NAME = "permissionManager";

    Permission getCustomPermissionByLabel(String permissionName);

    Permission getPermission(Object id);

    Permission getPermissionByName(String permissionName);

    void addCallPermission(Permission permission);

    Collection<Permission> getCallPermissions();

    void removeCallPermissions(Collection<Integer> permissionIds);

    Setting getPermissionModel();
}
