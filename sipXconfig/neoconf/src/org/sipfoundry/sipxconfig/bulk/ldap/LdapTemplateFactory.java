/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.bulk.ldap;

import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.ldap.core.LdapTemplate;
import org.springframework.ldap.core.support.SingleContextSource;

public class LdapTemplateFactory implements ApplicationContextAware {
    public static final String LDAP_TEMPLATE_BEAN_ID = "ldapTemplate";
    private static final String LDAP_CONTEXT_SOURCE_BEAN_ID = "ldapContextSource";
    private ApplicationContext m_applicationContext;

    /**
     * useful for testing a connection by getting access to API  before saving
     * connection params
     *
     * @return new instance every call
     */
    public LdapTemplate getLdapTemplate(LdapConnectionParams params) {
        LdapTemplate template = new LdapTemplate();
        ContextSourceFromConnectionParams source = new ContextSourceFromConnectionParams();
        params.applyToContext(source);
        source.applyParameters(params);
        template.setContextSource(new SingleContextSource(source.getReadOnlyContext()));
        return template;
    }

    @Override
    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }
}
