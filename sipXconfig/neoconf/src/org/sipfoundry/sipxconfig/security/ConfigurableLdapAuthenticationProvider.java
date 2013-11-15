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
package org.sipfoundry.sipxconfig.security;

import static org.sipfoundry.commons.security.Util.retrieveDomain;
import static org.sipfoundry.commons.security.Util.retrieveUsername;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.bulk.ldap.AttrMap;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapConnectionParams;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapSystemSettings;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.springframework.ldap.core.ContextSource;
import org.springframework.ldap.core.support.BaseLdapPathContextSource;
import org.springframework.ldap.core.support.LdapContextSource;
import org.springframework.security.authentication.AuthenticationServiceException;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.authentication.dao.AbstractUserDetailsAuthenticationProvider;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.AuthenticationException;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.ldap.authentication.BindAuthenticator;
import org.springframework.security.ldap.authentication.LdapAuthenticationProvider;
import org.springframework.security.ldap.authentication.LdapAuthenticator;
import org.springframework.security.ldap.search.FilterBasedLdapUserSearch;
import org.springframework.security.ldap.search.LdapUserSearch;

/**
 * Creates a rebuildable reference to Spring Security real LdapAuthenticationProvider as settings
 * change. We cannot use LdapAuthenticationProvider directly because ldap settings are immutable
 * and Spring keeps a more permanent reference list of auth providers.
 */
