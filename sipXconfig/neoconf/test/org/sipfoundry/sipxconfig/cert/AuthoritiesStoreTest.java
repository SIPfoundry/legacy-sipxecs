/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

import org.apache.commons.io.IOUtils;
import org.junit.Test;


public class AuthoritiesStoreTest {
    
    @Test
    public void store() throws IOException {
        AuthoritiesStore store = new AuthoritiesStore();
        String certText = IOUtils.toString(getClass().getResourceAsStream("ca.test.crt"));        
        ByteArrayOutputStream actual = new ByteArrayOutputStream();
        store.addAuthority("ca.test.crt", certText);
        store.store(actual);
        System.out.print(actual.size());
    }
}
