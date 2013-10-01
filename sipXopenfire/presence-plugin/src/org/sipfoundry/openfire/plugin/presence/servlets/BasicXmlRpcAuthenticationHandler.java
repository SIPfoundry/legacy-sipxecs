package org.sipfoundry.openfire.plugin.presence.servlets;

import org.apache.log4j.Logger;
import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.XmlRpcRequest;
import org.apache.xmlrpc.common.XmlRpcHttpRequestConfig;
import org.apache.xmlrpc.server.AbstractReflectiveHandlerMapping.AuthenticationHandler;
import org.sipfoundry.commons.security.Md5Encoder;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.util.DomainConfiguration;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;

public class BasicXmlRpcAuthenticationHandler implements AuthenticationHandler {

    private static Logger logger = Logger.getLogger(BasicXmlRpcAuthenticationHandler.class);
    private String m_sharedSecret = null;

    private boolean isAuthenticated(String username, String password) {
        if (m_sharedSecret == null) {
            DomainConfiguration config = new DomainConfiguration(System.getProperty("conf.dir")+"/domain-config");
            m_sharedSecret = config.getSharedSecret();
        }

        if (username == null || password == null) {
            logger.warn("Could not authenticate since no username or password was provided");
            return false;
        }
        if (password.equals(m_sharedSecret)) {
            return true;
        }
        try {
            User user = UnfortunateLackOfSpringSupportFactory.getValidUsers().getUser(username);
            if (user == null) {
                return false;
            }
            String encodedPassword = Md5Encoder.getEncodedPassword(password);
            return user.getPintoken().equals(encodedPassword);
        } catch (Exception ex) {
            logger.warn("Could not authenticate " + username);
            return false;
        }
    }

    @Override
    public boolean isAuthorized(XmlRpcRequest pRequest) throws XmlRpcException {
        XmlRpcHttpRequestConfig config = (XmlRpcHttpRequestConfig) pRequest.getConfig();
        return isAuthenticated(config.getBasicUserName(), config.getBasicPassword());
    }

}
