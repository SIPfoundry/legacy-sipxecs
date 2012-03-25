/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotSame;
import static org.junit.Assert.assertTrue;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.test.TestHelper;


public class JavaKeyStoreTest {          
    
    @Test
    public void store() throws IOException, KeyStoreException, NoSuchAlgorithmException, CertificateException {
        JavaKeyStore store = new JavaKeyStore();
        String certText = IOUtils.toString(getClass().getResourceAsStream("ca.test.crt"));        
        ByteArrayOutputStream actual = new ByteArrayOutputStream();
        store.addAuthority("ca.test.crt", certText);
        store.store(actual);
        
        KeyStore ks = KeyStore.getInstance(java.security.KeyStore.getDefaultType());
        ByteArrayInputStream in = new ByteArrayInputStream(actual.toByteArray());        
        ks.load(in, store.getPassword().toCharArray());
        
        FileUtils.writeByteArrayToFile(new File("/tmp/foo"), actual.toByteArray());
    }
    
    @Test
    public void equals() throws IOException {
        JavaKeyStore a = new JavaKeyStore();
        JavaKeyStore b = new JavaKeyStore();
        String cert = IOUtils.toString(getClass().getResourceAsStream("test.crt"));        
        String key = IOUtils.toString(getClass().getResourceAsStream("test.key"));        
        a.addKey("test", cert, key);
        b.addKey("test", cert, key);
        
        ByteArrayOutputStream bOut = new ByteArrayOutputStream();
        b.store(bOut);
        InputStream bIn = new ByteArrayInputStream(bOut.toByteArray());
        assertTrue(a.isEqual(bIn));
    }
    
    @Test
    public void storeIfDifferent() throws IOException {
        JavaKeyStore a = new JavaKeyStore();
        String cert = IOUtils.toString(getClass().getResourceAsStream("test.crt"));        
        String key = IOUtils.toString(getClass().getResourceAsStream("test.key"));        
        a.addKey("test", cert, key);        
        File f = new File(TestHelper.getTestDirectory(), "store-if-different-test.keystore");
        if (f.exists()) {
            f.delete();
        }
        
        a.storeIfDifferent(f);
        long checksumBefore = FileUtils.checksumCRC32(f);
        a.storeIfDifferent(f);
        long checksumAfter = FileUtils.checksumCRC32(f);
        assertEquals(checksumBefore, checksumAfter);
        a.addKey("test2", cert, key);
        a.storeIfDifferent(f);
        long checksumAfterChange = FileUtils.checksumCRC32(f);
        assertNotSame(checksumBefore, checksumAfterChange);
    }
    
    @Test
    public void storeIfDifferentAuth() throws IOException {
        String cert = IOUtils.toString(getClass().getResourceAsStream("ca.test.crt"));        
        JavaKeyStore a = storeFromCert(cert);
        File f = new File(TestHelper.getTestDirectory(), "store-auth-if-different-test.keystore");
        if (f.exists()) {
            f.delete();
        }
        
        a.storeIfDifferent(f);
        long checksumBefore = FileUtils.checksumCRC32(f);
        JavaKeyStore b = storeFromCert(cert);
        b.storeIfDifferent(f);
        long checksumAfter = FileUtils.checksumCRC32(f);
        assertEquals(checksumBefore, checksumAfter);
        JavaKeyStore c = storeFromCert(cert);
        c.addAuthority("test2", cert);
        c.storeIfDifferent(f);
        long checksumAfterChange = FileUtils.checksumCRC32(f);
        assertNotSame(checksumBefore, checksumAfterChange);
    }
    
    JavaKeyStore storeFromCert(String cert) throws IOException {
        JavaKeyStore a = new JavaKeyStore();
        a.addAuthority("test", cert);        
        return a;
    }
}
