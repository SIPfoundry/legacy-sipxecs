/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import static java.lang.String.format;

import java.io.IOException;
import java.io.Reader;
import java.io.Writer;
import java.security.KeyPair;
import java.security.PrivateKey;
import java.security.Security;
import java.security.cert.X509Certificate;

import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.openssl.PEMReader;
import org.bouncycastle.openssl.PEMWriter;
import org.sipfoundry.sipxconfig.common.UserException;

public class AbstractCertificateCommon {
    protected static final String PROVIDER = "BC";

    private String m_country = "US";
    private String m_state = "AnyState";
    private String m_locality = "AnyTown";
    private String m_organization;
    private String m_organizationUnit = "IT";
    private String m_commonName;
    private String m_email;
    private String m_dnsDomain;
    private String m_host;
    private int m_bitCount = 1024;
    private String m_algorithm = "SHA1WithRSAEncryption";

    static {
        Security.addProvider(new BouncyCastleProvider());
    }

    protected AbstractCertificateCommon(String domain, String hostname) {
        setDnsDomain(domain);
        setEmail("root@" + domain);
        setCommonName(hostname + '.' + domain);
    }

    public static X509Certificate readCertificate(Reader in) {
        Object o = readObject(in);
        if (!(o instanceof X509Certificate)) {
            String msg = format("Certificate was expected but found %s instead", o.getClass().getSimpleName());
            throw new UserException(msg);
        }
        return (X509Certificate) o;
    }

    public static Object readObject(Reader in) {
        PEMReader rdr = new PEMReader(in);
        try {
            Object o = rdr.readObject();
            if (o == null) {
                throw new UserException("No recognized security information was found. Files "
                        + "should be in PEM style format.");
            }
            return o;
        } catch (IOException e) {
            throw new UserException("Error reading certificate. " + e.getMessage(), e);
        }
    }

    public static PrivateKey readCertificateKey(Reader in) {
        Object o = readObject(in);
        if (o instanceof KeyPair) {
            return ((KeyPair) o).getPrivate();
        }
        if (o instanceof PrivateKey) {
            return (PrivateKey) o;
        }

        String msg = format("Private key was expected but found %s instead", o.getClass().getSimpleName());
        throw new UserException(msg);
    }

    public static void writeObject(Writer w, Object o, String description) {
        PEMWriter pw = new PEMWriter(w);
        try {
            if (description != null) {
                w.write(description);
            }
            pw.writeObject(o);
            pw.close();
        } catch (IOException e) {
            throw new UserException("Problem updating certificate authority. " + e.getMessage(), e);
        }
    }

    public String getCountry() {
        return m_country;
    }

    public void setCountry(String country) {
        m_country = country;
    }

    public String getState() {
        return m_state;
    }

    public void setState(String state) {
        m_state = state;
    }

    /**
     * In the US this would be the town as an example.
     */
    public String getLocality() {
        return m_locality;
    }

    public void setLocality(String town) {
        m_locality = town;
    }

    public String getOrganization() {
        return m_organization;
    }

    public void setOrganization(String organization) {
        m_organization = organization;
    }

    public String getOrganizationUnit() {
        return m_organizationUnit;
    }

    public void setOrganizationUnit(String organizationUnit) {
        m_organizationUnit = organizationUnit;
    }

    public String getCommonName() {
        return m_commonName;
    }

    public void setCommonName(String commonName) {
        m_commonName = commonName;
    }

    public String getEmail() {
        return m_email;
    }

    public void setEmail(String email) {
        m_email = email;
    }

    public String getDnsDomain() {
        return m_dnsDomain;
    }

    public void setDnsDomain(String dnsDomain) {
        m_dnsDomain = dnsDomain;
    }

    public String getSubject() {
        return format("C=%s, ST=%s, L=%s, O=%s, OU=%s, CN=%s/emailAddress=%s", m_country, m_state, m_locality,
                m_organization, m_organizationUnit, m_commonName, m_email);
    }

    public String getAlgorithm() {
        return m_algorithm;
    }

    public void setAlgorithm(String algorithm) {
        m_algorithm = algorithm;
    }

    public int getBitCount() {
        return m_bitCount;
    }

    public void setBitCount(int bitCount) {
        m_bitCount = bitCount;
    }

    public String getHost() {
        return m_host;
    }
}
