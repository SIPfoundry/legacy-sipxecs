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

import java.util.Properties;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.request.IUploadFile;
import org.sipfoundry.sipxconfig.admin.CertificateManager;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class ManageCertificates extends BasePage implements PageBeginRenderListener {

    public static final String PAGE = "ManageCertificates";
    public static final Integer UPLOAD = new Integer(1);
    public static final Integer TEXT = new Integer(2);

    private static final String COUNTRY_PROP = "countryName";
    private static final String STATE_PROP = "stateOrProvinceName";
    private static final String LOCALITY_PROP = "localityName";
    private static final String ORGANIZATION_PROP = "organizationName";
    private static final String EMAIL_PROP = "serverEmail";
    private static final String SERVER_NAME = "serverName";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject(value = "spring:certificateManagerImpl")
    public abstract CertificateManager getCertificateManager();

    @InjectObject(value = "spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @Persist
    @InitialValue(value = "literal:generate")
    public abstract String getTab();

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

        try {
            // load properties
            Properties properties = getCertificateManager().loadCertPropertiesFile();
            if (properties != null) {
                setCountry(properties.getProperty(COUNTRY_PROP));
                setState(properties.getProperty(STATE_PROP));
                setLocality(properties.getProperty(LOCALITY_PROP));
                setOrganization(properties.getProperty(ORGANIZATION_PROP));
                setEmail(properties.getProperty(EMAIL_PROP));
                String server = properties.getProperty(SERVER_NAME);
                server = (server == null) ? getDefaultServer() : server;
                setServer(server);

                // load csr
                String csr = getCertificateManager().readCSRFile(getServer());
                setCsr(csr);
            }
        } catch (UserException e) {
            validator.record(e, getMessages());
        }
    }

    public void generate() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }

        SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils.getValidator(this);

        // write properties file
        Properties properties = new Properties();
        properties.setProperty(COUNTRY_PROP, getCountry());
        properties.setProperty(STATE_PROP, getState());
        properties.setProperty(LOCALITY_PROP, getLocality());
        properties.setProperty(ORGANIZATION_PROP, getOrganization());
        properties.setProperty(EMAIL_PROP, getEmail());
        properties.setProperty(SERVER_NAME, getServer());
        getCertificateManager().writeCertPropertiesFile(properties);

        // generate key and csr files
        getCertificateManager().generateCSRFile();

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
        boolean isCsrBased = uploadKeyFile == null && key == null;

        if (uploadCrtFile == null && certificate == null) {
            validator.record(new UserException("&msg.selectOneSource"), getMessages());
            return;
        }

        if (isCsrBased) {
            if (uploadCrtFile != null) {
                uploadCrtFile.write(getCertificateManager().getCRTFile(getServer()));
            } else if (certificate != null) {
                getCertificateManager().writeCRTFile(certificate, getServer());
            }
        } else {
            if (uploadCrtFile != null) {
                uploadCrtFile.write(getCertificateManager().getExternalCRTFile());
            } else if (certificate != null) {
                getCertificateManager().writeExternalCRTFile(certificate);
            }
        }

        if (uploadKeyFile != null) {
            uploadKeyFile.write(getCertificateManager().getExternalKeyFile());
        } else if (key != null) {
            getCertificateManager().writeKeyFile(key);
        }

        getCertificateManager().importKeyAndCertificate(getServer(), isCsrBased);

        validator.recordSuccess(getMessages().getMessage("msg.importSuccess"));
    }

    public boolean getUploadDisabled() {
        return getImportMethodSelected() != UPLOAD;
    }

    public boolean getTextDisabled() {
        return getImportMethodSelected() != TEXT;
    }
}
