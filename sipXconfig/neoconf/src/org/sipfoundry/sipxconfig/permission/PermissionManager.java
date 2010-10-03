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
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * PermissionManger interface provides access to user/usergroup Permissions.
 *
 * There are three types of Permissions: Application, VMServer and CallHandling. CallHandling
 * permissions are used in coordination with dial plan rules to implement system dialing
 * restrictions. CallHandling permissions are further classified as built in and custom, i.e.
 * created by the administrator. Compared to built-in permissions, which are statically defined in
 * permission model, custom permissions can be aded/modified/removed by the administor Application
 * permissions control non-dialplan related priveledges of users/usergroups. VMServer permissions
 * control which VM server hosts the user VM box. Only built-in application permissions are
 * currently supported.
 */
public interface PermissionManager extends DataObjectSource<Permission> {
    public static final String CONTEXT_BEAN_NAME = "permissionManager";

    Permission getCallPermission(Object id);

    Collection<Permission> getCallPermissions();

    /*
     * Calling addCallPermission on existing call Permission object updates the object properties
     */
    void addCallPermission(Permission permission);

    void removeCallPermissions(Collection<Integer> permissionIds);

    Permission getPermissionByName(String permissionName);

    Permission getPermissionByName(Permission.Type type, String permissionName);

    Permission getPermissionByLabel(String permissionLabel);

    Permission getPermissionByLabel(Permission.Type type, String permissionLabel);

    Collection<Permission> getPermissions();

    Collection<Permission> getPermissions(Permission.Type type);

    Setting getPermissionModel();

    String getDefaultInitDelay();

    /*
     * Remove all custom permissions
     */
    void clear();

    //used only for tests
    void setModelFilesContext(ModelFilesContext modelFilesContext);
}
