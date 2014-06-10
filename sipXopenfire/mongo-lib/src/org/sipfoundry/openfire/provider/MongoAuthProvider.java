/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 */
package org.sipfoundry.openfire.provider;

import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.auth.AuthFactory;
import org.jivesoftware.openfire.auth.ConnectionException;
import org.jivesoftware.openfire.auth.InternalUnauthenticatedException;
import org.jivesoftware.openfire.auth.UnauthorizedException;
import org.jivesoftware.openfire.provider.AuthProvider;
import org.jivesoftware.openfire.user.UserNotFoundException;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;

public class MongoAuthProvider implements AuthProvider {
    public static final String SUPERADMIN = "superadmin";
    private String m_xmppDomain;

    @Override
    public void authenticate(String userName, String password) throws UnauthorizedException, ConnectionException,
            InternalUnauthenticatedException {
        User user = UnfortunateLackOfSpringSupportFactory.getValidUsers().getUserByInsensitiveJid(
                getUserName(userName));
        if (user == null) {
            throw new UnauthorizedException("no user");
        }
        if (!user.getUserName().equals(SUPERADMIN) && !user.isImEnabled()) {
            throw new UnauthorizedException("im not enabled for user");
        }
        if (!user.getPintoken().equals(password)) {
            throw new UnauthorizedException("wrong password");
        }
    }

    @Override
    public void authenticate(String username, String token, String digest) throws UnauthorizedException,
            ConnectionException, InternalUnauthenticatedException {
        if (username == null || token == null || digest == null) {
            throw new UnauthorizedException();
        }
        User user = UnfortunateLackOfSpringSupportFactory.getValidUsers().getUserByInsensitiveJid(
                getUserName(username));
        if (user == null) {
            throw new UnauthorizedException("no user");
        }
        if (!user.isImEnabled()) {
            throw new UnauthorizedException("im not enabled for user");
        }
        String anticipatedDigest = AuthFactory.createDigest(token, user.getPintoken());
        if (!digest.equalsIgnoreCase(anticipatedDigest)) {
            throw new UnauthorizedException("bad digest");
        }
    }

    @Override
    public String getPassword(String userName) throws UserNotFoundException, UnsupportedOperationException {
        throw new UnsupportedOperationException("get password not supported");
    }

    @Override
    public boolean isDigestSupported() {
        return true;
    }

    @Override
    public boolean isPlainSupported() {
        return true;
    }

    @Override
    public void setPassword(String arg0, String arg1) throws UserNotFoundException, UnsupportedOperationException {
        throw new UnsupportedOperationException("set password not supported");
    }

    @Override
    public boolean supportsPasswordRetrieval() {
        return false;
    }

    private String getDomain() {
        if (m_xmppDomain == null) {
            m_xmppDomain = XMPPServer.getInstance().getServerInfo().getXMPPDomain();
        }
        return m_xmppDomain;
    }

    private String getUserName(String username) throws UnauthorizedException {
        String newUsername = username.trim().toLowerCase();
        if (newUsername.contains("@")) {
            // Check that the specified domain matches the server's domain
            int index = newUsername.indexOf("@");
            String domain = newUsername.substring(index + 1);
            if (domain.equals(getDomain())) {
                newUsername = newUsername.substring(0, index);
            } else {
                // Unknown domain. Return authentication failed.
                throw new UnauthorizedException("unknow domain");
            }
        }
        return newUsername;
    }

}
