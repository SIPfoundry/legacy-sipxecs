/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import static java.lang.String.format;

import java.io.StringReader;
import java.security.GeneralSecurityException;
import java.security.KeyPair;
import java.security.PrivateKey;
import java.security.cert.X509Certificate;

import org.apache.commons.lang.StringUtils;
import org.bouncycastle.asn1.DEROctetString;
import org.bouncycastle.asn1.x509.BasicConstraints;
import org.bouncycastle.asn1.x509.X509Extensions;
import org.bouncycastle.jce.X509Principal;
import org.bouncycastle.x509.X509V3CertificateGenerator;
import org.bouncycastle.x509.extension.SubjectKeyIdentifierStructure;

public class CertificateGenerator extends AbstractCertificateGenerator {
    private String m_issuer;
    private String m_sipDomain;
    private String m_authorityKey;
    private boolean m_sip;

    protected CertificateGenerator(String domain, String hostname, String issuer, String authorityKey) {
        super(domain, hostname);
        m_issuer = issuer;
        m_authorityKey = authorityKey;
    }

    public static CertificateGenerator web(String domain, String hostname, String issuer, String authorityKey) {
        return new CertificateGenerator(domain, hostname, issuer, authorityKey);
    }

    public static CertificateGenerator sip(String sipDomain, String issuer, String authorityKey) {
        CertificateGenerator gen = new CertificateGenerator(sipDomain, "", issuer, authorityKey);
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
        X509V3CertificateGenerator gen = createCertificateGenerator();
        gen.setIssuerDN(new X509Principal(m_issuer));
        KeyPair pair = getKeyPair();
        gen.setPublicKey(pair.getPublic());
        gen.addExtension(X509Extensions.SubjectKeyIdentifier, false,
                new SubjectKeyIdentifierStructure(pair.getPublic()));
        String alt = format("DNS:%s.%s", getHost(), getDnsDomain());
        if (StringUtils.isNotBlank(m_sipDomain)) {
            alt = format("URI:sip:%s,%s", m_sipDomain, alt);
        }
        gen.addExtension(X509Extensions.SubjectAlternativeName, false, new DEROctetString(alt.getBytes()));
        gen.addExtension(X509Extensions.BasicConstraints, true, new BasicConstraints(0));
        return gen.generate(pair.getPrivate());
    }

    PrivateKey getAuthorityPrivateKey() {
        return readCertificateKey(new StringReader(m_authorityKey));
    }
}
