/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxconfig.acccode;

import static java.util.Collections.singletonList;
import static org.sipfoundry.sipxconfig.common.DaoUtils.requireOneOrZero;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.RandomStringUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.Permission.Type;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.springframework.beans.factory.annotation.Required;

public class AuthCodeManagerImpl extends SipxHibernateDaoSupport<AuthCode> implements AuthCodeManager  {
    private static final String AUTH_CODE_CODE = "code";
    private static final String INTERNAL_NAME = "~~ac~%s";
    private static final Log LOG = LogFactory.getLog(AuthCodeManagerImpl.class);

    private CoreContext m_coreContext;
    private AuthCodesImpl m_authCodesImpl;
    private FeatureManager m_featureManager;

    @Override
    public void deleteAuthCode(AuthCode authCode) {
        deleteAuthCodes(singletonList(authCode.getId()));
    }

    @Override
    public void deleteAuthCodes(Collection<Integer> allSelected) {
        List<AuthCode> codes = getAuthCodes(allSelected);
        for (AuthCode code : codes) {
            getDaoEventPublisher().publishDelete(code);
        }
        getHibernateTemplate().deleteAll(codes);
    }

    @Override
    public AuthCode getAuthCode(Integer authCodeId) {
        return (AuthCode) getHibernateTemplate().load(AuthCode.class, authCodeId);
    }

    @Override
    public List<AuthCode> getAuthCodes() {
        return getHibernateTemplate().loadAll(AuthCode.class);
    }

    @Override
    public List<AuthCode> getAuthCodes(Collection<Integer> ids) {
        List<AuthCode> codes = new ArrayList<AuthCode>(ids.size());
        for (Integer id : ids) {
            codes.add(getAuthCode(id));
        }
        return codes;
    }

    public boolean gotCallPermission(Collection<String> permissionNames) {
        for (String name : permissionNames) {
            PermissionName pm = PermissionName.findByName(name);
            if (pm != null) {
                Type type = pm.getType();
                // search for built-in permissions of "CALL" permission type
                if (type.equals(Type.CALL)) {
                    return true;
                }
            } else if (name.startsWith(Permission.NAME_PREFIX)) {
                // all customer permissions are "CALL" permission
                // and custom permissions names starts with Permission.NAME_PREFIX
                return true;
            }
        }
        return false;
    }

    @Override
    public void saveAuthCode(AuthCode authCode) {
        // check if auth code name is empty
        if (StringUtils.isBlank(authCode.getCode())) {
            throw new UserException("&blank.code.error");
        }

        // Check for duplicate codes before saving the Auth Code
        if (authCode.isNew() || (!authCode.isNew() && isCodeChanged(authCode))) {
            checkForDuplicateCode(authCode);
        }

        String userName = StringUtils.deleteWhitespace(String.format(INTERNAL_NAME, authCode.getId()));

        InternalUser internalUser = authCode.getInternalUser();
        Collection<String> permissionNames = internalUser.getUserPermissionNames();
        LOG.info("AuthCodeManagerImpl::saveAuthCode() got permissionNames:" + permissionNames);

        if (!gotCallPermission(permissionNames)) {
            throw new UserException("&blank.permission.error");
        }
        authCode.getInternalUser().setUserName(userName);
        if (!authCode.isNew()) {
            getHibernateTemplate().merge(authCode);
        } else {
            getHibernateTemplate().save(authCode);
            // Need to update authname since we should have a real authcode id now
            userName = StringUtils.deleteWhitespace(String.format(INTERNAL_NAME, authCode.getId()));
            LOG.info("::authcode interanl user name after save: " + authCode.getInternalUser().getUserName());
            authCode.getInternalUser().setUserName(userName);
        }
    }

    @Override
    public AuthCode getAuthCodeByCode(String code) {
        String query = "authCodeByCode";
        Collection<AuthCode> codes = getHibernateTemplate().findByNamedQueryAndNamedParam(query, AUTH_CODE_CODE,
                code);
        return requireOneOrZero(codes, query);
    }

    @Override
    public AuthCode newAuthCode() {
        AuthCode code = new AuthCode();
        InternalUser internaluser = m_coreContext.newInternalUser();
        internaluser.setSipPassword(RandomStringUtils.randomAlphanumeric(10));
        internaluser.setSettingTypedValue(PermissionName.VOICEMAIL.getPath(), false);
        internaluser.setSettingTypedValue(PermissionName.FREESWITH_VOICEMAIL.getPath(), false);
        code.setInternalUser(internaluser);

        return code;
    }

    private void checkForDuplicateCode(AuthCode authCode) {
        String code = authCode.getCode();
        AuthCode existingCode = getAuthCodeByCode(code);
        if (existingCode != null) {
            throw new UserException("&duplicate.code.error", code);
        }
    }

    private boolean isCodeChanged(AuthCode code) {
        // lookup via id what the old code is and compare with the input
        // parameter
        return !getAuthCode(code.getId()).getCode().equals(code.getCode());
    }

    @Override
    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        Collection ids = Collections.emptyList();
        if (!m_authCodesImpl.isEnabled()) {
            return ids;
        }

        AuthCodeSettings settings = m_authCodesImpl.getSettings();
        if (settings != null) {
            Set<String> aliases = settings.getAliasesAsSet();
            aliases.add(settings.getAuthCodePrefix());
            for (String serviceAlias : aliases) {
                if (serviceAlias.equals(alias)) {
                    ids = BeanId.createBeanIdCollection(Collections.singletonList(settings.getId()),
                            AuthCodeSettings.class);
                    break;
                }
            }
        }
        getHibernateTemplate().evict(settings);
        return ids;
    }

    @Override
    public boolean isAliasInUse(String alias) {
        List<Location> locations = m_featureManager.getLocationsForEnabledFeature(AuthCodes.FEATURE);
        if (!locations.isEmpty()) {
            AuthCodeSettings settings = m_authCodesImpl.getSettings();
            //when enabling auth code feature, default settings bean has id -1
            //thus the test is always true (see AliasManagerImpl.canObjectUseAlias
            //we need to disable the test for this situation
            if (settings.isNew()) {
                return false;
            }
            Set<String> aliases = settings.getAliasesAsSet();
            aliases.add(settings.getAuthCodePrefix());
            for (String serviceAlias : aliases) {
                if (serviceAlias.equals(alias)) {
                    return true;
                }
            }
            getHibernateTemplate().evict(settings);
        }

        return false;
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setAuthCodesImpl(AuthCodesImpl authCodes) {
        m_authCodesImpl = authCodes;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }
}
