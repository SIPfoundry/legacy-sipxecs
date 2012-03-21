/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.io.IOException;

import org.apache.axis.utils.StringUtils;
import org.apache.commons.io.IOUtils;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.request.IUploadFile;
import org.sipfoundry.sipxconfig.cert.CertificateManager;
import org.sipfoundry.sipxconfig.cert.CertificateRequestGenerator;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.setting.Setting;

public abstract class ManageCertificates extends SipxBasePage implements PageBeginRenderListener {

    public static final String PAGE = "admin/ManageCertificates";
    public static final Integer UPLOAD = new Integer(1);
    public static final Integer TEXT = new Integer(2);
    public static final String GENERATE_CSR_TAB = "generate";
    public static final String IMPORT_WEB_CERT_TAB = "import";
    public static final String CERT_AUTH_TAB = "authorities";

    private static final String COUNTRY_PROP = "countryName";
    private static final String STATE_PROP = "stateOrProvinceName";
    private static final String LOCALITY_PROP = "localityName";
    private static final String ORGANIZATION_PROP = "organizationName";
    private static final String EMAIL_PROP = "serverEmail";
    private static final String SERVER_NAME = "serverName";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject(value = "spring:certificateManager")
    public abstract CertificateManager getCertificateManager();

    @InjectObject(value = "spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @Persist
    @InitialValue(value = "literal:generate")
    public abstract String getTab();

    public abstract void setTab(String tab);

    public abstract String getCountry();

    public abstract void setCountry(String country);

    public abstract String getState();

    public abstract void setState(String state);

    public abstract String getLocality();

    public abstract void setLocality(String locality);

    public abstract String getOrganization();

    public abstract void setOrganization(String organization);

    public abstract String getDefaultServer();

    public abstract void setDefaultServer(String defaultServer);

    public abstract String getServer();

    public abstract void setServer(String server);

    public abstract String getEmail();

    public abstract void setEmail(String email);

    public abstract Setting getCsrSettings();

    public abstract void setCsrSettings(Setting settings);

    public abstract String getCsr();

    public abstract void setCsr(String csr);

    public abstract IUploadFile getUploadCrtFile();

    public abstract IUploadFile getUploadKeyFile();

    public abstract String getCertificate();

    public abstract String getKey();

    @InitialValue(value = "@org.sipfoundry.sipxconfig.site.admin.ManageCertificates@UPLOAD")
    public abstract Integer getImportMethodSelected();

    public void pageBeginRender(PageEvent event_) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        setDefaultServer(getLocationsManager().getPrimaryLocation().getFqdn());
        setServer(getDefaultServer());

        SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils.getValidator(this);

        Setting settings = getCsrSettings();
        if (settings == null) {
            settings = getCertificateManager().getSettings().getSettings().getSetting("csr");
            setCsrSettings(settings);
        }
    }

    public void generate() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }

        SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils.getValidator(this);
        CertificateManager mgr = getCertificateManager();
        String web = mgr.getWebCertificate();
        String key = mgr.getWebPrivateKey();
        Location primary = getLocationsManager().getPrimaryLocation();
        String domain = Domain.getDomain().getName();
        CertificateRequestGenerator csr = new CertificateRequestGenerator(domain, primary.getHostname());
        setCsr(csr.getCertificateRequestText(web, key));

        validator.recordSuccess(getMessages().getMessage("msg.generateSuccess"));
    }

    public void importCertificate() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }

        IUploadFile uploadCrtFile = getUploadCrtFile();
        IUploadFile uploadKeyFile = getUploadKeyFile();
        String certificate = getCertificate();
        String key = getKey();

        SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils.getValidator(this);
        if (uploadCrtFile == null && certificate == null) {
            validator.record(new UserException("&msg.selectOneSource"), getMessages());
            return;
        }

        String fname = null;
        try {
            if (uploadCrtFile != null) {
                fname = uploadCrtFile.getFileName();
                certificate = IOUtils.toString(uploadCrtFile.getStream());
            }

            if (uploadKeyFile != null) {
                fname = uploadKeyFile.getFileName();
                key = IOUtils.toString(uploadKeyFile.getStream());
            }
            CertificateManager mgr = getCertificateManager();
            if (StringUtils.isEmpty(key)) {
                mgr.setThirdPartyTrustedWebCertificate(certificate);
            } else {
                mgr.setSelfSignedWebCertificate(certificate, key);
            }
            validator.recordSuccess(getMessages().getMessage("msg.importSuccess"));
        } catch (IOException e) {
            validator.record(new UserException("&msg.readError", fname), getMessages());
        }
    }

    public boolean getUploadDisabled() {
        return getImportMethodSelected() != UPLOAD;
    }

    public boolean getTextDisabled() {
        return getImportMethodSelected() != TEXT;
    }
}
