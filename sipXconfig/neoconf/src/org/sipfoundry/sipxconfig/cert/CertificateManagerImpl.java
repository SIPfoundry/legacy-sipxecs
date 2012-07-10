/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import static java.lang.String.format;

import java.io.IOException;
import java.io.InputStream;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateNotYetValidException;
import java.security.cert.X509Certificate;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.springframework.jdbc.core.JdbcTemplate;

/**
 * Certificate Management Implementation.
 */
public class CertificateManagerImpl extends SipxHibernateDaoSupport implements CertificateManager, SetupListener {
    private static final Log LOG = LogFactory.getLog(CertificateManager.class);
    private static final String WEB_CERT = "ssl-web";
    private static final String COMM_CERT = "ssl";
    private static final String AUTHORITY_TABLE = "authority";
    private static final String CERT_TABLE = "cert";
    private static final String CERT_COLUMN = "data";
    private static final String KEY_COLUMN = "private_key";
    private static final String SELF_SIGN_AUTHORITY_PREFIX = "ca.";
    private BeanWithSettingsDao<CertificateSettings> m_settingsDao;
    private LocationsManager m_locationsManager;
    private JdbcTemplate m_jdbc;
    private ConfigManager m_configManager;
    private List<String> m_thirdPartyAuthorites;

