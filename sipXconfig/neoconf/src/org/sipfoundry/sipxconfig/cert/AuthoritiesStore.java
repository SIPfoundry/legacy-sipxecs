/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import java.io.IOException;
import java.io.OutputStream;
import java.io.StringReader;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.Security;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

import org.bouncycastle.jce.provider.BouncyCastleProvider;

public class AuthoritiesStore {
    private static final String PROVIDER = "BC"; // bouncycastle
    private KeyStore m_store;

    static {
        Security.addProvider(new BouncyCastleProvider());
    }

    public AuthoritiesStore() {
        try {
            m_store = KeyStore.getInstance("PKCS12", PROVIDER);
            m_store.load(null, null);
        } catch (NoSuchProviderException e) {
            throw new RuntimeException(e);
        } catch (KeyStoreException e) {
            throw new RuntimeException(e);
        } catch (CertificateException e) {
            throw new RuntimeException(e);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException(e);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void addAuthority(String authority, String certText) throws IOException {
        try {
            X509Certificate cert = AbstractCertificateGenerator.readCertificate(new StringReader(certText));
            m_store.setCertificateEntry(authority, cert);
        } catch (KeyStoreException e) {
            throw new RuntimeException(e);
        }
    }

    public void store(OutputStream out) throws IOException {
        try {
            m_store.store(out, "".toCharArray());
        } catch (KeyStoreException e) {
            throw new RuntimeException(e);
        } catch (CertificateException e) {
            throw new RuntimeException(e);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException(e);
        }
    }
}
