/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import java.security.GeneralSecurityException;
import java.security.KeyPair;
import java.security.cert.X509Certificate;

import org.bouncycastle.asn1.x509.BasicConstraints;
import org.bouncycastle.asn1.x509.X509Extension;
import org.bouncycastle.cert.CertIOException;
import org.bouncycastle.cert.X509v3CertificateBuilder;

public class CertificateAuthorityGenerator extends AbstractCertificateGenerator {
    public CertificateAuthorityGenerator(String domain, String host) {
        super(domain, host);

        // historical defaults, otherwise insignificant AFAIK
        setValidYears(10);
        setBitCount(2048);
    }

    public CertificateAuthorityGenerator(String domain) {
        this(domain, "ca");
    }

    @Override
    public X509Certificate createCertificate() throws GeneralSecurityException {
        try {
            KeyPair pair = getKeyPair();
            String issuer = getSubject();
            X509v3CertificateBuilder gen = createCertificateGenerator(issuer, pair.getPublic());
            gen.addExtension(X509Extension.basicConstraints, true, new BasicConstraints(0));
            return CertificateUtils.generateCert(gen, getAlgorithm(), pair.getPrivate());
        } catch (CertIOException e) {
            throw new GeneralSecurityException(e);
        }
    }
}
