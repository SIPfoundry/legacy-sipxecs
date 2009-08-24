/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.security;

import org.acegisecurity.providers.dao.SaltSource;
import org.acegisecurity.userdetails.UserDetails;

public class SaltSourceImpl implements SaltSource {
    public Object getSalt(UserDetails userDetails) {
        if (userDetails instanceof UserDetailsImpl) {
            UserDetailsImpl impl = (UserDetailsImpl) userDetails;
            return impl.getCanonicalUserName();
        }
        return null;
    }
}
