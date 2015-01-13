/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.common.UserException;

public class CertificateUtilsTest {

    @Test
    public void readValidCert() throws IOException {
        Reader in = new InputStreamReader(getClass().getResourceAsStream("test.crt"));
        CertificateUtils.readCertificate(in);
        in.close();        
    }
    
    @Test(expected = UserException.class)
    public void readInvalidCert() {
        Reader in = new StringReader("Invalid cert");
        CertificateUtils.readCertificate(in);
    }    

    @Test
    public void play() {
        assertEquals("c", CertificateUtils.stripPath("a/b/c"));
        assertEquals("c", CertificateUtils.stripPath("c"));
        assertEquals("", CertificateUtils.stripPath("c/"));
        assertEquals("", CertificateUtils.stripPath(""));
        assertEquals(null, CertificateUtils.stripPath(null));
    }
    
    @Test
    public void createKeystore() {
        JavaKeyStore sslOpenfire = new JavaKeyStore();
        
        try {
            sslOpenfire.addKeys("ezuce.com", 
                IOUtils.toString(getClass().getResourceAsStream("certificate.crt")), 
                IOUtils.toString(getClass().getResourceAsStream("certificate.key")));          
        } catch (FileNotFoundException e) {
            fail("File not found");
        } catch (IOException e) {
            fail("IO exception");
        }        
    }
}
