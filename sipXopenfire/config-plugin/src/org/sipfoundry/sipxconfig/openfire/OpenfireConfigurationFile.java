/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.openfire;

import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.TreeSet;

import org.apache.commons.lang.StringUtils;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.bulk.ldap.AttrMap;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapConnectionParams;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapSystemSettings;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.common.AbstractUser;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;
import org.springframework.beans.factory.annotation.Required;

public class OpenfireConfigurationFile {
    private static final String SEPARATOR = ", ";

    private String m_multipleLdapConfFile;

    private Map<String, String> m_properties;
    private Map<String, String> m_nonLdapProperties;
    private Map<String, String> m_ldapProperties;

    private LdapManager m_ldapManager;
    private CoreContext m_coreContext;
    private LocalizationContext m_localizationContext;
    private VelocityEngine m_velocityEngine;

    public void writeMultipleLdapConfiguration(Writer writer) throws IOException {
        if (m_multipleLdapConfFile != null) {
            List<LdapConnectionParams> allParams = m_ldapManager.getAllConnectionParams();
            VelocityContext context = new VelocityContext();
            List<LdapData> ldapDataList = new ArrayList<LdapData>();
            for (LdapConnectionParams params : allParams) {
                ldapDataList.add(new LdapData(params, m_ldapManager.getAttrMap(params.getId())));
            }
            context.put("ldapDataList", ldapDataList);
            try {
                m_velocityEngine.mergeTemplate(m_multipleLdapConfFile, context, writer);
            } catch (Exception e) {
                throw new IOException(e);
            }
        }
    }

    public void writeOfPropertyConfig(Writer w, OpenfireSettings settings) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(w);

        config.writeSettings(settings.getOfProperty());

        for (Map.Entry<String, Object> property : buildOpenfirePropertiesMap().entrySet()) {
            config.write(property.getKey(), property.getValue());
        }
    }

    protected SortedMap<String, Object> buildOpenfirePropertiesMap() {
        SortedMap<String, Object> props = new TreeMap<String, Object>();

        props.put("admin.authorizedJIDs", getAuthorizedUsernames());
        LdapSystemSettings ldapSettings = m_ldapManager.getSystemSettings();
        boolean ldapEnabled = ldapSettings.isEnableOpenfireConfiguration() && ldapSettings.isConfigured();

        if (ldapEnabled) {
            List<LdapConnectionParams> allParams = m_ldapManager.getAllConnectionParams();
            LdapConnectionParams ldapConnectionParams = allParams.get(0);
            AttrMap attrMap = m_ldapManager.getAttrMap(ldapConnectionParams.getId());

            props.put("ldap.host", ldapConnectionParams.getHost());
            props.put("ldap.port", ldapConnectionParams.getPort());
            props.put("ldap.sslEnabled", ldapConnectionParams.getUseTls());
            props.put("ldap.baseDN", attrMap.getAttribute("searchBase"));
            props.put("ldap.usernameField", attrMap.getAttribute("imAttributeName"));
            props.put("ldap.searchFilter", attrMap.getAttribute("searchFilter"));

            boolean ldapAnonymousAccess = StringUtils.isBlank(ldapConnectionParams.getPrincipal());
            if (!ldapAnonymousAccess) {
                props.put("ldap.adminDN", ldapConnectionParams.getPrincipal());
                props.put("ldap.adminPassword", ldapConnectionParams.getSecret());
            }
            if (m_ldapProperties != null) {
                props.putAll(m_ldapProperties);
            } else {
                throw new IllegalArgumentException("LDAP properties not found");
            }
        } else {
            if (m_nonLdapProperties != null) {
                props.putAll(m_nonLdapProperties);
            }
        }
        for (Map.Entry<String, String> entry : m_properties.entrySet()) {
            props.put(entry.getKey(), entry.getValue());
        }
        props.put("locale", m_localizationContext.getCurrentLanguage());
        props.put("log.debug.enabled", false);

        return props;
    }

    /**
     * Get authorized usernames. The defaults are admin and superadmin. When you have
     * LDAP-Openfire configured different other users can be added with admin rights.
     */
    protected String getAuthorizedUsernames() {
        List<User> admins = m_coreContext.loadUserByAdmin();
        Set<String> authorizedList = new TreeSet<String>();
        authorizedList.add(AbstractUser.SUPERADMIN);
        for (User user : admins) {
            authorizedList.add(user.getUserName());
        }
        return StringUtils.join(authorizedList, SEPARATOR);
    }

    @Required
    public void setLdapManager(LdapManager ldapManager) {
        m_ldapManager = ldapManager;
    }

    protected LdapManager getLdapManager() {
        return m_ldapManager;
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setLocalizationContext(LocalizationContext localizationContext) {
        m_localizationContext = localizationContext;
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public VelocityEngine getVelocityEngine() {
        return m_velocityEngine;
    }

    public void setMultipleLdapConfFile(String multipleLdapConfFile) {
        m_multipleLdapConfFile = multipleLdapConfFile;
    }

    public void setProperties(Map<String, String> properties) {
        m_properties = properties;
    }

    public void setNonLdapProperties(Map<String, String> properties) {
        m_nonLdapProperties = properties;
    }

    public void setLdapProperties(Map<String, String> properties) {
        m_ldapProperties = properties;
    }

    public static class LdapData {
        private final LdapConnectionParams m_ldapParams;
        private final AttrMap m_attrMap;
        private final boolean m_ldapAnonymousAccess;

        public LdapData(LdapConnectionParams ldapParams, AttrMap attrMap) {
            m_ldapParams = ldapParams;
            m_attrMap = attrMap;
            m_ldapAnonymousAccess = (StringUtils.isBlank(m_ldapParams.getPrincipal())) ? true : false;
        }

        public LdapConnectionParams getLdapParams() {
            return m_ldapParams;
        }

        public AttrMap getAttrMap() {
            return m_attrMap;
        }

        public String getImAttribute() {
            String imAttribute = m_attrMap.getImAttributeName();
            String usernameAttribute = m_attrMap.getIdentityAttributeName();
            // if im id is not mapped, default it to username -
            // because this is a rule, when a user gets created the im id has to automatically be
            // defaulted to username
            return imAttribute == null ? usernameAttribute == null ? StringUtils.EMPTY : usernameAttribute
                    : imAttribute;
        }

        public String getDomain() {
            String domain = m_ldapParams.getDomain();
            return domain == null ? StringUtils.EMPTY : domain;
        }

        public boolean isLdapAnonymousAccess() {
            return m_ldapAnonymousAccess;
        }
    }
}
