/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipximbot;

import org.apache.commons.lang.StringUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;

/**
 * Holds the valid user data needed for the AutoAttendant, parsing from IMDB XML files
 * 
 */
public enum FullUsers {
    INSTANCE;
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipximbot");

    /**
     * See if a given user_name is valid (aka it can be dialed and reach a user)
     * 
     * @param userNname
     * 
     * @return user found or null
     */
    public User isValidUser(String userName) {
        User user = UnfortunateLackOfSpringSupportFactory.getValidUsers().getUser(userName);
        if (user != null) {
            String jid = user.getJid() + "@" + ImbotConfiguration.get().getSipxchangeDomainName();
            user.setJid(jid);
            return user;
        }
        return null;
    }

    public User findByjid(String jid) {
        String id = StringUtils.remove(jid, "@" + ImbotConfiguration.get().getSipxchangeDomainName());
        User user = UnfortunateLackOfSpringSupportFactory.getValidUsers().getUserByInsensitiveJid(id);
        if (user != null) {
            user.setJid(jid);
        }
        return user;
    }
    
    public User findByConfName(String confName) {
        User user = UnfortunateLackOfSpringSupportFactory.getValidUsers().getUserByConferenceName(confName);
        if (user != null) {
            String jid = user.getJid() + "@" + ImbotConfiguration.get().getSipxchangeDomainName();
            user.setJid(jid);
            return user;
        }
        return null;
    }
    

}
