/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import static org.sipfoundry.sipxconfig.cert.CertificateUtils.x500;

import java.io.StringWriter;
import java.math.BigInteger;
import java.security.GeneralSecurityException;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.cert.X509Certificate;
import java.util.Calendar;
import java.util.Date;

import org.bouncycastle.asn1.x509.SubjectPublicKeyInfo;
import org.bouncycastle.cert.X509v3CertificateBuilder;
import org.sipfoundry.sipxconfig.common.UserException;

public abstract class AbstractCertificateGenerator extends AbstractCertificateCommon {
    public static final String NETSCAPE_CERT_TYPE_OID = "2.16.840.1.113730.1.1";
    public static final String NETSCAPE_COMMENT_OID = "2.16.840.1.113730.1.13";
    private int m_validYears = 3; // historical default, otherwise insignificant AFAIK
    private KeyPair m_keys;
    private X509Certificate m_certificate;

    protected AbstractCertificateGenerator(String domain, String fqdn) {
        super(domain, fqdn);
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

    X509v3CertificateBuilder createCertificateGenerator(String issuer, PublicKey pub) {
        // use minutes in the epoch as the initial serial number
        // to prevent clashes when the admin wipes out the system and starts over.
        BigInteger serialNum = BigInteger.valueOf(System.currentTimeMillis() / (1000 * 60));
        Calendar cal = Calendar.getInstance();
        // cert valid 1hr ago just to be sure we're covered.
        cal.add(Calendar.HOUR, -1);
        Date start = cal.getTime();
        cal.add(Calendar.YEAR, getValidYears());
        Date end = cal.getTime();
        SubjectPublicKeyInfo pubKeyInfo = SubjectPublicKeyInfo.getInstance(pub.getEncoded());
        X509v3CertificateBuilder gen = new X509v3CertificateBuilder(x500(issuer), serialNum, start, end,
                x500(getSubject()), pubKeyInfo);
        return gen;
    }

    public String getPrivateKeyText() {
        PrivateKey key = getKeyPair().getPrivate();
        StringWriter data = new StringWriter();
        CertificateUtils.writeObject(data, key, null);
        return data.toString();
    }

    public String getCertificateText() {
        X509Certificate cert = getCertificate();
        StringWriter data = new StringWriter();
        CertificateUtils.writeObject(data, cert, getDescription(cert));
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
            KeyPairGenerator kpg = KeyPairGenerator.getInstance("RSA", CertificateUtils.getProvider());
            kpg.initialize(getBitCount());
            m_keys = kpg.genKeyPair();
            return m_keys;
        } catch (GeneralSecurityException e) {
            throw new RuntimeException(e);
        }
    }
}