public class ConfigurableLdapAuthenticationProvider extends AbstractUserDetailsAuthenticationProvider implements
    DaoEventListener {

    private static final Log LOG = LogFactory.getLog(ConfigurableLdapAuthenticationProvider.class);
    private LdapManager m_ldapManager;
    private List<SipxLdapAuthenticationProvider> m_providers = new ArrayList<SipxLdapAuthenticationProvider>();
    private UserDetailsService m_userDetailsService;
    private boolean m_initialized;

    public UserDetailsService getUserDetailsService() {
        return m_userDetailsService;
    }

    public void setUserDetailsService(UserDetailsService userDetailsService) {
        m_userDetailsService = userDetailsService;
    }

    public LdapManager getLdapManager() {
        return m_ldapManager;
    }

    public void setLdapManager(LdapManager ldapManager) {
        m_ldapManager = ldapManager;
    }

    public void addProvider(SipxLdapAuthenticationProvider provider) {
        m_providers.add(provider);
    }

    @Override
    protected void additionalAuthenticationChecks(UserDetails userDetails,
            UsernamePasswordAuthenticationToken authentication) throws AuthenticationException {
        // do nothing
    }

    @Override
    protected UserDetails retrieveUser(String userLoginName, UsernamePasswordAuthenticationToken authentication)
        throws AuthenticationException {
        if (!m_ldapManager.getSystemSettings().isConfigured()) {
            String message = "LDAP Authentication not configured";
            if (LOG.isDebugEnabled()) {
                LOG.debug(message);
            }
            throw new AuthenticationServiceException(message);
        }
        initialize();
        if (!isEnabled()) {
            String message = "LDAP Authentication not enabled";
            if (LOG.isDebugEnabled()) {
                LOG.debug(message);
            }
            throw new AuthenticationServiceException(message);
        } else {
            Authentication result = null;
            String username = retrieveUsername(userLoginName);
            String userDomain = retrieveDomain(userLoginName);
            UserDetailsImpl loaddedUser = (UserDetailsImpl) m_userDetailsService.loadUserByUsername(username);
            if (loaddedUser == null) {
                if (LOG.isDebugEnabled()) {
                    LOG.debug("userDetailsService didnt find any user " + username);
                }
                throw new AuthenticationServiceException("UserDetailsService returned null, which "
                        + "is an interface contract violation");
            }

            for (SipxLdapAuthenticationProvider provider : m_providers) {
                String providerDomain = provider.getDomain();
                if (LOG.isDebugEnabled()) {
                    LOG.debug("current provider domain is " + providerDomain + " user input domain is " + userDomain);
                }
                // verify if user input domain can be handled by this provider, continue with next
                // provider
                if (userDomain != null && !userDomain.equalsIgnoreCase(providerDomain)) {
                    if (LOG.isDebugEnabled()) {
                        LOG.debug(
                            "User domain which is: " + userDomain + " does not match provider domain which is: "
                            + providerDomain + " continue with the next provider");
                    }
                    continue;
                }

                // if no domain specified by user, compare the one from database with the provider
                // domain
                if (userDomain == null) {
                    String savedDomain = loaddedUser.getUserDomain();
                    if (!StringUtils.equalsIgnoreCase(StringUtils.defaultString(savedDomain, StringUtils.EMPTY),
                            StringUtils.defaultString(providerDomain, StringUtils.EMPTY))) {
                        if (LOG.isDebugEnabled()) {
                            LOG.debug("provider domain which is: "
                                + StringUtils.defaultString(providerDomain, "Null")
                                + " is different than user domain which is: "
                                + StringUtils.defaultString(savedDomain, "NULL") + " continue with next provider");
                        }
                        continue;
                    }
                }

                try {
                    UsernamePasswordAuthenticationToken myAuthentication = new UsernamePasswordAuthenticationToken(
                            loaddedUser.getCanonicalUserName(), authentication.getCredentials());
                    result = provider.authenticate(myAuthentication);
                } catch (AuthenticationException ex) {
                    result = null;
                }
                if (result == null) {
                    continue;
                } else {
                    return loaddedUser;
                }
            }
        }
        throw new AuthenticationServiceException("not able to authenticate user against LDAP providers chain");
    }

    @Override
    public boolean supports(Class authentication) {
        if (!m_ldapManager.getSystemSettings().isConfigured()) {
            return false;
        }
        initialize();
        if (!isEnabled()) {
            return false;
        } else {
            boolean result = false;
            for (LdapAuthenticationProvider provider : m_providers) {
                result = provider.supports(authentication);
                if (!result) {
                    continue;
                } else {
                    return result;
                }
            }
        }
        return false;

    }

    private boolean isEnabled() {
        return !m_providers.isEmpty() && m_ldapManager.getSystemSettings().isLdapEnabled();
    }

    private void initialize() {
        if (!m_initialized) {
            synchronized (this) {
                m_providers.clear();
                List<LdapConnectionParams> params = m_ldapManager.getAllConnectionParams();
                for (LdapConnectionParams param : params) {
                    addProvider(createProvider(param.getId()));
                }
                m_initialized = true;
            }
        }
    }

    SipxLdapAuthenticationProvider createProvider(int connectionId) {
        LdapConnectionParams params = m_ldapManager.getConnectionParams(connectionId);
        if (params == null) {
            return null;
        }
        ContextSource dirFactory = getDirFactory(params);
        BindAuthenticator authenticator = new BindAuthenticator((BaseLdapPathContextSource) dirFactory);
        authenticator.setUserSearch(getSearch((BaseLdapPathContextSource) dirFactory, connectionId)); // used
                                                                                                      // for
                                                                                                      // user
                                                                                                      // login
        SipxLdapAuthenticationProvider provider = new SipxLdapAuthenticationProvider(authenticator);
        provider.setDomain(params.getDomain());
        return provider;
    }

    ContextSource getDirFactory(LdapConnectionParams params) {
        String bindUrl = params.getUrl();
        LdapContextSource dirContextFactory = new LdapContextSource();
        dirContextFactory.setUrl(bindUrl);
        // allow anonymous access if so configured in LDAP server configuration page
        if (!StringUtils.isEmpty(params.getPrincipal())) {
            dirContextFactory.setUserDn(params.getPrincipal());
            dirContextFactory.setPassword(params.getSecret());
        } else {
            dirContextFactory.setAnonymousReadOnly(true);
        }
        Map<String, String> envProps = new HashMap<String, String>();
        envProps.put(LdapConnectionParams.LDAP_TIMEOUT, String.valueOf(params.getTimeout()));
        dirContextFactory.setBaseEnvironmentProperties(envProps);
        try {
            dirContextFactory.afterPropertiesSet();
        } catch (Exception ex) {
            LOG.error("LdapContextSource not corectly initialized");
        }
        return dirContextFactory;
    }

    LdapUserSearch getSearch(BaseLdapPathContextSource dirFactory, int connectionId) {
        AttrMap attrMap = m_ldapManager.getAttrMap(connectionId);

        String sbase = StringUtils.defaultString(attrMap.getSearchBase());
        // Any additional LDAP filters (RFC 2254) are removed from authentication search because
        // here the filter is used to specify what LDAP attribute represents the username and it
        // does not respect RFC2254 guidelines
        String filter = String.format("(%s={0})", attrMap.getIdentityAttributeName());

        FilterBasedLdapUserSearch search = new FilterBasedLdapUserSearch(sbase, filter, dirFactory);
        search.setSearchSubtree(true);
        return search;
    }

    class SipxLdapAuthenticationProvider extends LdapAuthenticationProvider {
        private String m_domain;

        public SipxLdapAuthenticationProvider(LdapAuthenticator authenticator) {
            // 2nd, arg - no authority provider nec, the userdetailservice adds
            // those
            super(authenticator);
        }

        public String getDomain() {
            return m_domain;
        }

        public void setDomain(String domain) {
            m_domain = domain;
        }

    }

    @Override
    public void onDelete(Object entity) {
        onChange(entity);
    }

    @Override
    public void onSave(Object entity) {
        onChange(entity);
    }

    void onChange(Object entity) {
        // any change to any ldap object, trigger lazy rebuild of authentication
        // objects
        if (entity instanceof LdapConnectionParams || entity instanceof AttrMap
                || entity instanceof LdapSystemSettings) {
            m_initialized = false;
        }
    }
}
