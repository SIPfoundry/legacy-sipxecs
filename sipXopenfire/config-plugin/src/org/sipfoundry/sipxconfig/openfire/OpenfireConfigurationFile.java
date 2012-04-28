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
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import org.apache.commons.lang.StringUtils;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapConnectionParams;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapSystemSettings;
import org.sipfoundry.sipxconfig.common.AbstractUser;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.beans.factory.annotation.Required;

public class OpenfireConfigurationFile {
    private static final String PROVIDER_ADMIN_CLASSNAME = "org.jivesoftware.openfire.admin.DefaultAdminProvider";
    private static final String PROVIDER_AUTH_CLASSNAME = "org.jivesoftware.openfire.auth.DefaultAuthProvider";
    private static final String PROVIDER_GROUP_CLASSNAME = "org.jivesoftware.openfire.group.DefaultGroupProvider";
    private static final String PROVIDER_USER_CLASSNAME = "org.jivesoftware.openfire.user.DefaultUserProvider";
    private static final String PROVIDER_LOCKOUT_CLASSNAME = "org.jivesoftware.openfire.lockout.DefaultLockOutProvider";
    private static final String PROVIDER_SECURITY_AUDIT_CLASSNAME =
        "org.jivesoftware.openfire.security.DefaultSecurityAuditProvider";
    private static final String PROVIDER_SIPX_VCARD_CLASSNAME =
        "org.sipfoundry.openfire.vcard.provider.SipXVCardProvider";
    private static final String PROVIDER_LDAP_AUTH_CLASSNAME = "org.jivesoftware.openfire.ldap.LdapAuthProvider";
    private static final String PROVIDER_LDAP_USER_CLASSNAME = "org.jivesoftware.openfire.ldap.LdapUserProvider";
    private static final String PROVIDER_LDAP_VCARD_CLASSNAME = "org.jivesoftware.openfire.ldap.LdapVCardProvider";
    private static final String SEPARATOR = ", ";
    private static final String ADMIN = "admin";

    private LdapManager m_ldapManager;
    private CoreContext m_coreContext;
    private VelocityEngine m_velocityEngine;

    public void write(Writer writer) throws IOException {
        VelocityContext context = new VelocityContext();
        LdapSystemSettings settings = m_ldapManager.getSystemSettings();
        boolean isEnableOpenfireConfiguration = settings.isEnableOpenfireConfiguration() && settings.isConfigured();
        context.put("isEnableOpenfireConfiguration", isEnableOpenfireConfiguration);
        List<LdapConnectionParams> allParams = m_ldapManager.getAllConnectionParams();
        if (!isEnableOpenfireConfiguration || allParams == null || allParams.isEmpty()) {
            context.put("adminProvider", PROVIDER_ADMIN_CLASSNAME);
            context.put("authProvider", PROVIDER_AUTH_CLASSNAME);
            context.put("groupProvider", PROVIDER_GROUP_CLASSNAME);
            context.put("userProvider", PROVIDER_USER_CLASSNAME);
            context.put("lockoutProvider", PROVIDER_LOCKOUT_CLASSNAME);
            context.put("securityAuditProvider", PROVIDER_SECURITY_AUDIT_CLASSNAME);
            context.put("sipxVcardProvider", PROVIDER_SIPX_VCARD_CLASSNAME);
        } else if (allParams != null && !allParams.isEmpty()) {
            LdapConnectionParams ldapConnectionParams = allParams.get(0);
            boolean isLdapAnonymousAccess = (StringUtils.isBlank(ldapConnectionParams.getPrincipal())) ? true
                    : false;
            context.put("isLdapAnonymousAccess", isLdapAnonymousAccess);
            context.put("ldapParams", ldapConnectionParams);
            context.put("attrMap", m_ldapManager.getAttrMap(ldapConnectionParams.getId()));
            context.put("ldapAuthProvider", PROVIDER_LDAP_AUTH_CLASSNAME);
            context.put("ldapUserProvider", PROVIDER_LDAP_USER_CLASSNAME);
            context.put("ldapVcardProvider", PROVIDER_LDAP_VCARD_CLASSNAME);
        }

        context.put("authorizedUsernames", getAuthorizedUsernames());

        try {
            m_velocityEngine.mergeTemplate("openfire/openfire.vm", context, writer);
        } catch (Exception e) {
            throw new IOException(e);
        }
    }

    /**
     * Get authorized usernames. The defaults are admin and superadmin.
     * When you have LDAP-Openfire configured different other users
     * can be added with admin rights.
     */
    private String getAuthorizedUsernames() {
        List<User> admins = m_coreContext.loadUserByAdmin();
        Set<String> authorizedList = new TreeSet<String>();
        authorizedList.add(ADMIN);
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

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }
}
