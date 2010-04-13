
/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */

package org.sipfoundry.commons.x509selector;
import java.io.PrintStream;
import java.security.NoSuchAlgorithmException;

import javax.net.ssl.KeyManagerFactory;

/**
 * Jetty defaults to SunX509 algorithm which is not available on IBM's VM.  This finds the 
 * available X509 algorithms and returns one.
 */
public class X509Selector {

    public static void main(String[] args) {
        PrintStream out = System.out;
        out.print(new X509Selector().getAvailableAlgorithm());
    }

    public String getAvailableAlgorithm() {
        String[] algs = new String[] {
            "SunX509", "IbmX509"
        };
        return getAvailableAlgorithm(algs);
    }

    String getAvailableAlgorithm(String[] algs) {
        String retval = null;
        for (int i = 0; i < algs.length; i++) {
            try {
                KeyManagerFactory.getInstance(algs[i]);
                retval =  algs[i];
                break;
            } catch (NoSuchAlgorithmException e) {
               
            }
        }
        if ( retval == null ) {
            System.err.println("Could not find avaliable X.509 algorithm." );
        }
        return retval;
    }

}


