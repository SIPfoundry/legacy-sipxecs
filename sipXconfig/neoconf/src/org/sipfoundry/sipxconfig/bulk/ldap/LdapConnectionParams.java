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


import static org.apache.commons.lang.StringUtils.EMPTY;
import static org.apache.commons.lang.StringUtils.defaultString;
import static org.apache.commons.lang.StringUtils.isBlank;
import static org.apache.commons.lang.StringUtils.join;
import static org.apache.commons.lang.StringUtils.split;

import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import javax.naming.Context;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.CronSchedule;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.springframework.ldap.core.support.LdapContextSource;

/**
 * Used to store LDAP connections in the DB LdapConnectionParams
 */
public class LdapConnectionParams extends BeanWithId implements DeployConfigOnEdit {
    private static final int DEFAULT_PORT = 389;
    private static final int DEFAULT_SSL_PORT = 636;

    private String m_host;
    private String m_fullHost;
    private String m_domain;
    private Integer m_port;
    private String m_principal;
    private String m_secret;
    private boolean m_useTls;

    /**
     * Used set Context.REFERRAL property. Needs to be 'follow' for ActiveDirecory.
     */
    private String m_referral;

    private CronSchedule m_schedule = new CronSchedule();

    public String getHost() {
        return m_host;
    }

    public void setHost(String host) {
        m_host = host;
        m_fullHost = getFullHostValue();
    }

    private String getFullHostValue() {
        if (!isBlank(m_domain) && !isBlank(m_host)) {
            return join(new String [] {m_host, " ", m_domain});
        } else {
            return m_host;
        }
    }

    public Integer getPort() {
        return m_port;
    }

    public void setPort(Integer port) {
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

    public boolean getUseTls() {
        return m_useTls;
    }

    public void setUseTls(boolean useTls) {
        m_useTls = useTls;
    }

    public Integer getPortToUse() {
        Integer portToUse = m_port;
        if (portToUse == null) {
            portToUse = m_useTls ? DEFAULT_SSL_PORT : DEFAULT_PORT;
        }
        return portToUse;
    }

    public String getUrl() {
        Integer portToUse = getPortToUse();
        if (m_useTls) {
            return String.format("ldaps://%s:%d", m_host, portToUse);
        }
        return String.format("ldap://%s:%d", m_host, portToUse);
    }

    public void setSchedule(CronSchedule schedule) {
        m_schedule = schedule;
    }

    public CronSchedule getSchedule() {
        return m_schedule;
    }

    public void setReferral(String referral) {
        m_referral = referral;
    }

    public void applyToContext(LdapContextSource config) {
        config.setUserDn(defaultString(m_principal, EMPTY));
        config.setPassword(defaultString(m_secret, EMPTY));
        config.setUrl(getUrl());
        Map<String, String> otherParams = new HashMap<String, String>();
        otherParams.put(Context.REFERRAL, m_referral);
        config.setBaseEnvironmentProperties(otherParams);
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) LdapManager.FEATURE);
    }

    public String getDomain() {
        return m_domain;
    }

    public void setDomain(String domain) {
        m_domain = domain;
        m_fullHost = getFullHostValue();
    }

    public String getFullHost() {
        return m_fullHost;
    }

    public void setFullHost(String fullHost) {
        m_fullHost = fullHost;
        String[] host = split(m_fullHost);
        if (host.length == 2) {
            m_host = host[0];
            m_domain = host[1];
        } else {
            m_host = fullHost;
        }
    }
}
