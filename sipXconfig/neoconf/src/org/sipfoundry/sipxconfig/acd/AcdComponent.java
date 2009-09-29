/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd;

import java.io.Serializable;
import java.util.Collection;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public abstract class AcdComponent extends BeanWithSettings implements NamedObject {
    private CoreContext m_coreContext;

    private CallGroupContext m_callGroupContext;

    private final String m_modelFileName;

    private final String m_groupName;

    private String m_description;

    private String m_name;

    public AcdComponent(String modelFileName, String groupName) {
        m_modelFileName = modelFileName;
        m_groupName = groupName;
    }

    protected Setting loadSettings() {
        Setting settings = getModelFilesContext().loadModelFile(m_modelFileName, "acd");
        return settings;
    }

    public CoreContext getCoreContext() {
        return m_coreContext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public CallGroupContext getCallGroupContext() {
        return m_callGroupContext;
    }

    public void setCallGroupContext(CallGroupContext callGroupContext) {
        m_callGroupContext = callGroupContext;
    }

    boolean create(XmlRpcSettings xmlRpc) {
        Setting defaultGroup = getProvisioningSettings();
        return xmlRpc.create(defaultGroup);
    }

    boolean set(XmlRpcSettings xmlRpc) {
        Setting defaultGroup = getProvisioningSettings();
        return xmlRpc.set(defaultGroup);
    }

    boolean smartCreate(XmlRpcSettings xmlRpc, boolean exist) {
        Setting defaultGroup = getProvisioningSettings();
        return exist ? xmlRpc.set(defaultGroup) : xmlRpc.create(defaultGroup);
    }

    boolean delete(XmlRpcSettings xmlRpc) {
        Setting defaultGroup = getProvisioningSettings();
        return xmlRpc.delete(defaultGroup);

    }

    boolean deleteAll(XmlRpcSettings xmlRpc, String objectClass, Collection uris, String key) {
        return xmlRpc.deleteAll(objectClass, uris, key);
    }

    Set getAll(XmlRpcSettings xmlRpc, String className, String key) {
        return xmlRpc.getAll(className, key);
    }

    protected Setting getProvisioningSettings() {
        Setting settings = getSettings();
        Setting defaultGroup = settings.getSetting(m_groupName);
        return defaultGroup;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    /**
     * Calculated URI for this object using injected core context and object name
     */
    public String calculateUri() {
        String domainName = getCoreContext().getDomainName();
        return SipUri.format(getName(), domainName, false);
    }

    public abstract Serializable getAcdServerId();
}
