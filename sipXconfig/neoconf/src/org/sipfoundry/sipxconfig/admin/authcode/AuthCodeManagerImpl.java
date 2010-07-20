/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxconfig.admin.authcode;

import java.util.ArrayList;
import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import static java.util.Collections.singletonList;

import org.apache.commons.lang.RandomStringUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventPublisher;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.Permission.Type;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxAccCodeService;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.annotation.Required;

import static org.sipfoundry.sipxconfig.common.DaoUtils.requireOneOrZero;

public class AuthCodeManagerImpl extends SipxHibernateDaoSupport implements
        AuthCodeManager {

    private static final Log LOG = LogFactory.getLog(AuthCodeManagerImpl.class);
    private static final String AUTH_CODE_CODE = "code";
    private static final String INTERNAL_NAME = "~~ac~%s";
    private static final String ACC_CONTACT_NAME = "ACC";

    private CoreContext m_coreContext;
    private DaoEventPublisher m_daoEventPublisher;
    private SipxServiceManager m_sipxServiceManager;
    private ServiceConfigurator m_serviceConfigurator;

    @Override
    public void deleteAuthCode(AuthCode authCode) {
        deleteAuthCodes(singletonList(authCode.getId()));
    }

    @Override
    public void deleteAuthCodes(Collection<Integer> allSelected) {
        LOG.info("ENTERED ::deleteAuthCodes");
        List<AuthCode> codes = getAuthCodes(allSelected);
        for (AuthCode code : codes) {
            LOG.info("::deleteAuthCodes() code: " + code);
            m_daoEventPublisher.publishDelete(code);
        }
        getHibernateTemplate().deleteAll(codes);
        LOG.info("::deleteAuthCodes invoking replicateServiceConfig()");
        replicateServicesConfig();
    }

    @Override
    public AuthCode getAuthCode(Integer authCodeId) {
        return (AuthCode) getHibernateTemplate().load(AuthCode.class,
                authCodeId);
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

        String userName = StringUtils.deleteWhitespace(String.format(
                INTERNAL_NAME, authCode.getId()));

        InternalUser internalUser = authCode.getInternalUser();
        Collection<String>permissionNames = internalUser.getUserPermissionNames();
        LOG.info("AuthCodeManagerImpl::saveAuthCode() got permissionNames:" + permissionNames);

        if (!gotCallPermission(permissionNames)) {
            throw new UserException("&blank.permission.error");
        }
        authCode.getInternalUser().setUserName(userName);
        if (!authCode.isNew()) {
            getHibernateTemplate().merge(authCode);
        } else {
            getHibernateTemplate().save(authCode);
            //Need to update authname since we should have a real authcode id now
            userName = StringUtils.deleteWhitespace(String.format(
                    INTERNAL_NAME, authCode.getId()));
            LOG.info("::authcode interanl user name after save: " + authCode.getInternalUser().getUserName());
            authCode.getInternalUser().setUserName(userName);
        }

        replicateServicesConfig();
    }

    @Override
    public AuthCode getAuthCodeByCode(String code) {
        String query = "authCodeByCode";
        Collection<AuthCode> codes = getHibernateTemplate()
                .findByNamedQueryAndNamedParam(query, AUTH_CODE_CODE, code);
        return requireOneOrZero(codes, query);
    }

    public AuthCode newAuthCode() {
        AuthCode code = new AuthCode();
        InternalUser internaluser = m_coreContext.newInternalUser();
        internaluser.setSipPassword(RandomStringUtils.randomAlphanumeric(10));
        internaluser.setSettingTypedValue(PermissionName.VOICEMAIL.getPath(),
                false);
        internaluser.setSettingTypedValue(PermissionName.FREESWITH_VOICEMAIL
                .getPath(), false);
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

    private void replicateServicesConfig() {
        replicateService(SipxProxyService.BEAN_ID);
        replicateService(SipxAccCodeService.BEAN_ID);
    }

    private void replicateService(String id) {
        SipxService sipxService = m_sipxServiceManager.getServiceByBeanId(id);
        LOG.info("::replicateService with id: " + sipxService);
        m_serviceConfigurator.replicateServiceConfig(sipxService);
    }

    private boolean isCodeChanged(AuthCode code) {
        // lookup via id what the old code is and compare with the input
        // parameter
        return !getAuthCode(code.getId()).getCode().equals(code.getCode());
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Required
    public void setServiceConfigurator(ServiceConfigurator serviceConfigurator) {
        m_serviceConfigurator = serviceConfigurator;
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setDaoEventPublisher(DaoEventPublisher daoEventPublisher) {
        m_daoEventPublisher = daoEventPublisher;
    }

    public boolean gotCallPermission(Collection<String> permissionNames) {
        for (String name : permissionNames) {
            PermissionName pm = PermissionName.findByName(name);
            if (pm != null) {
                LOG.info("AuthCodeManagerImpl::saveAuthCode() got a perm:" + pm);
                Type type = pm.getType();
                //search for built-in permissions of "CALL" permission type
                if (type.equals(Type.CALL)) {
                    LOG.info("AuthCodeManagerImpl::saveAuthCode() got a call perm:" + name);
                    return true;
                }
            } else if (name.startsWith(Permission.NAME_PREFIX)) {
                //all customer permissions are "CALL" permission
                //and custom permissions names starts with Permission.NAME_PREFIX
                LOG.info("AuthCodeManagerImpl::saveAuthCode() got a custom perm:" + name);
                return true;
            }
        }
        return false;
    }


    /**
     * Format the identity string for the alias.xml
     */
    public String getIdentityUri(String alias) {
        String identity =  SipUri.format(alias, getSipxFreeswitchService().getDomainName(), false);
        LOG.info("AuthCodeManagerImpl::getIdentityUri() for alias:" + alias + " is identity:" + identity);
        return identity;
    }

    @Override
    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        SipxAccCodeService service =
            (SipxAccCodeService) m_sipxServiceManager.getServiceByBeanId(SipxAccCodeService.BEAN_ID);

        Set<String> set = service.getAliasesAsSet();
        if (!set.contains(alias)) {
            Collection ids = new ArrayList(0);
            return ids;
        } else {
            Collection ids = new ArrayList(1);
            ids.add(service.getId());
            Collection bids = BeanId.createBeanIdCollection(ids, SipxAccCodeService.class);
            return bids;
        }
    }

    @Override
    public boolean isAliasInUse(String alias) {
        SipxAccCodeService service =
            (SipxAccCodeService) m_sipxServiceManager.getServiceByBeanId(SipxAccCodeService.BEAN_ID);

        Set<String> set = service.getAliasesAsSet();
        return set.contains(alias);
    }

    private SipxFreeswitchService getSipxFreeswitchService() {
        return (SipxFreeswitchService) m_sipxServiceManager.getServiceByBeanId(SipxFreeswitchService.BEAN_ID);
    }

    // Build an alias which indirectly maps to the acc service via
    // the acc extension prefix
    // (something like sip:*81@bcmsl2077.ca.nortel.com)
    // which then gets mapped in mappingrule.xml to the acc service
    //( something like sip:AUTH@47.135.162.72:15060;command=auth;)
    private String getContactUri() {
        SipxAccCodeService service =
            (SipxAccCodeService) m_sipxServiceManager.getServiceByBeanId(SipxAccCodeService.BEAN_ID);
        Map<String, String> params = new LinkedHashMap<String, String>();
        String prefix = service.getAuthCodePrefix();
        return SipUri.format(prefix, getSipxFreeswitchService().getDomainName(), false);
    }

    // Build an alias which maps directly to the acc service
    // (something like sip:AUTH@47.135.162.72:15060;command=auth;)
    // direct mapping is for testing only
    private String getDirectContactUri() {
        Map<String, String> params = new LinkedHashMap<String, String>();
        params.put("command", "auth");
        return SipUri.format(ACC_CONTACT_NAME, getSipxFreeswitchAddressAndPort(), params);
    }

    public Collection getAliasMappings() {
        //first figure out how many aliases
        SipxAccCodeService service = (SipxAccCodeService)
            m_sipxServiceManager.getServiceByBeanId(SipxAccCodeService.BEAN_ID);
        Set<String> aliasesSet = service.getAliasesAsSet();

        // Add alias entry for each extension alias
        // all entries points to the same auth code url
        // sip:AUTH@47.135.162.72:15060;command=auth;
        // see mappingrules.xml
        List<AliasMapping> aliasMappings = new ArrayList<AliasMapping>(aliasesSet.size());
        if (aliasesSet.size() == 0) {
            return aliasMappings;
        }

        String contact = null;
        String identity = null;
        for (String alias : aliasesSet) {
            identity = getIdentityUri(alias);
            contact = getContactUri();
            // direct mapping is for testing only
            // contact = getDirectContactUri();
            aliasMappings.add(new AliasMapping(identity, contact, "alias"));
        }

        LOG.info("RETURNING AuthCodeManagerImpl::getAliasMapping: " + aliasMappings);
        return aliasMappings;
    }
    private String getSipxFreeswitchAddressAndPort() {
        SipxFreeswitchService service = getSipxFreeswitchService();
        String host;
        if (service.getAddresses().size() > 1) {
            // HACK: this assumes that one of the freeswitch instances runs on a primary location
            // (but that neeeds to be true in order for MOH to work anyway)
            host = service.getLocationsManager().getPrimaryLocation().getAddress();
        } else {
            host = service.getAddress();
        }

        return host + ":" + service.getFreeswitchSipPort();
    }
}
