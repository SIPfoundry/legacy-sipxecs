/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;

import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.admin.commserver.ConflictingFeatureCodeValidator;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxPresenceService extends SipxService implements AliasProvider {
    public static final String BEAN_ID = "sipxPresenceService";
    
    private static final String PRESENCE_SIGN_IN_CODE = "presence-config/SIP_PRESENCE_SIGN_IN_CODE";
    private static final String PRESENCE_SIGN_OUT_CODE = "presence-config/SIP_PRESENCE_SIGN_OUT_CODE";
    private static final String PRESENCE_SERVER_SIP_PORT = "presence-config/PRESENCE_SERVER_SIP_PORT";
    // note: the name of the setting is misleading - this is actually full host name not just a
    // domain name
    private static final String PRESENCE_SERVER_LOCATION = "presence-config/SIP_PRESENCE_DOMAIN_NAME";
    private static final String PRESENCE_API_PORT = "presence-config/SIP_PRESENCE_HTTP_PORT";
    
    private SipxServiceManager m_sipxServiceManager;
    private CoreContext m_coreContext;
    
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }
    
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
    
    /**
     * Validates the data in this service and throws a UserException if there is a problem
     */
    public void validate() {
        SipxService registrarService = m_sipxServiceManager.getServiceByBeanId(SipxRegistrarService.BEAN_ID);
        new ConflictingFeatureCodeValidator().validate(Arrays.asList(new Setting[] {
            getSettings(), registrarService.getSettings()
        }));
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
}
