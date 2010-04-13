/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.api;

import java.rmi.RemoteException;
import java.util.Collection;
import java.util.Collections;
import java.util.Set;

import org.sipfoundry.sipxconfig.permission.PermissionManager;

public class PermissionServiceImpl implements PermissionService {

    private PermissionManager m_permissionManager;

    private PermissionBuilder m_builder;

    public void setPermissionManager(PermissionManager permissionManager) {
        m_permissionManager = permissionManager;
    }

    public void setPermissionBuilder(PermissionBuilder builder) {
        m_builder = builder;
    }

    public void addPermission(AddPermission addPermission) throws RemoteException {
        org.sipfoundry.sipxconfig.permission.Permission myPermission =
            new org.sipfoundry.sipxconfig.permission.Permission();
        Permission apiPermission = addPermission.getPermission();
        ApiBeanUtil.toMyObject(m_builder, myPermission, apiPermission);

        m_permissionManager.addCallPermission(myPermission);
    }

    public FindPermissionResponse findPermission(FindPermission findPermission) throws RemoteException {
        FindPermissionResponse response = new FindPermissionResponse();
        org.sipfoundry.sipxconfig.permission.Permission[] myPermissions = permissionSearch(findPermission.getSearch());
        Permission[] arrayOfPermissions =
            (Permission[]) ApiBeanUtil.toApiArray(m_builder, myPermissions, Permission.class);
        response.setPermissions(arrayOfPermissions);

        return response;
    }

    org.sipfoundry.sipxconfig.permission.Permission[] permissionSearch(PermissionSearch search) {
        Collection<org.sipfoundry.sipxconfig.permission.Permission> permissions = Collections.EMPTY_LIST;
        if (search == null) {
            permissions = m_permissionManager.getPermissions();
        } else if (search.getByName() != null) {
            // non-empty list of permissions
            org.sipfoundry.sipxconfig.permission.Permission permission =
                m_permissionManager.getPermissionByName(search.getByName());
            if (permission != null) {
                permissions  = Collections.singletonList(permission);
            }
        } else if (search.getByLabel() != null) {
            // non-empty list of permissions
            org.sipfoundry.sipxconfig.permission.Permission permission =
                m_permissionManager.getPermissionByLabel(search.getByLabel());
            if (permission != null) {
                permissions  = Collections.singletonList(permission);
            }
        }

        return permissions.toArray(new org.sipfoundry.sipxconfig.permission.Permission[permissions.size()]);
    }

    public void managePermission(ManagePermission managePermission) throws RemoteException {
        org.sipfoundry.sipxconfig.permission.Permission[] myPermissions =
            permissionSearch(managePermission.getSearch());
        for (int i = 0; i < myPermissions.length; i++) {
            org.sipfoundry.sipxconfig.permission.Permission p =
                m_permissionManager.getPermissionByName(myPermissions[i].getName());
            if (p == null) {
                throw new IllegalArgumentException("Permission not found " + myPermissions[i].getName());
            }
            if (p.isBuiltIn()) {
                // Can not modify builtIn permission
                // Do not throw, we allow bulk modification for all custom permissions
                // with and empty search block
                continue;
            }
            if (Boolean.TRUE.equals(managePermission.getDeletePermission())) {
                m_permissionManager.removeCallPermissions(Collections.singletonList(p.getId()));
                continue; // all other edits wouldn't make sense
            }

            if (managePermission.getEdit() != null) {
                Permission apiPermission = new Permission();
                Set properties = ApiBeanUtil.getSpecifiedProperties(managePermission.getEdit());
                ApiBeanUtil.setProperties(apiPermission, managePermission.getEdit());
                m_builder.toMyObject(p, apiPermission, properties);
                m_permissionManager.addCallPermission(p);
            }
        }
    }
}
