/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;


import java.io.StringWriter;
import java.math.BigInteger;
import java.security.GeneralSecurityException;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.PrivateKey;
import java.security.cert.X509Certificate;
import java.util.Calendar;
import java.util.Date;

import org.bouncycastle.jce.X509Principal;
import org.bouncycastle.x509.X509V3CertificateGenerator;
import org.sipfoundry.sipxconfig.common.UserException;

public abstract class AbstractCertificateGenerator extends AbstractCertificateCommon {
    private int m_validYears = 3; // historical default, otherwise insignificant AFAIK
    private KeyPair m_keys;
    private X509Certificate m_certificate;

    protected AbstractCertificateGenerator(String domain, String hostname) {
        super(domain, hostname);
    }

    public abstract X509Certificate createCertificate() throws GeneralSecurityException;

    public X509Certificate getCertificate() {
        if (m_certificate == null) {
            try {
                m_certificate = createCertificate();
            } catch (GeneralSecurityException e) {
                throw new UserException("Could not generate certificate " + e.getMessage(), e);
            }
        }

        return m_certificate;
    }

    public static String getDescription(X509Certificate cert) {
        StringBuilder buf = new StringBuilder();
        String nl = System.getProperty("line.separator");
        buf.append("Version: ").append(cert.getVersion()).append(nl);
        buf.append("Serial Number: ").append(cert.getSerialNumber()).append(nl);
        buf.append("Signature Algorithm: ").append(cert.getSigAlgName()).append(nl);
        buf.append("Issuer: ").append(cert.getIssuerDN()).append(nl);
        buf.append("Not Before: ").append(cert.getNotBefore()).append(nl);
        buf.append("Not After: ").append(cert.getNotAfter()).append(nl);
        buf.append("Subject: ").append(cert.getSubjectDN()).append(nl);
        return buf.toString();
    }

    X509V3CertificateGenerator createCertificateGenerator() {
        X509V3CertificateGenerator gen = new X509V3CertificateGenerator();
        // use minutes in the epoch as the initial serial number
        // to prevent clashes when the admin wipes out the system and starts over.
        long minSinceEpoc = System.currentTimeMillis() / (1000 * 60);
        gen.setSerialNumber(BigInteger.valueOf(minSinceEpoc));
        Calendar cal = Calendar.getInstance();
        // cert valid 1hr ago just to be sure we're covered.
        cal.add(Calendar.HOUR, -1);
        Date start = cal.getTime();
        cal.add(Calendar.YEAR, getValidYears());
        Date end = cal.getTime();
        gen.setNotBefore(start);
        gen.setNotAfter(end);
        gen.setSubjectDN(new X509Principal(getSubject()));
        gen.setSignatureAlgorithm(getAlgorithm());
        return gen;
    }

    public String getPrivateKeyText() {
        PrivateKey key = getKeyPair().getPrivate();
        StringWriter data = new StringWriter();
        writeObject(data, key, null);
        return data.toString();
    }

    public String getCertificateText() {
        X509Certificate cert = getCertificate();
        StringWriter data = new StringWriter();
        writeObject(data, cert, getDescription(cert));
        return data.toString();
    }

    public int getValidYears() {
        return m_validYears;
    }

    public void setValidYears(int validYears) {
        m_validYears = validYears;
    }

    public KeyPair getKeyPair() {
        if (m_keys != null) {
            return m_keys;
        }
        try {
            KeyPairGenerator kpg = KeyPairGenerator.getInstance("RSA", PROVIDER);
            kpg.initialize(getBitCount());
            m_keys = kpg.genKeyPair();
            return m_keys;
        } catch (GeneralSecurityException e) {
            throw new RuntimeException(e);
        }
    }
}
