/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.security;

import org.acegisecurity.providers.UsernamePasswordAuthenticationToken;

public class DigestUsernamePasswordAuthenticationToken extends UsernamePasswordAuthenticationToken {

    public DigestUsernamePasswordAuthenticationToken(Object principal, Object credentials) {
        super(principal, credentials);
    }

}
