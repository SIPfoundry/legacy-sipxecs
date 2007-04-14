/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.login;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Md5Encoder;
import org.sipfoundry.sipxconfig.common.User;

public class LoginContextImpl implements LoginContext {
    private CoreContext m_coreContext;

    /**
     * Returns user if credentials check out.
     * Return null if the user does not exist or the password is wrong.
     * The userId may be either the userName or an alias.
     */
    public User checkCredentials(String userNameOrAlias, String password) {
        User user = m_coreContext.loadUserByUserNameOrAlias(userNameOrAlias);
        if (user == null) {
            return null;
        }
        
        String userName = user.getUserName();
        String pintoken = user.getPintoken();
        String encodedPassword = getEncodedPassword(userName, password);
        
        // Real match
        if (encodedPassword.equals(pintoken)) {
            return user;
        }
        
        // Special case: if the password is empty and the pintoken is empty, then declare a match.
        // We have publicized the ability for admins to reset users to have an empty password by
        // zeroing out the pintoken entry in the database.
        if (StringUtils.isBlank(password) && pintoken.length() == 0) {
            return user;
        }

        return null;
    }

    public String getEncodedPassword(String userName, String password) {
        return Md5Encoder.digestPassword(userName, m_coreContext.getAuthorizationRealm(), password);
    }
    
    public boolean isAdmin(Integer userId) {
        User user = m_coreContext.loadUser(userId);
        return isAdmin(user);
    }

    public boolean isAdmin(User user) {
        if (user == null) {
            return false;
        }
        return user.isAdmin();
    }
    
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
