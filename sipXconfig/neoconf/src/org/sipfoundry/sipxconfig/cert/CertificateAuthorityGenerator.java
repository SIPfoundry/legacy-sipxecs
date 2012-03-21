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

import org.bouncycastle.jce.X509Principal;
import org.bouncycastle.x509.X509V3CertificateGenerator;

public class CertificateAuthorityGenerator extends AbstractCertificateGenerator {
    public CertificateAuthorityGenerator(String domain) {
        super(domain, "ca");

        // historical defaults, otherwise insignificant AFAIK
        setValidYears(10);
        setBitCount(2048);
    }

    public X509Certificate createCertificate() throws GeneralSecurityException {
        X509V3CertificateGenerator gen = createCertificateGenerator();
        gen.setIssuerDN(new X509Principal(getSubject()));
        KeyPair pair = getKeyPair();
        gen.setPublicKey(pair.getPublic());
        return gen.generate(pair.getPrivate());
    }
}
