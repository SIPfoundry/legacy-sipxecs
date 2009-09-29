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
import org.springframework.ldap.LdapTemplate;

public class LdapTemplateFactory implements ApplicationContextAware {
    public static final String LDAP_TEMPLATE_BEAN_ID = "ldapTemplate";
    private static final String LDAP_CONTEXT_SOURCE_BEAN_ID = "ldapContextSource";
    private ApplicationContext m_applicationContext;

    /**
     * access to spring's enhanced API into LDAP
     * @return new instance every call
     */
    public LdapTemplate getLdapTemplate() {
        return (LdapTemplate) m_applicationContext.getBean(LDAP_TEMPLATE_BEAN_ID);
    }

    /**
     * useful for testing a connection by getting access to API  before saving
     * connection params
     *
     * @return new instance every call
     */
    public LdapTemplate getLdapTemplate(LdapConnectionParams params) {
        LdapTemplate template = getLdapTemplate();
        ContextSourceFromConnectionParams source = (ContextSourceFromConnectionParams)
            m_applicationContext.getBean(LDAP_CONTEXT_SOURCE_BEAN_ID);
        params.applyToContext(source);
        source.applyParameters(params);
        template.setContextSource(source);
        return template;
    }

    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }
}
