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
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.request.IUploadFile;
import org.sipfoundry.sipxconfig.cert.CertificateManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class ImportCertificate extends BaseComponent {
    public static final Integer UPLOAD = new Integer(1);
    public static final Integer TEXT = new Integer(2);
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

    @InitialValue(value = "@org.sipfoundry.sipxconfig.site.admin.ImportCertificate@UPLOAD")
    public abstract Integer getImportMethodSelected();

    @Parameter(required = true)
    public abstract String getCertificateType();

    public void rebuild() {
        if (getCertificateType().equals(WEB)) {
            getCertificateManager().rebuildWebCert();
        } else {
            getCertificateManager().rebuildCommunicationsCert();
        }
        getValidator().recordSuccess(getMessages().getMessage("msg.rebuild.success"));
    }

    public String getCertificateName() {
        if (getCertificateType().equals(WEB)) {
            return getCertificateManager().WEB_CERT + getCertificateManager().CRT;
        }
        return getCertificateManager().COMM_CERT + getCertificateManager().CRT;
    }

    public String getCertificateText() {
        if (getCertificateType().equals(WEB)) {
            return getCertificateManager().getWebCertificate();
        }
        return getCertificateManager().getCommunicationsCertificate();
    }

    public String getChainCertificateName() {
        return getCertificateManager().CHAIN_CERT + getCertificateManager().CRT;
    }

    public String getChainCertificateText() {
        return getCertificateManager().getChainCertificate();
    }

    public String getCACertificateName() {
        return getCertificateManager().CA_CERT + getCertificateManager().CRT;
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
                        mgr.setWebCertificate(certificate);
                    } else {
                        mgr.setWebCertificate(certificate, key);
                    }
                }
                if (!StringUtils.isBlank(chainCertificate)) {
                    mgr.setChainCertificate(chainCertificate);
                }

                if (!StringUtils.isBlank(caCertificate)) {
                    mgr.setCACertificate(caCertificate);
                }
            } else {
                if (StringUtils.isBlank(key)) {
                    mgr.setCommunicationsCertificate(certificate);
                } else {
                    mgr.setCommunicationsCertificate(certificate, key);
                }
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
