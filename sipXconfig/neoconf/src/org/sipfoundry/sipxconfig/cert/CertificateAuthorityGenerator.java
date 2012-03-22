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

import org.bouncycastle.cert.X509v3CertificateBuilder;

public class CertificateAuthorityGenerator extends AbstractCertificateGenerator {
    public CertificateAuthorityGenerator(String domain) {
        super(domain, "ca");

        // historical defaults, otherwise insignificant AFAIK
        setValidYears(10);
        setBitCount(2048);
    }

    @Override
    public X509Certificate createCertificate() throws GeneralSecurityException {
        KeyPair pair = getKeyPair();
        String issuer = getSubject();
        X509v3CertificateBuilder gen = createCertificateGenerator(issuer, pair.getPublic());
        return CertificateUtils.generateCert(gen, getAlgorithm(), pair.getPrivate());
    }
}
