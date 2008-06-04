/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.IOException;
import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;

import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.ConfigFileStorage;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxServer extends BeanWithSettings implements Server, AliasProvider {
    private static final String DOMAIN_NAME = "domain/SIPXCHANGE_DOMAIN_NAME";
    private static final String PRESENCE_SIGN_IN_CODE = "presence/SIP_PRESENCE_SIGN_IN_CODE";
    private static final String PRESENCE_SIGN_OUT_CODE = "presence/SIP_PRESENCE_SIGN_OUT_CODE";
    private static final String PRESENCE_SERVER_SIP_PORT = "presence/PRESENCE_SERVER_SIP_PORT";
    // note: the name of the setting is misleading - this is actually full host name not just a
    // domain name
    private static final String PRESENCE_SERVER_LOCATION = "presence/SIP_PRESENCE_DOMAIN_NAME";
    private static final String PRESENCE_API_PORT = "presence/SIP_PRESENCE_HTTP_PORT";
    private static final String PAGING_LOG_LEVEL = "Logging/log.level";

    private String m_configDirectory;
    private ConfigFileStorage m_storage;
    private SipxReplicationContext m_sipxReplicationContext;
    private CoreContext m_coreContext;
    private String m_mohUser;
    private SipxServiceManager m_sipxServiceManager;

    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setMohUser(String mohUser) {
        m_mohUser = mohUser;
    }

    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("server.xml", "commserver");
    }

    /**
     * Still need to call <code>applySettings</code> save to send to disk
     */
    public void setDomainName(String domainName) {
        setSettingValue(DOMAIN_NAME, domainName);
    }

    public void applySettings() {
        SipxService registrarService = m_sipxServiceManager
                .getServiceByBeanId(SipxRegistrarService.BEAN_ID);
        new ConflictingFeatureCodeValidator().validate(Arrays.asList(new Setting[] {
            getSettings(), registrarService.getSettings()
        }));
        try {
            m_storage.flush();
            handlePossiblePresenceServerChange();
        } catch (IOException e) {
            // TODO: catch and report as User Exception
            throw new RuntimeException(e);
        }
    }

    public void resetSettings() {
        m_storage.reset();
    }

    private void handlePossiblePresenceServerChange() {
        // TODO: in reality only need to do that if sing-in/sign-out code changed
        m_sipxReplicationContext.generate(DataSet.ALIAS);
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
        m_storage = new ConfigFileStorage(m_configDirectory);
        setValueStorage(m_storage);
    }

    public Collection getAliasMappings() {
        Collection aliases = new ArrayList();
        String domainName = m_coreContext.getDomainName();
        int presencePort = getPresenceServerPort();
        String signInCode = getSettingValue(PRESENCE_SIGN_IN_CODE);
        String signOutCode = getSettingValue(PRESENCE_SIGN_OUT_CODE);
        String presenceServer = getPresenceServerLocation();

        aliases.add(createPresenceAliasMapping(signInCode.trim(), domainName, presenceServer,
                presencePort));
        aliases.add(createPresenceAliasMapping(signOutCode.trim(), domainName, presenceServer,
                presencePort));

        return aliases;
    }

    public String getPresenceServiceUri() {
        Object[] params = new Object[] {
            getPresenceServerLocation(), String.valueOf(getPresenceServerApiPort())
        };
        return MessageFormat.format("http://{0}:{1}/RPC2", params);
    }

    private String getPresenceServerLocation() {
        return getSettingValue(PRESENCE_SERVER_LOCATION);
    }

    private int getPresenceServerApiPort() {
        return (Integer) getSettingTypedValue(PRESENCE_API_PORT);
    }

    private int getPresenceServerPort() {
        return (Integer) getSettingTypedValue(PRESENCE_SERVER_SIP_PORT);
    }

    private AliasMapping createPresenceAliasMapping(String code, String domainName,
            String presenceServer, int port) {
        AliasMapping mapping = new AliasMapping();
        mapping.setIdentity(AliasMapping.createUri(code, domainName));
        mapping.setContact(SipUri.format(code, presenceServer, port));
        return mapping;
    }

    public String getPresenceServerUri() {
        return SipUri.format(getPresenceServerLocation(), getPresenceServerPort());
    }

    public String getMusicOnHoldUri(String domainName) {
        return SipUri.format(m_mohUser, domainName, false);
    }

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    public String getPagingLogLevel() {
        return getSettingValue(PAGING_LOG_LEVEL);
    }
}
