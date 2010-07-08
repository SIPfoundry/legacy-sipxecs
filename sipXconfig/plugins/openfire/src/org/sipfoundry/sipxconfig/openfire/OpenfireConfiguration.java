/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.openfire;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.TemplateConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.annotation.Required;

public class OpenfireConfiguration extends TemplateConfigurationFile {

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

    private LdapManager m_ldapManager;

    private SipxServiceManager m_sipxServiceManager;

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        boolean isEnableOpenfireConfiguration = m_ldapManager.getSystemSettings().isEnableOpenfireConfiguration();
        context.put("isEnableOpenfireConfiguration", isEnableOpenfireConfiguration);
        if (!isEnableOpenfireConfiguration) {
            context.put("adminProvider", PROVIDER_ADMIN_CLASSNAME);
            context.put("authProvider", PROVIDER_AUTH_CLASSNAME);
            context.put("groupProvider", PROVIDER_GROUP_CLASSNAME);
            context.put("userProvider", PROVIDER_USER_CLASSNAME);
            context.put("lockoutProvider", PROVIDER_LOCKOUT_CLASSNAME);
            context.put("securityAuditProvider", PROVIDER_SECURITY_AUDIT_CLASSNAME);
            context.put("sipxVcardProvider", PROVIDER_SIPX_VCARD_CLASSNAME);
        } else {
            context.put("ldapParams", m_ldapManager.getConnectionParams());
            context.put("attrMap", m_ldapManager.getAttrMap());
            context.put("ldapAuthProvider", PROVIDER_LDAP_AUTH_CLASSNAME);
            context.put("ldapUserProvider", PROVIDER_LDAP_USER_CLASSNAME);
            context.put("ldapVcardProvider", PROVIDER_LDAP_VCARD_CLASSNAME);
        }

        return context;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Required
    public void setLdapManager(LdapManager ldapManager) {
        m_ldapManager = ldapManager;
    }

    @Override
    public boolean isReplicable(Location location) {
        return m_sipxServiceManager.isServiceInstalled(location.getId(), SipxOpenfireService.BEAN_ID);
    }
}
