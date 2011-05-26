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
package org.sipfoundry.sipxconfig.security;

import org.acegisecurity.Authentication;
import org.acegisecurity.AuthenticationServiceException;
import org.acegisecurity.BadCredentialsException;
import org.acegisecurity.ldap.DefaultInitialDirContextFactory;
import org.acegisecurity.ldap.InitialDirContextFactory;
import org.acegisecurity.ldap.LdapUserSearch;
import org.acegisecurity.ldap.search.FilterBasedLdapUserSearch;
import org.acegisecurity.providers.AuthenticationProvider;
import org.acegisecurity.providers.UsernamePasswordAuthenticationToken;
import org.acegisecurity.providers.ldap.LdapAuthenticationProvider;
import org.acegisecurity.providers.ldap.LdapAuthenticator;
import org.acegisecurity.providers.ldap.LdapAuthoritiesPopulator;
import org.acegisecurity.providers.ldap.authenticator.BindAuthenticator;
import org.acegisecurity.userdetails.UserDetails;
import org.acegisecurity.userdetails.UserDetailsService;
import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.bulk.ldap.AttrMap;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapConnectionParams;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapSystemSettings;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.springframework.dao.DataAccessException;

/**
 * Creates a rebuildable reference to Acegi's real LdapAuthenticationProvider as settings change.
 * We cannot use LdapAuthenticationProvider directly because ldap settings are immutable and
 * Spring keeps a more permanent reference list of auth providers.
 */
public class ConfigurableLdapAuthenticationProvider implements AuthenticationProvider, DaoEventListener {

    private LdapManager m_ldapManager;
    private LdapSystemSettings m_settings;
    private LdapAuthenticationProvider m_provider;
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

    public void setProvider(LdapAuthenticationProvider provider) {
        m_provider = provider;
    }

    @Override
    public Authentication authenticate(Authentication authentication) {
        initialize();
        return (isEnabled() ? m_provider.authenticate(authentication) : null);
    }

    @Override
    public boolean supports(Class authentication) {
        if (!m_ldapManager.verifyLdapConnection()) {
            return false;
        }
        initialize();
        return (isEnabled() ? m_provider.supports(authentication) : false);
    }

    private boolean isEnabled() {
        return m_provider != null && m_settings.isLdapEnabled() && m_settings.isConfigured();
    }

    private void initialize() {
        if (!m_initialized) {
            synchronized (this) {
                m_provider = createProvider();
                m_settings = m_ldapManager.getSystemSettings();
                m_initialized = true;
            }
        }
    }

    LdapAuthenticationProvider createProvider() {
        LdapConnectionParams params = m_ldapManager.getConnectionParams();
        if (params == null) {
            return null;
        }
        InitialDirContextFactory dirFactory = getDirFactory(params);
        BindAuthenticator authenticator = new BindAuthenticator(dirFactory);
        authenticator.setUserSearch(getSearch(dirFactory)); // used for user login
        if (!StringUtils.isEmpty(params.getPrincipal())) {
            authenticator.setUserDnPatterns(new String[] {
                params.getPrincipal()
            // used for binding
            });
        }
        LdapAuthenticationProvider provider = new SipxLdapAuthenticationProvider(authenticator);

        return provider;
    }

    InitialDirContextFactory getDirFactory(LdapConnectionParams params) {
        String bindUrl = params.getUrl();
        DefaultInitialDirContextFactory dirContextFactory = new DefaultInitialDirContextFactory(bindUrl);
        //allow anonymous access if so configured in LDAP server configuration page
        if (!StringUtils.isEmpty(params.getPrincipal())) {
            dirContextFactory.setManagerDn(params.getPrincipal());
            dirContextFactory.setManagerPassword(params.getSecret());
        }
        return dirContextFactory;
    }

    LdapUserSearch getSearch(InitialDirContextFactory dirFactory) {
        AttrMap attrMap = m_ldapManager.getAttrMap();

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

        public SipxLdapAuthenticationProvider(LdapAuthenticator authenticator) {
            // 2nd, arg - no authority provider nec, the userdetailservice adds
            // those
            super(authenticator);
        }

        /**
         * When the user id introduced in the login form is user's alias, we need to tell LDAP the true user ID
         * otherwise cannot get authenticated against LDAP using user alias
         */
        @Override
        protected UserDetails retrieveUser(String username, UsernamePasswordAuthenticationToken authentication) {
            UserDetailsImpl user = null;
            try {
                user = (UserDetailsImpl) m_userDetailsService.loadUserByUsername(username);
                if (user == null) {
                    throw new AuthenticationServiceException("UserDetailsService returned null, which "
                        + "is an interface contract violation");
                }
                UsernamePasswordAuthenticationToken myAuthentication = new UsernamePasswordAuthenticationToken(
                        user.getCanonicalUserName(), authentication.getCredentials());
                //we call the super method to verifiy true username/password ldap authentication
                //we don't return default LDAP retrieved user, but our sipX user, authenticated against LDAP
                super.retrieveUser(user.getCanonicalUserName(), myAuthentication);
            } catch (DataAccessException repositoryProblem) {
                throw new AuthenticationServiceException(repositoryProblem.getMessage(), repositoryProblem);
            }
            return user;
        }

        @Override
        protected void additionalAuthenticationChecks(UserDetails userDetails,
                UsernamePasswordAuthenticationToken authentication) {
            // passwords are checked in ldap layer
            // make sure that LDAP bind password is rejected
            LdapConnectionParams params = m_ldapManager.getConnectionParams();
            if (ObjectUtils.equals(authentication.getCredentials(), params.getSecret())) {
                throw new BadCredentialsException(messages.getMessage(
                        "AbstractUserDetailsAuthenticationProvider.badCredentials", "Bad credentials"),
                        userDetails.getUsername());
            }
            return;
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
