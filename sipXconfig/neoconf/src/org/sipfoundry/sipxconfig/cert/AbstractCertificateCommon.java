/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import static java.lang.String.format;

public class AbstractCertificateCommon {
    public static final int DEFAULT_KEY_SIZE = 2048;

    private String m_country = "US";
    private String m_state = "AnyState";
    private String m_locality = "AnyTown";
    private String m_organization;
    private String m_organizationUnit = "sipXecs";
    private String m_commonName;
    private String m_email;
    private String m_dnsDomain;
    private String m_host;
    private int m_bitCount = DEFAULT_KEY_SIZE;
    private String m_algorithm = "SHA1WithRSAEncryption";

    protected AbstractCertificateCommon(String domain, String fqdn) {
        setDnsDomain(domain);
        setEmail("root@" + domain);
        setCommonName(fqdn);
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
        return format("C=%s, ST=%s, L=%s, O=%s, OU=%s, CN=%s, EMAILADDRESS=%s", m_country, m_state, m_locality,
                m_dnsDomain, m_organizationUnit, m_commonName, m_email);
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
