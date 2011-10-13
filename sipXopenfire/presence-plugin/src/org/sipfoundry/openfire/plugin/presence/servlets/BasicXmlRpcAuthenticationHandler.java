package org.sipfoundry.openfire.plugin.presence.servlets;

import java.security.Principal;

import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.XmlRpcRequest;
import org.apache.xmlrpc.common.XmlRpcHttpRequestConfig;
import org.apache.xmlrpc.server.AbstractReflectiveHandlerMapping.AuthenticationHandler;
import org.apache.commons.codec.digest.DigestUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.commons.util.DomainConfiguration;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;

public class BasicXmlRpcAuthenticationHandler implements AuthenticationHandler{

    private static Logger logger = Logger.getLogger(BasicXmlRpcAuthenticationHandler.class);
    private String m_sharedSecret = null;
    private String m_sipRealm = null;

    public BasicXmlRpcAuthenticationHandler() {
        DomainConfiguration config = new DomainConfiguration(System.getProperty("conf.dir")+"/domain-config");
        m_sharedSecret = config.getSharedSecret();
        m_sipRealm = config.getSipRealm();
    }

    private String digestPassword(String user, String realm, String password) {
        String full = user + ':' + realm + ':' + password;
        String digest = DigestUtils.md5Hex(full);
        return digest;
    }

    private boolean isAuthenticated(String username, String password) {
        if(username == null ||password == null) {
            logger.warn("Could not authenticate since no username or password was provided");
            return false;
        }
        if(password.equals(m_sharedSecret)) {
            return true;
        }
        try {
            User user = UnfortunateLackOfSpringSupportFactory.getValidUsers().getUser(username);
            if (user == null) {
                return false;
            } else {
                String digestPassword = digestPassword(username, m_sipRealm, password);
                return user.getPintoken().equals(digestPassword);
            }
        } catch (Exception ex) {
            logger.warn("Could not authenticate "+username);
            return false;
        }
    }

    @Override
    public boolean isAuthorized(XmlRpcRequest pRequest) throws XmlRpcException {
        XmlRpcHttpRequestConfig config = (XmlRpcHttpRequestConfig) pRequest.getConfig();
        return isAuthenticated(config.getBasicUserName(), config.getBasicPassword());
    }

}
