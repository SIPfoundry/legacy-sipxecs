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

import java.io.Serializable;
import java.util.Collection;
import java.util.Collections;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.setting.Setting;

public class PermissionManagerImpl extends SipxHibernateDaoSupport<Permission> implements
        PermissionManager {

    private ModelFilesContext m_modelFilesContext;

    private SipxReplicationContext m_replicationContext;

    public void addCallPermission(Permission permission) {
        getHibernateTemplate().saveOrUpdate(permission);
        m_replicationContext.generate(DataSet.PERMISSION);
    }

    public Collection<Permission> getCallPermissions() {
        return getAllPermissions();
    }

    public void removeCallPermissions(Collection<Integer> permissionIds) {
        removeAll(Permission.class, permissionIds);
        m_replicationContext.generate(DataSet.PERMISSION);
    }

    public Permission getPermission(Object id) {
        return load(Permission.class, (Serializable) id);
    }

    public Permission getPermissionByName(String permissionName) {
        Map<String, Permission> builtInPermissions = getBuiltInPermissions();
        Permission permission = builtInPermissions.get(permissionName);
        if (permission != null) {
            return permission;
        }
        Collection<Permission> custom = loadCustomPermissions();
        for (Permission customPermission : custom) {
            if (customPermission.getName().equals(permissionName)) {
                return customPermission;
            }
        }
        return null;
    }

    public Permission load(Class<Permission> c, Serializable id) {
        if (id instanceof String) {
            Map<String, Permission> builtInPermissions = getBuiltInPermissions();
            return builtInPermissions.get(id);
        }
        return super.load(c, id);
    }

    /**
     * Combine built-in and custom permissions into a single permission model
     */
    public Setting getPermissionModel() {
        Setting settings = loadSettings();
        Setting callHandlingGroup = settings.getSetting(Permission.CALL_PERMISSION_PATH);
        Collection<Permission> callPermissions = loadCustomPermissions();
        for (Permission permission : callPermissions) {
            callHandlingGroup.addSetting(permission.getSetting());
        }
        return settings;
    }

    /**
     * Loads permission settings and creates Permissions collection
     */
    private Map<String, Permission> getBuiltInPermissions() {
        Setting userSettingsModel = loadSettings();
        Map<String, Permission> permissions = new TreeMap<String, Permission>();
        Setting callHandlingGroup = userSettingsModel.getSetting(Permission.CALL_PERMISSION_PATH);
        for (Setting setting : callHandlingGroup.getValues()) {
            permissions.put(setting.getName(), new SettingPermission(setting));
        }
        return permissions;
    }

    /**
     * Load all permissions from the DB
     * 
     * HACK: In order to make this function work in non-DB test we return empty set of permissions
     * in case hibernate session factory is not set.
     */
    private Collection<Permission> loadCustomPermissions() {
        if (getSessionFactory() != null) {
            return getHibernateTemplate().loadAll(Permission.class);
        }
        return Collections.emptyList();
    }

    private Set<Permission> getAllPermissions() {
        Set<Permission> permissions = new TreeSet<Permission>();
        permissions.addAll(getBuiltInPermissions().values());
        permissions.addAll(loadCustomPermissions());
        return permissions;
    }

    private Setting loadSettings() {
        return m_modelFilesContext.loadModelFile("commserver/user-settings.xml");
    }

    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    public void setModelFilesContext(ModelFilesContext modelFilesContext) {
        m_modelFilesContext = modelFilesContext;
    }
}
