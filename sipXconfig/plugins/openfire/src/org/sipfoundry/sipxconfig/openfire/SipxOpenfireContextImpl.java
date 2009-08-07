/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.openfire;

import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;

import static org.apache.commons.lang.StringUtils.EMPTY;
import static org.apache.commons.lang.StringUtils.isEmpty;
import static org.apache.commons.lang.StringUtils.isNotEmpty;

public class SipxOpenfireContextImpl implements SipxOpenfireContext {
    private static final Log LOG = LogFactory.getLog(SipxOpenfireContextImpl.class);

    private SipxServiceManager m_sipxServiceManager;
    private MailboxManager m_mailboxManager;
    private ApiProvider<OpenfireApi> m_openfireApiProvider;

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Required
    public void setOpenfireApiProvider(ApiProvider<OpenfireApi> openfireApiProvider) {
        m_openfireApiProvider = openfireApiProvider;
    }

    @Required
    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }

    public boolean saveOpenfireAccount(String previousUserName, String userName, String password,
            String sipUserName, String sipPassword, String email) {
        try {
            OpenfireApi api = getOpenfireApi();
            if (isNotEmpty(previousUserName) && userExists(previousUserName)) {
                api.destroyUserAccount(previousUserName);
            }
            if (isEmpty(userName)) {
                LOG.warn("Openfire user name is empty - cannot create");
                return false;
            }
            Map<String, String> retval = api.createUserAccount(userName, userName, EMPTY, email);
            if (!isSuccess(retval)) {
                return false;
            }
            String domainName = getSipxOpenfireService().getDomainName();
            retval = api.setSipId(userName, sipUserName + "@" + domainName);
            if (!isSuccess(retval)) {
                return false;
            }
            retval = api.setSipPassword(userName, sipPassword);
            return isSuccess(retval);
        } catch (XmlRpcRemoteException ex) {
            LOG.warn("Openfire error on saving user", ex);
        }
        return false;
    }

    private boolean userExists(String userName) {
        Map<String, String> retval = getOpenfireApi().userExists(userName);
        return retval.get("account-exists").equals("true");
    }

    public boolean deleteOpenfireAccount(String userName) {
        if (isEmpty(userName)) {
            LOG.warn("Openfire user is empty - cannot delete");
            return false;
        }
        try {
            if (userExists(userName)) {
                Map<String, String> retval = getOpenfireApi().destroyUserAccount(userName);
                return isSuccess(retval);
            }
        } catch (XmlRpcRemoteException ex) {
            LOG.warn("Openfire error on deleting user", ex);
        }
        return false;
    }

    private OpenfireApi getOpenfireApi() {
        return m_openfireApiProvider.getApi(getOpenfireServerUrl());
    }

    public String getOpenfireServerUrl() {
        SipxOpenfireService service = getSipxOpenfireService();
        if (service == null) {
            throw new UserException("&message.noOpenfireService");
        }
        return String.format("http://%s:%d/plugins/sipx-openfire/user", service.getServerAddress(), service
                .getPort());
    }

    private boolean isSuccess(Map<String, String> retval) {
        return retval.get("status-code").equals("ok");
    }

    private SipxOpenfireService getSipxOpenfireService() {
        return (SipxOpenfireService) m_sipxServiceManager.getServiceByBeanId(SipxOpenfireService.BEAN_ID);
    }
}
