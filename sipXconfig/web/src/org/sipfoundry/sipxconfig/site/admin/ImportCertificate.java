/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.io.IOException;

import org.apache.axis.utils.StringUtils;
import org.apache.commons.io.IOUtils;
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

    public abstract String getCertificate();

    public abstract String getKey();

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

    public String getCertificateText() {
        if (getCertificateType().equals(WEB)) {
            return getCertificateManager().getWebCertificate();
        }
        return getCertificateManager().getCommunicationsCertificate();
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
            if (getCertificateType().equals(WEB)) {
                if (StringUtils.isEmpty(key)) {
                    mgr.setWebCertificate(certificate);
                } else {
                    mgr.setWebCertificate(certificate, key);
                }
            } else {
                if (StringUtils.isEmpty(key)) {
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
