/**
 *  * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  * Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  * Licensed to the User under the LGPL license.
 *     
*/

package org.sipfoundry.commons.sipkeystorebuilder;

import org.apache.commons.ssl.KeyStoreBuilder;
import org.apache.commons.ssl.TrustChain;
import org.apache.commons.ssl.TrustMaterial;
import org.apache.commons.ssl.Util;

import java.io.File;
import java.io.FilenameFilter;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.security.KeyStore;

/**
 * Builds Java Key Store and Trust Store files out of openssl keys and certs.
 */
public class sipkeystorebuilder {
    protected sipkeystorebuilder () {
    }
    
    public static void main(String[] args) throws Exception {
        if (args.length != 1) {
            System.out.println("sipkeystorebuilder:  creates '[sip or sip-web].keystore' (Java Key Store) and java truststore '[authority].jks");
            System.out.println("[alias] will be set to the first CN value of the X509 certificate.");
            System.out.println("-------------------------------------------------------------------");
            System.out.println("Usage: [sipX ssl directory]");
            System.out.println("-------------------------------------------------------------------");
            System.exit(1);
        }
        char[] password = "changeit".toCharArray();
        final String auth = "authorities";
        final String keySuffix = ".key";
        final String certSuffix = ".crt";
        final String keystoreSuffix = ".keystore";
        final String truststoreSuffix = ".jks";

        File ssldir = new File(args[0]);
        File authdir = new File(args[0] + "/" + auth);

        FilenameFilter keyFilter = new FilenameFilter() {
           public boolean accept(File dir, String name) {
             return name.endsWith(keySuffix);
           }
        };
        FilenameFilter certFilter = new FilenameFilter() {
           public boolean accept(File dir, String name) {
             return name.endsWith(certSuffix);
           }
        };

        if (ssldir.isDirectory()) {
           // Valid directory specified.  Now scan for key and cert files to build the KeyStore.
           KeyStore[] SipXKeyStores = new KeyStore[2];
           File[] keyfiles = ssldir.listFiles(keyFilter);
           for (int i = 0; i < keyfiles.length; i++) {
              // get the matching certificate file.
              String certPath = keyfiles[i].toString().replaceAll(keySuffix,certSuffix);
              File certFile = new File(certPath);
              FileInputStream fin1 = new FileInputStream(keyfiles[i].toString());
              byte[] bytes1 = Util.streamToBytes(fin1);
              FileInputStream fin2 = new FileInputStream(certFile.toString());
              byte[] bytes2 = Util.streamToBytes(fin2);
              SipXKeyStores[i] = KeyStoreBuilder.build(bytes1, bytes2, password);
              File outks = new File(keyfiles[i].toString().replaceAll(keySuffix, keystoreSuffix));
              FileOutputStream fout = new FileOutputStream(outks);
              SipXKeyStores[i].store(fout, password);
              fout.flush();
              fout.close();

           }
        }
        if (authdir.isDirectory()) {
           // Valid authority directory specified.  Now scan for cert files to build the TrustStore.
           File[] certFiles = authdir.listFiles(certFilter);
           TrustChain trustChain = new TrustChain();
           for (int i = 0; i < certFiles.length; i++) {
              TrustMaterial trustCerts = new TrustMaterial(certFiles[i].toString());
              trustChain.addTrustMaterial(trustCerts);
           }
           File outts = new File(args[0] + "/" + auth + truststoreSuffix);
           FileOutputStream fout = new FileOutputStream(outts);
           KeyStore trustKeyStore = trustChain.getUnifiedKeyStore();
           trustKeyStore.store(fout, password);
           fout.flush();
           fout.close();
        }

        System.exit(0);
    }
}
