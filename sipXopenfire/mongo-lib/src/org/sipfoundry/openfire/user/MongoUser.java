/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 */
package org.sipfoundry.openfire.user;

import java.util.Date;
import java.util.HashMap;
import java.util.Map;

import org.jivesoftware.openfire.user.User;

/**
 * Was only used by {@link MongoUserProvider}
 */
public class MongoUser extends User {

    private Map<String, String> m_properties;

    public MongoUser() {

    }

    public MongoUser(String username, String name, String email, Date creationDate, Date modificationDate) {
        super(username, name, email, creationDate, modificationDate);
    }

    @Override
    public Map<String, String> getProperties() {
        if (m_properties == null) {
            m_properties = new HashMap<String, String>();
        }
        return m_properties;
    }

}
