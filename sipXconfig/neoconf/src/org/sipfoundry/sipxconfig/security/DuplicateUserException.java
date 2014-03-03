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

package org.sipfoundry.sipxconfig.security;

import java.util.List;

import org.sipfoundry.sipxconfig.common.User;
import org.springframework.security.core.AuthenticationException;

public class DuplicateUserException extends AuthenticationException {
    private static final long serialVersionUID = 1L;

    private List<User> m_users;

    public DuplicateUserException(String msg, List<User> users) {
        super(msg);
        m_users = users;
    }

    public List<User> getUsers() {
        return m_users;
    }
}
