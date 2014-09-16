/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
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
