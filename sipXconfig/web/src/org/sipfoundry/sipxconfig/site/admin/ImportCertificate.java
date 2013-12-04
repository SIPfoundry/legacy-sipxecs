/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.io.IOException;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.request.IUploadFile;
import org.sipfoundry.sipxconfig.cert.AbstractCertificateCommon;
import org.sipfoundry.sipxconfig.cert.CertificateManager;
import org.sipfoundry.sipxconfig.cert.CertificateRequestGenerator;
import org.sipfoundry.sipxconfig.cert.CertificateSettings;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.common.IntegerPropertySelectionModel;

public abstract class ImportCertificate extends BaseComponent implements PageBeginRenderListener {
    public static final Integer UPLOAD = new Integer(1);
    public static final Integer TEXT = new Integer(2);
    private static final Log LOG = LogFactory.getLog(ImportCertificate.class);

    private static final String WEB = "web";

    public abstract IUploadFile getUploadCrtFile();

    public abstract IUploadFile getUploadKeyFile();

    public abstract IUploadFile getUploadChainCertificateFile();

    public abstract IUploadFile getUploadCACertificateFile();

    public abstract String getCertificate();

    public abstract String getKey();

    public abstract String getChainCertificate();

    public abstract String getCaCertificate();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject(value = "spring:certificateManager")
    public abstract CertificateManager getCertificateManager();

    @InjectObject(value = "spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InitialValue(value = "@org.sipfoundry.sipxconfig.site.admin.ImportCertificate@UPLOAD")
    public abstract Integer getImportMethodSelected();

    @Parameter(required = true)
    public abstract String getCertificateType();

    @Persist
    public abstract void setKeySize(int keySize);

    public abstract int getKeySize();

    public Setting getCsrSettings() {
        return getSettings().getSettings().getSetting("csr");
    }

    public abstract void setSettings(CertificateSettings settings);

    public abstract CertificateSettings getSettings();

    public abstract String getCsr();

    public abstract void setCsr(String csr);

    public abstract String getKeySizeDescr();

    public abstract void setKeySizeDescr(String descr);

    public void rebuild() {
        if (getCertificateType().equals(WEB)) {
            getCertificateManager().rebuildWebCert(getKeySize());
        } else {
            getCertificateManager().rebuildCommunicationsCert(getKeySize());
        }
        getValidator().recordSuccess(getMessages().getMessage("msg.rebuild.success"));
    }

    public String getCertificateName() {
        if (getCertificateType().equals(WEB)) {
            return CertificateManager.WEB_CERT + CertificateManager.CRT;
        }
        return CertificateManager.COMM_CERT + CertificateManager.CRT;
    }

    public String getCertificateText() {
        if (getCertificateType().equals(WEB)) {
            return getCertificateManager().getWebCertificate();
        }
        return getCertificateManager().getCommunicationsCertificate();
    }

    public static String getChainCertificateName() {
        return CertificateManager.CHAIN_CERT + CertificateManager.CRT;
    }

    public String getChainCertificateText() {
        return getCertificateManager().getChainCertificate();
    }

    public static String getCACertificateName() {
        return CertificateManager.CA_CERT + CertificateManager.CRT;
    }

    public String getCACertificateText() {
        return getCertificateManager().getCACertificate();
    }

    public void importCertificate() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }

        IUploadFile uploadCrtFile = getUploadCrtFile();
        IUploadFile uploadKeyFile = getUploadKeyFile();
        IUploadFile uploadChainCertificateFile = getUploadChainCertificateFile();
        IUploadFile uploadCACertificateFile = getUploadCACertificateFile();

        String certificate = getCertificate();
        String key = getKey();
        String chainCertificate = getChainCertificate();
        String caCertificate = getCaCertificate();

        SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils.getValidator(this);
        if ((uploadCrtFile == null && certificate == null)
                && (uploadChainCertificateFile == null && chainCertificate == null)
                && (uploadCACertificateFile == null && caCertificate == null)) {
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

            if (uploadChainCertificateFile != null) {
                fname = uploadChainCertificateFile.getFileName();
                chainCertificate = IOUtils.toString(uploadChainCertificateFile.getStream());
            }
            if (uploadCACertificateFile != null) {
                fname = uploadCACertificateFile.getFileName();
                caCertificate = IOUtils.toString(uploadCACertificateFile.getStream());
            }

            CertificateManager mgr = getCertificateManager();
            if (getCertificateType().equals(WEB)) {
                if (!StringUtils.isBlank(certificate)) {
                    if (StringUtils.isBlank(key)) {
                        key = mgr.getWebPrivateKey();
                    }
                    mgr.setWebCertificate(certificate, key);
                }
                if (!StringUtils.isBlank(chainCertificate)) {
                    mgr.setChainCertificate(chainCertificate);
                }

                if (!StringUtils.isBlank(caCertificate)) {
                    mgr.setCACertificate(caCertificate);
                }
            } else {
                if (StringUtils.isBlank(key)) {
                    key = mgr.getCommunicationsPrivateKey();
                }
                mgr.setCommunicationsCertificate(certificate, key);
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

    public IPropertySelectionModel getKeySizeModel() {
        return new IntegerPropertySelectionModel(this, new int[] {
            1024, 2048, 4096
        });
    }

    public void generate() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }

        SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils.getValidator(this);
        CertificateManager mgr = getCertificateManager();
        CertificateSettings settings = getSettings();
        mgr.saveSettings(settings);

        String cert;
        String key;
        if (getCertificateType().equals(WEB)) {
            cert = mgr.getWebCertificate();
            key = mgr.getWebPrivateKey();
        } else {
            cert = mgr.getCommunicationsCertificate();
            key = mgr.getCommunicationsPrivateKey();
        }
        Location primary = getLocationsManager().getPrimaryLocation();
        String domain = Domain.getDomain().getName();
        CertificateRequestGenerator csr = new CertificateRequestGenerator(domain, primary.getFqdn());
        settings.updateCertificateDetails(csr);
        setCsr(csr.getCertificateRequestText(cert, key));

        validator.recordSuccess(getMessages().getMessage("msg.generateSuccess"));
    }

    @Override
    public void pageBeginRender(PageEvent evt) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        String key;
        if (getCertificateType().equals(WEB)) {
            key = getCertificateManager().getWebPrivateKey();
        } else {
            key = getCertificateManager().getCommunicationsPrivateKey();
        }

        String size;
        try {
            size = String.valueOf(CertificateUtils.getEncryptionStrength(key));
        } catch (Exception e) {
            LOG.error("Could not retrieve encryption strength for current key: " + e.getMessage());
            size = "undetermined";
        }

        setKeySizeDescr(getMessages().format("description.keySize", size));

        if (getKeySize() == 0) {
            setKeySize(AbstractCertificateCommon.DEFAULT_KEY_SIZE);
        }

        CertificateSettings settings = getSettings();
        if (settings == null) {
            settings = getCertificateManager().getSettings();
            setSettings(settings);
        }
    }
}
