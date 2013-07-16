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
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.bulk.ldap.AttrMap;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapConnectionParams;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapSystemSettings;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.springframework.ldap.core.DirContextOperations;
import org.springframework.ldap.core.support.BaseLdapPathContextSource;
import org.springframework.ldap.core.support.DirContextSource;
import org.springframework.security.authentication.AuthenticationProvider;
import org.springframework.security.authentication.AuthenticationServiceException;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.AuthenticationException;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.ldap.authentication.BindAuthenticator;
import org.springframework.security.ldap.authentication.LdapAuthenticationProvider;
import org.springframework.security.ldap.authentication.LdapAuthenticator;
import org.springframework.security.ldap.search.FilterBasedLdapUserSearch;
import org.springframework.security.ldap.search.LdapUserSearch;
import org.springframework.security.ldap.userdetails.LdapAuthoritiesPopulator;

/**
 * Creates a rebuildable reference to Acegi's real LdapAuthenticationProvider as settings change.
 * We cannot use LdapAuthenticationProvider directly because ldap settings are immutable and
 * Spring keeps a more permanent reference list of auth providers.
 */
public class ConfigurableLdapAuthenticationProvider implements AuthenticationProvider, DaoEventListener {

    private LdapManager m_ldapManager;
    private List<SipxLdapAuthenticationProvider> m_providers = new ArrayList<SipxLdapAuthenticationProvider>();
    private LdapAuthoritiesPopulator m_authoritiesPopulator;
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

    public LdapAuthoritiesPopulator getAuthoritiesPopulator() {
        return m_authoritiesPopulator;
    }

    public void setAuthoritiesPopulator(LdapAuthoritiesPopulator authoritiesPopulator) {
        m_authoritiesPopulator = authoritiesPopulator;
    }

    public void addProvider(SipxLdapAuthenticationProvider provider) {
        m_providers.add(provider);
    }

    @Override
    public Authentication authenticate(Authentication authentication) {
        if (!m_ldapManager.getSystemSettings().isConfigured()) {
            return null;
        }
        initialize();
        if (!isEnabled()) {
            return null;
        } else {
            Authentication result = null;
            String username = authentication.getName();
            String userDomain = retrieveDomain(username);

            for (SipxLdapAuthenticationProvider provider : m_providers) {
                if (userDomain != null && !userDomain.equals(provider.getDomain())) {
                    continue;
                }
                try {
                    result = provider.authenticate(authentication);
                } catch (AuthenticationException ex) {
                    result = null;
                }
                if (result == null) {
                    continue;
                } else {
                    return result;
                }
            }
        }
        return null;
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
        DirContextSource dirFactory = getDirFactory(params);
        BindAuthenticator authenticator = new BindAuthenticator(dirFactory);
        authenticator.setUserSearch(getSearch(dirFactory, connectionId)); // used for user login
        SipxLdapAuthenticationProvider provider = new SipxLdapAuthenticationProvider(authenticator);
        provider.setDomain(params.getDomain());
        return provider;
    }

    DirContextSource getDirFactory(LdapConnectionParams params) {
        String bindUrl = params.getUrl();
        DirContextSource dirContextFactory = new DirContextSource();
        dirContextFactory.setUrl(bindUrl);
        //allow anonymous access if so configured in LDAP server configuration page
        if (!StringUtils.isEmpty(params.getPrincipal())) {
            dirContextFactory.setUserDn(params.getPrincipal());
            dirContextFactory.setPassword(params.getSecret());
        }
        return dirContextFactory;
    }

    LdapUserSearch getSearch(BaseLdapPathContextSource dirFactory, int connectionId) {
        AttrMap attrMap = m_ldapManager.getAttrMap(connectionId);

        String sbase = StringUtils.defaultString(attrMap.getSearchBase());
        //Any additional LDAP filters (RFC 2254) are removed from authentication search because
        //here the filter is used to specify what LDAP attribute represents the username and it
        //does not respect RFC2254 guidelines
        String filter = String.format("(%s={0})", attrMap.getIdentityAttributeName());

        FilterBasedLdapUserSearch search = new FilterBasedLdapUserSearch(sbase, filter, dirFactory);
        search.setSearchSubtree(true);
        return search;
    }

    /**
     * we need to construct UserDetailsImpl objects because that's what web layer expects,
     * otherwise we wouldn't have have to extend LdapAuthenticationProvider
     */
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

        /**
         * When the user id introduced in the login form is user's alias, we need to tell LDAP the true user ID
         * otherwise cannot get authenticated against LDAP using user alias
         */
        @Override
        protected DirContextOperations doAuthentication(UsernamePasswordAuthenticationToken authentication) {
            UserDetailsImpl user = null;
            String username = (String) authentication.getPrincipal();
            String userLoginName = retrieveUsername(username);
            String domain = retrieveDomain(username);
            try {
                user = (UserDetailsImpl) m_userDetailsService.loadUserByUsername(userLoginName);
                if (user == null) {
                    throw new AuthenticationServiceException("UserDetailsService returned null, which "
                        + "is an interface contract violation");
                }
                if (domain != null && !StringUtils.equals(user.getUserDomain(), domain)) {
                    throw new AuthenticationServiceException(
                            "The following domain does not belong to the actual user: " + domain
                            + " in the system - is an interface contract violation");
                }
                UsernamePasswordAuthenticationToken myAuthentication = new UsernamePasswordAuthenticationToken(
                        user.getCanonicalUserName(), authentication.getCredentials());
                //we call the super method to verifiy true username/password ldap authentication
                //we don't return default LDAP retrieved user, but our sipX user, authenticated against LDAP
                return super.doAuthentication(myAuthentication);
            } catch (AuthenticationException repositoryProblem) {
                throw new AuthenticationServiceException(repositoryProblem.getMessage(), repositoryProblem);
            }
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
