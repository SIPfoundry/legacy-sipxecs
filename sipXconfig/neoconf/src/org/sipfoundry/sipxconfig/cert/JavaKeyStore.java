/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.KeyStore;
import java.security.KeyStore.Entry;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class JavaKeyStore {
    private static final Log LOG = LogFactory.getLog(JavaKeyStore.class);
    private KeyStore m_store;

    // historical password that jdk proper uses in it's default keystore. not meant
    // to be a big secret or that anyone should need to actually "changeit".
    private char[] m_password = "changeit".toCharArray();

    // Default store type, not need to change it although bouncycastle does not write out
    // to JKS, but we don't need it to at this time
    private String m_type = "JKS";

    public JavaKeyStore() {
        try {
            m_store = KeyStore.getInstance(m_type);
            m_store.load(null, null);
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
            X509Certificate cert = CertificateUtils.readCertificate(certText);
            m_store.setCertificateEntry(authority, cert);
        } catch (KeyStoreException e) {
            throw new RuntimeException(e);
        }
    }

    public void addKey(String alias, String certText, String keyText) throws IOException {
        try {
            PrivateKey key = CertificateUtils.readCertificateKey(keyText);
            X509Certificate[] cert = new X509Certificate[] {
                CertificateUtils.readCertificate(certText)
            };
            m_store.setKeyEntry(alias, key, m_password, cert);
        } catch (KeyStoreException e) {
            throw new RuntimeException(e);
        }
    }

    public void addKeys(String alias, String certText, String keyText) throws IOException {
        try {
            PrivateKey key = CertificateUtils.readCertificateKey(keyText);
            X509Certificate[] cert = CertificateUtils.readCertificates(certText);

            m_store.setKeyEntry(alias, key, m_password, cert);
        } catch (KeyStoreException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Compare entries from one keystore with another
     *
     * @return false if they differ
     */
    public boolean isEqual(InputStream bStream) {
        try {
            KeyStore b = KeyStore.getInstance(m_type);
            b.load(bStream, m_password);
            Set<String> bAliases = toSet(b.aliases());
            Set<String> aAliases = toSet(m_store.aliases());
            KeyStore.PasswordProtection keyPass = new KeyStore.PasswordProtection(m_password);
            if (!aAliases.equals(bAliases)) {
                return false;
            }

            for (String alias : aAliases) {

                // weird, when not using password, cert require null and keys
                // require the password given to the filestore.
                KeyStore.PasswordProtection password = keyPass;
                if (m_store.entryInstanceOf(alias, KeyStore.TrustedCertificateEntry.class)) {
                    password = null;
                }

                Entry aEntry = m_store.getEntry(alias, password);
                Entry bEntry = b.getEntry(alias, password);
                if (!isEqual(aEntry, bEntry)) {
                    return false;
                }
            }

            return true;
        } catch (Exception e) {
            LOG.error("Could not read store", e);
            return false;
        }
    }

    public boolean isEqual(KeyStore.Entry a, KeyStore.Entry b) {
        if (!a.getClass().equals(b.getClass())) {
            return false;
        }
        if (a instanceof KeyStore.PrivateKeyEntry) {
            KeyStore.PrivateKeyEntry aKey = (KeyStore.PrivateKeyEntry) a;
            KeyStore.PrivateKeyEntry bKey = (KeyStore.PrivateKeyEntry) b;
            if (!aKey.getPrivateKey().equals(bKey.getPrivateKey())) {
                return false;
            }
            if (!ArrayUtils.isEquals(aKey.getCertificateChain(), bKey.getCertificateChain())) {
                return false;
            }
            return true;
        }

        if (a instanceof KeyStore.TrustedCertificateEntry) {
            KeyStore.TrustedCertificateEntry aCert = (KeyStore.TrustedCertificateEntry) a;
            KeyStore.TrustedCertificateEntry bCert = (KeyStore.TrustedCertificateEntry) b;
            return aCert.getTrustedCertificate().equals(bCert.getTrustedCertificate());
        }

        if (a instanceof KeyStore.SecretKeyEntry) {
            KeyStore.SecretKeyEntry aSecret = (KeyStore.SecretKeyEntry) a;
            KeyStore.SecretKeyEntry bSecret = (KeyStore.SecretKeyEntry) b;
            return (aSecret.getSecretKey().equals(bSecret.getSecretKey()));
        }

        LOG.error("Unrecognized keystore entry " + a.getClass());
        return false;
    }

    Set<String> toSet(Enumeration<String> e) {
        Set<String> set = new HashSet<String>();
        while (e.hasMoreElements()) {
            set.add(e.nextElement());
        }
        return set;
    }

    public void store(OutputStream out) throws IOException {
        try {
            m_store.store(out, m_password);
        } catch (KeyStoreException e) {
            throw new RuntimeException(e);
        } catch (CertificateException e) {
            throw new RuntimeException(e);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Only saves store if entries have changed. It's been discovered if you write out the same
     * key store into 2 different files, the checksum of the files differ. Normally one wouldn't
     * care, but if you have logic that needs to detect a change in file contents then you'll need
     * to use this method.
     */
    public void storeIfDifferent(File f) {
        if (f.exists()) {
            InputStream in = null;
            try {
                in = new FileInputStream(f);
                if (isEqual(in)) {
                    return;
                }
            } catch (FileNotFoundException e) {
                throw new RuntimeException(e);
            } finally {
                IOUtils.closeQuietly(in);
            }
        }
        OutputStream out = null;
        try {
            out = new FileOutputStream(f);
            store(out);
        } catch (FileNotFoundException e) {
            throw new RuntimeException(e);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(out);
        }
    }

    public void setPassword(String password) {
        m_password = password.toCharArray();
    }

    public String getPassword() {
        return new String(m_password);
    }
}
