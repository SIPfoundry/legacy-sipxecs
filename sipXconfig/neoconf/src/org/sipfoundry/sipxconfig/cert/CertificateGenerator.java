/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;


import static java.lang.String.format;

import java.security.GeneralSecurityException;
import java.security.KeyPair;
import java.security.PrivateKey;
import java.security.cert.X509Certificate;

import org.apache.commons.lang.StringUtils;
import org.bouncycastle.asn1.DEROctetString;
import org.bouncycastle.asn1.x509.BasicConstraints;
import org.bouncycastle.asn1.x509.X509Extension;
import org.bouncycastle.cert.CertIOException;
import org.bouncycastle.cert.X509v3CertificateBuilder;
import org.bouncycastle.x509.extension.SubjectKeyIdentifierStructure;

public class CertificateGenerator extends AbstractCertificateGenerator {
    private String m_issuer;
    private String m_sipDomain;
    private String m_authorityKey;

    protected CertificateGenerator(String domain, String hostname, String issuer, String authorityKey) {
        super(domain, hostname);
        m_issuer = issuer;
        m_authorityKey = authorityKey;
    }

    public static CertificateGenerator web(String domain, String hostname, String issuer, String authorityKey) {
        return new CertificateGenerator(domain, hostname, issuer, authorityKey);
    }

    public static CertificateGenerator sip(String sipDomain, String hostname, String issuer, String authorityKey) {
        CertificateGenerator gen = new CertificateGenerator(sipDomain, hostname, issuer, authorityKey);
        gen.m_sipDomain = sipDomain;
        return gen;
    }

    public String getSipDomain() {
        return m_sipDomain;
    }

    public String getIssuer() {
        return m_issuer;
    }

    @Override
    public X509Certificate createCertificate() throws GeneralSecurityException {
        try {
            KeyPair pair = getKeyPair();
            X509v3CertificateBuilder gen = createCertificateGenerator(m_issuer, pair.getPublic());
            gen.addExtension(X509Extension.subjectKeyIdentifier, false,
                    new SubjectKeyIdentifierStructure(pair.getPublic()));
            String alt = format("DNS:%s", getCommonName());
            if (StringUtils.isNotBlank(m_sipDomain)) {
                alt = format("URI:sip:%s,%s", m_sipDomain, alt);
            }
            gen.addExtension(X509Extension.subjectAlternativeName, false, new DEROctetString(alt.getBytes()));
            gen.addExtension(X509Extension.basicConstraints, true, new BasicConstraints(0));
            return CertificateUtils.generateCert(gen, getAlgorithm(), getAuthorityPrivateKey());
        } catch (CertIOException e) {
            throw new GeneralSecurityException(e);
        }
    }

    PrivateKey getAuthorityPrivateKey() {
        return CertificateUtils.readCertificateKey(m_authorityKey);
    }
}
