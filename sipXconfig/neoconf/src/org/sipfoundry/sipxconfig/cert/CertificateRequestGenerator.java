/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import java.io.StringWriter;
import java.security.GeneralSecurityException;
import java.security.PrivateKey;
import java.security.cert.X509Certificate;
import java.util.Set;
import java.util.Vector;

import org.bouncycastle.asn1.ASN1ObjectIdentifier;
import org.bouncycastle.asn1.DEROctetString;
import org.bouncycastle.asn1.DERSet;
import org.bouncycastle.asn1.pkcs.Attribute;
import org.bouncycastle.asn1.pkcs.PKCSObjectIdentifiers;
import org.bouncycastle.asn1.x509.X509Extension;
import org.bouncycastle.asn1.x509.X509Extensions;
import org.bouncycastle.jce.PKCS10CertificationRequest;
import org.bouncycastle.jce.X509Principal;
import org.sipfoundry.sipxconfig.common.UserException;

public class CertificateRequestGenerator extends AbstractCertificateCommon {
    private String m_algorithm = "SHA1WithRSAEncryption";

    public CertificateRequestGenerator(String domain, String fqdn) {
        super(domain, fqdn);
    }

    /**
     * Take an existing certificate and private key and generate a CSR from that with new company
     * details but use cert's public key and other details.
     *
     * Many deprecated calls, but there's no documentation on what the new calls are
     */
    public String getCertificateRequestText(String certTxt, String keyTxt) {
        X509Certificate cert = CertificateUtils.readCertificate(certTxt);
        PrivateKey key = CertificateUtils.readCertificateKey(keyTxt);
        try {
            X509Principal subject = new X509Principal(getSubject());
            Vector<ASN1ObjectIdentifier> oids = new Vector<ASN1ObjectIdentifier>();
            Vector<X509Extension> values = new Vector<X509Extension>();
            copyExtensions(cert, cert.getNonCriticalExtensionOIDs(), false, oids, values);
            copyExtensions(cert, cert.getCriticalExtensionOIDs(), true, oids, values);
            X509Extensions extensions = new X509Extensions(oids, values);
            Attribute attribute = new Attribute(PKCSObjectIdentifiers.pkcs_9_at_extensionRequest, new DERSet(
                    extensions));
            PKCS10CertificationRequest csr = new PKCS10CertificationRequest(m_algorithm, subject,
                    cert.getPublicKey(), new DERSet(attribute), key);
            StringWriter data = new StringWriter();
            CertificateUtils.writeObject(data, csr, null);
            return data.toString();
        } catch (GeneralSecurityException e) {
            throw new UserException(e);
        } catch (IllegalArgumentException e) {
            throw new UserException(e);
        }
    }

    void copyExtensions(X509Certificate cert, Set<String> in, boolean critical, Vector<ASN1ObjectIdentifier> out,
            Vector<X509Extension> values) {
        for (String oid : in) {
            out.add(new ASN1ObjectIdentifier(oid));
            values.add(new X509Extension(critical, new DEROctetString(cert.getExtensionValue(oid))));
        }
    }
}
