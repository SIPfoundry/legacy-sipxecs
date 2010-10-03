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
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class PermissionManagerImpl extends SipxHibernateDaoSupport<Permission> implements
        PermissionManager {

    private ModelFilesContext m_modelFilesContext;

    private SipxReplicationContext m_replicationContext;

    private SipxServiceManager m_sipxServiceManager;

    public void addCallPermission(Permission permission) {
        if (isLabelInUse(permission)) {
            throw new DuplicatePermissionLabelException(permission.getLabel());
        }

        getHibernateTemplate().saveOrUpdate(permission);
        m_replicationContext.generate(DataSet.PERMISSION);
    }

    public void removeCallPermissions(Collection<Integer> permissionIds) {
        removeAll(Permission.class, permissionIds);
        m_replicationContext.generate(DataSet.PERMISSION);
    }

    public Permission getCallPermission(Object id) {
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

    public Permission getPermissionByName(Permission.Type type, String permissionName) {

        Map<String, Permission> builtInPermissions = getBuiltInPermissionsByType(type);
        Permission permission = builtInPermissions.get(permissionName);
        if (permission != null) {
            return permission;
        }

        // There exist only Call-Handling custom permissions
        if (Permission.Type.CALL.getName().equals(type.getName())) {
            Collection<Permission> custom = loadCustomPermissions();
            for (Permission customPermission : custom) {
                if (customPermission.getName().equals(permissionName)) {
                    return customPermission;
                }
            }
        }
        return null;
    }

    public Permission getPermissionByLabel(String permissionLabel) {
        Map<String, Permission> builtInPermissions = getBuiltInPermissions();
        // Build-in permissions have the name equal to label
        Permission permission = builtInPermissions.get(permissionLabel);
        if (permission != null) {
            return permission;
        }
        Collection<Permission> custom = loadCustomPermissions();
        for (Permission customPermission : custom) {
            if (customPermission.getLabel().equals(permissionLabel)) {
                return customPermission;
            }
        }
        return null;
    }

    public Permission getPermissionByLabel(Permission.Type type, String permissionLabel) {

        Map<String, Permission> builtInPermissions = getBuiltInPermissionsByType(type);
        // Build-in permissions have the name equal to label
        Permission permission = builtInPermissions.get(permissionLabel);
        if (permission != null) {
            return permission;
        }

        // There exist only Call-Handling custom permissions
        if (Permission.Type.CALL.getName().equals(type.getName())) {
            Collection<Permission> custom = loadCustomPermissions();
            for (Permission customPermission : custom) {
                if (customPermission.getLabel().equals(permissionLabel)) {
                    return customPermission;
                }
            }
        }
        return null;
    }

    public Permission load(Class<Permission> c, Serializable id) {
        if (id instanceof String) {
            Map<String, Permission> builtInPermissions = getBuiltInCallPermissions();
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

    private Map<String, Permission> getBuiltInPermissionsByType(Permission.Type theType) {
        Setting userSettingsModel = loadSettings();
        Map<String, Permission> permissions = new TreeMap<String, Permission>();
        Setting theGroup = userSettingsModel.getSetting(theType.getPath());
        for (Setting setting : theGroup.getValues()) {
            permissions.put(setting.getName(), new SettingPermission(setting, theType, true));
        }

        return permissions;
    }

    /**
     * Loads permission settings and creates Permission collection of built-in call-handling
     * permissions
     */
    private Map<String, Permission> getBuiltInCallPermissions() {
        return getBuiltInPermissionsByType(Permission.Type.CALL);
    }

    /**
     * Loads permission settings and creates Permission collection of built-in application
     * permissions
     */
    private Map<String, Permission> getBuiltInApplicationPermissions() {
        return getBuiltInPermissionsByType(Permission.Type.APPLICATION);
    }

    /**
     * Loads permission settings and creates Permission collection of built-in call-handling
     * permissions
     */
    private Map<String, Permission> getBuiltInVMServerPermissions() {
        return getBuiltInPermissionsByType(Permission.Type.VOICEMAIL_SERVER);
    }

    /**
     * Creates Permission collection of built-in call-handling and application permissions
     */
    private Map<String, Permission> getBuiltInPermissions() {
        Map<String, Permission> permissions = getBuiltInCallPermissions();
        permissions.putAll(getBuiltInApplicationPermissions());
        permissions.putAll(getBuiltInVMServerPermissions());
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

    public Collection<Permission> getPermissions() {

        Set<Permission> permissions = new TreeSet<Permission>();
        permissions.addAll(getBuiltInPermissions().values());
        permissions.addAll(loadCustomPermissions());
        return permissions;
    }

    public Collection<Permission> getPermissions(Permission.Type type) {

        Set<Permission> permissions = new TreeSet<Permission>();
        permissions.addAll(getBuiltInPermissionsByType(type).values());

        // There exist only Call-Handling custom permissions
        if (Permission.Type.CALL.getName().equals(type.getName())) {
            permissions.addAll(loadCustomPermissions());
        }
        return permissions;
    }

    public Collection<Permission> getCallPermissions() {

        return getPermissions(Permission.Type.CALL);
    }

    private Setting loadSettings() {
        return m_modelFilesContext.loadModelFile("commserver/user-settings.xml");
    }

    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    public void setModelFilesContext(ModelFilesContext modelFilesContext) {
        m_modelFilesContext = modelFilesContext;
    }

    public String getDefaultInitDelay() {
        SipxService proxyService = m_sipxServiceManager.getServiceByBeanId(SipxProxyService.BEAN_ID);
        Setting proxySettings = proxyService.getSettings().getSetting("proxy-configuration");
        return proxySettings.getSetting("SIPX_PROXY_DEFAULT_SERIAL_EXPIRES").getValue();
    }

    private boolean isLabelInUse(Permission permission) {
        List count = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "anotherPermissionWithTheSameLabel", new String[] {
                    "id", "label"
                }, new Object[] {
                    permission.getId(), permission.getLabel()
                });

        return DataAccessUtils.intResult(count) > 0;
    }

    private static class DuplicatePermissionLabelException extends UserException {
        private static final String ERROR = "Permission {0} was already defined. Please choose another name.";

        public DuplicatePermissionLabelException(String label) {
            super(ERROR, label);
        }
    }

    /**
     * Remove all custom permissions - mostly used for testing
     */
    public void clear() {
        HibernateTemplate template = getHibernateTemplate();
        Collection<Permission> permissions = loadCustomPermissions();
        template.deleteAll(permissions);
    }
}
