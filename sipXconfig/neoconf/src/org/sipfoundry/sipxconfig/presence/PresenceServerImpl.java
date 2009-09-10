/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.presence;

import java.util.Hashtable;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcProxyFactoryBean;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;

/**
 * Interaction with PresenceServer
 */
public class PresenceServerImpl implements PresenceServer {
    public static final String OBJECT_CLASS_KEY = "object-class";
    private static final Log LOG = LogFactory.getLog(PresenceServerImpl.class);
    private CoreContext m_coreContext;
    private boolean m_enabled;

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public void signIn(User user, AcdServer acdServer) {
        signInAction(SignIn.SIGN_IN, user, acdServer);
    }

    public void signOut(User user, AcdServer acdServer) {
        signInAction(SignIn.SIGN_OUT, user, acdServer);
    }

    public PresenceStatus getStatus(User user, AcdServer acdServer) {
        PresenceStatus status = PresenceStatus.NOT_AVAILABLE;
        if (m_enabled) {
            try {
                Hashtable response = signInAction(SignIn.STATUS, user, acdServer);
                String statusId = (String) response.get(SignIn.RESULT_TEXT);
                status = PresenceStatus.resolve(statusId);
            } catch (Exception e) {
                LOG.error(e);
            }
        }
        return status;
    }


    private Hashtable signInAction(String action, User user, AcdServer acdServer) {
        String presenceServiceUri = acdServer.getPresenceServiceUri();

        if (!m_enabled) {
            return null;
        }

        if (presenceServiceUri == null) {
            LOG.warn("No ACD presence server available.");
            return null;
        }
        XmlRpcProxyFactoryBean factory = new XmlRpcProxyFactoryBean();
        factory.setServiceInterface(SignIn.class);
        factory.setServiceUrl(presenceServiceUri);
        factory.afterPropertiesSet();
        SignIn api = (SignIn) factory.getObject();
        return userAction(api, action, user);
    }

    Hashtable userAction(SignIn api, String actionId, User user) {
        try {
            Hashtable action = new Hashtable();
            String domainName = m_coreContext.getDomainName();
            action.put(OBJECT_CLASS_KEY, SignIn.OBJECT_CLASS_ID);
            action.put(actionId, user.getUri(domainName));
            Hashtable response = api.action(action);
            checkErrorCode(response);
            return response;
        } catch (XmlRpcRemoteException e) {
            LOG.error("XML/RPC related problems in presence service.", e);
            throw new SignInException(e.getCause().getMessage());
        }
    }

    static void checkErrorCode(Hashtable response) {
        Integer resultCode = (Integer) response.get(SignIn.RESULT_CODE);
        if (!SignIn.SUCCESS.equals(resultCode)) {
            String rawMessage = (String) response.get(SignIn.RESULT_TEXT);
            String message = StringUtils.defaultString(rawMessage, "Error calling remote api");
            throw new SignInException(message);
        }
    }

    static class SignInException extends UserException {
        SignInException(String msg) {
            super(msg);
        }
    }

    /**
     * Raw API from presence server. Not very useful outside this context
     */
    public static interface SignIn {
        public static final Integer SUCCESS = new Integer(1);
        public static final String RESULT_CODE = "result-code";
        public static final String RESULT_TEXT = "result-text";
        public static final String OBJECT_CLASS_ID = "login";
        public static final String SIGN_IN = "sign-in";
        public static final String STATUS = "sign-in-status";
        public static final String SIGN_OUT = "sign-out";

        public Hashtable action(Hashtable params);
    }
}