    public CertificateSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    public void saveSettings(CertificateSettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public void setWebCertificate(String cert) {
        setWebCertificate(cert, null);
    }

    @Override
    public void setWebCertificate(String cert, String key) {
        validateCert(cert, key);
        updateCertificate(WEB_CERT, cert, key, getSelfSigningAuthority());
    }

    @Override
    public String getNamedPrivateKey(String id) {
        return getSecurityData(CERT_TABLE, KEY_COLUMN, id);
    }

    @Override
    public String getNamedCertificate(String id) {
        return getSecurityData(CERT_TABLE, CERT_COLUMN, id);
    }

    @Override
    public void setCommunicationsCertificate(String cert) {
        setCommunicationsCertificate(cert, null);
    }

    public void setCommunicationsCertificate(String cert, String key) {
        validateCert(cert, key);
        updateCertificate(COMM_CERT, cert, key, getSelfSigningAuthority());
    }

    void updateCertificate(String name, String cert, String key, String authority) {
        updateNamedCertificate(name, cert, key, authority);
        m_configManager.configureEverywhere(FEATURE);
    }

    @Override
    public void updateNamedCertificate(String name, String cert, String key, String authority) {
        m_jdbc.update("delete from cert where name = ?", name);
        m_jdbc.update("insert into cert (name, data, private_key, authority) values (?, ?, ?, ?)", name, cert, key,
                authority);
    }

    void addThirdPartyAuthority(String name, String data) {
        addAuthority(name, data, null);
    }

    void addAuthority(String name, String data, String key) {
        m_jdbc.update("delete from authority where name = ? ", name);
        m_jdbc.update("delete from cert where authority = ? ", name); // should be zero
        m_jdbc.update("insert into authority (name, data, private_key) values (?, ?, ?)", name, data, key);
        m_configManager.configureEverywhere(FEATURE);
    }

    String getSecurityData(String table, String column, String name) {
        String sql = format("select %s from %s where name = ?", column, table);
        return DaoUtils.requireOneOrZero(m_jdbc.queryForList(sql, String.class, name), sql);
    }

    @Override
    public String getWebCertificate() {
        return getSecurityData(CERT_TABLE, CERT_COLUMN, WEB_CERT);
    }

    @Override
    public String getWebPrivateKey() {
        return getSecurityData(CERT_TABLE, KEY_COLUMN, WEB_CERT);
    }

    @Override
    public String getCommunicationsCertificate() {
        return getSecurityData(CERT_TABLE, CERT_COLUMN, COMM_CERT);
    }

    @Override
    public String getCommunicationsPrivateKey() {
        return getSecurityData(CERT_TABLE, KEY_COLUMN, COMM_CERT);
    }

    @Override
    public List<String> getThirdPartyAuthorities() {
        List<String> authorities = m_jdbc.queryForList("select name from authority where name != ? order by name",
                String.class, getSelfSigningAuthority());
        return authorities;
    }

    @Override
    public List<String> getAuthorities() {
        List<String> authorities = m_jdbc.queryForList("select name from authority order by name", String.class);
        return authorities;
    }

    @Override
    public String getAuthorityCertificate(String authority) {
        return getSecurityData(AUTHORITY_TABLE, CERT_COLUMN, authority);
    }

    @Override
    public String getAuthorityKey(String authority) {
        return getSecurityData(AUTHORITY_TABLE, KEY_COLUMN, authority);
    }

    @Override
    public String getSelfSigningAuthority() {
        String domain = Domain.getDomain().getName();
        return SELF_SIGN_AUTHORITY_PREFIX + domain;
    }

    @Override
    public String getSelfSigningAuthorityText() {
        return getAuthorityCertificate(getSelfSigningAuthority());
    }

    @Override
    public void addTrustedAuthority(String authority, String cert) {
        validateAuthority(cert);
        addAuthority(authority, cert, null);
    }

    @Override
    public void rebuildSelfSignedData() {
        forceDeleteTrustedAuthority(getSelfSigningAuthority());
        checkSetup();
    }

    @Override
    public void rebuildCommunicationsCert() {
        rebuildCert(COMM_CERT);
    }

    @Override
    public void rebuildWebCert() {
        rebuildCert(WEB_CERT);
    }

    void rebuildCert(String type) {
        String domain = Domain.getDomain().getName();
        String fqdn = m_locationsManager.getPrimaryLocation().getFqdn();
        String authority = getSelfSigningAuthority();
        String issuer = getIssuer(authority);
        String authKey = getAuthorityKey(authority);
        CertificateGenerator gen;
        if (type.equals(COMM_CERT)) {
            gen = CertificateGenerator.sip(domain, fqdn, issuer, authKey);
        } else {
            gen = CertificateGenerator.web(domain, fqdn, issuer, authKey);
        }
        updateCertificate(type, gen.getCertificateText(), gen.getPrivateKeyText(), authority);
    }

    @Override
    public void deleteTrustedAuthority(String authority) {
        if (authority.equals(getSelfSigningAuthority())) {
            throw new UserException("Cannot delete self signing certificate authority");
        }

        forceDeleteTrustedAuthority(authority);
    }

    void forceDeleteTrustedAuthority(String authority) {
        m_jdbc.update("delete from authority where name = ?", authority);
        m_jdbc.update("delete from cert where authority = ?", authority);
        m_configManager.configureEverywhere(FEATURE);
    }

    void checkSetup() {
        String domain = Domain.getDomain().getName();
        String authority = getSelfSigningAuthority();
        String authorityCertificate = getAuthorityCertificate(authority);
        if (authorityCertificate == null) {
            CertificateAuthorityGenerator gen = new CertificateAuthorityGenerator(domain);
            addAuthority(authority, gen.getCertificateText(), gen.getPrivateKeyText());
        }
        for (String thirdPartAuth : m_thirdPartyAuthorites) {
            if (getAuthorityCertificate(thirdPartAuth) == null) {
                try {
                    InputStream thirdPartIn = getClass().getResourceAsStream(thirdPartAuth);
                    if (thirdPartIn == null) {
                        throw new IOException("Missing resource " + thirdPartAuth);
                    }
                    String thirdPartCert = IOUtils.toString(thirdPartIn);
                    String thirdPartAuthId = CertificateUtils.stripPath(thirdPartAuth);
                    addThirdPartyAuthority(thirdPartAuthId, thirdPartCert);
                } catch (IOException e) {
                    LOG.error("Cannot import authority " + thirdPartAuth, e);
                }
            }
        }

        String fqdn = m_locationsManager.getPrimaryLocation().getFqdn();
        String issuer = getIssuer(authority);
        String authKey = getAuthorityKey(authority);
        if (!hasCertificate(COMM_CERT, authority)) {
            CertificateGenerator gen = CertificateGenerator.sip(domain, fqdn, issuer, authKey);
            updateCertificate(COMM_CERT, gen.getCertificateText(), gen.getPrivateKeyText(), authority);
        }
        if (!hasCertificate(WEB_CERT, authority)) {
            CertificateGenerator gen = CertificateGenerator.web(domain, fqdn, issuer, authKey);
            updateCertificate(WEB_CERT, gen.getCertificateText(), gen.getPrivateKeyText(), authority);
        }
    }

    boolean hasCertificate(String id, String authority) {
        int check = m_jdbc.queryForInt("select count(*) from cert where name = ? and authority = ?", id, authority);
        return (check >= 1);
    }

    String getIssuer(String authority) {
        String authCertText = getSecurityData(AUTHORITY_TABLE, CERT_COLUMN, authority);
        X509Certificate authCert = CertificateUtils.readCertificate(authCertText);
        return authCert.getSubjectDN().getName();
    }

    void validateCert(String certTxt, String keyTxt) {
        X509Certificate cert = CertificateUtils.readCertificate(certTxt);
        try {
            cert.checkValidity();
        } catch (CertificateExpiredException e) {
            throw new UserException("Certificate has expired.");
        } catch (CertificateNotYetValidException e) {
            throw new UserException("Certificate valid date range is in the future, it is not yet valid.");
        }
        if (StringUtils.isNotBlank(keyTxt)) {
            CertificateUtils.readCertificateKey(keyTxt);
        }
        // to do, validate key w/cert and cert w/authorities
    }

    void validateAuthority(String cert) {
        validateCert(cert, null);
        // to do validate authority cert
    }

    @Override
    public boolean setup(SetupManager manager) {
        checkSetup();
        return true;
    }

    public void setJdbc(JdbcTemplate jdbc) {
        m_jdbc = jdbc;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setSettingsDao(BeanWithSettingsDao<CertificateSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public void setThirdPartyAuthorites(List<String> thirdPartyAuthorites) {
        m_thirdPartyAuthorites = thirdPartyAuthorites;
    }
}
