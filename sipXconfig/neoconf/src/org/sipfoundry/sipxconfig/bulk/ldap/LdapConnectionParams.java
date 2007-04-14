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

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.CronSchedule;
import org.sipfoundry.sipxconfig.common.BeanWithId;

/**
 * Used to store LDAP connections in the DB LdapConnectionParams
 */
public class LdapConnectionParams extends BeanWithId {
    private String m_host;
    private int m_port;
    private String m_principal;
    private String m_secret;
    private CronSchedule m_schedule = new CronSchedule();

    public String getHost() {
        return m_host;
    }

    public void setHost(String host) {
        m_host = host;
    }

    public int getPort() {
        return m_port;
    }

    public void setPort(int port) {
        m_port = port;
    }

    public String getPrincipal() {
        return m_principal;
    }

    public void setPrincipal(String principal) {
        m_principal = principal;
    }

    public String getSecret() {
        return m_secret;
    }

    public void setSecret(String secret) {
        m_secret = secret;
    }

    private String getUrl() {
        return String.format("ldap://%s:%d", m_host, m_port);
    }
    
    public void setSchedule(CronSchedule schedule) {
        m_schedule = schedule;
    }
    
    public CronSchedule getSchedule() {
        return m_schedule;
    }

    public void applyToTemplate(JndiLdapTemplate template) {
        template.setProviderUrl(getUrl());
        template.setSecurityPrincipal(m_principal);
        template.setSecurityCredentials(m_secret);
        String authentication = StringUtils.isEmpty(m_secret) ? "none" : "basic";
        template.setSecurityAuthentication(authentication);
    }
}
