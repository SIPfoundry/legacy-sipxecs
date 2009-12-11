/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.io.File;
import java.util.Collection;
import java.util.Set;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.request.IUploadFile;
import org.sipfoundry.sipxconfig.admin.CertificateDecorator;
import org.sipfoundry.sipxconfig.admin.CertificateManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

@ComponentClass
public abstract class CertificateAuthorities extends BaseComponent {

    public abstract IUploadFile getUploadFile();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract CertificateDecorator getCurrentRow();

    @Persist(value = "client")
    public abstract CertificateDecorator getRowToShow();

    public abstract void setRowToShow(CertificateDecorator row);

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract CertificateSqueezeAdapter getCertificateConverter();

    @InjectObject(value = "spring:certificateManager")
    public abstract CertificateManager getCertificateManager();

    @InitialValue(value = "false")
    public abstract boolean isShowCertificate();

    public abstract void setShowCertificate(boolean showCertificate);

    public abstract String getCertificateText();

    public abstract void setCertificateText(String certificateText);

    public abstract String getSavedCertificateText();

    public abstract void setSavedCertificateText(String savedCertificateText);

    public boolean isShowDescription() {
        return getRowToShow() == null ? false : getRowToShow().equals(getCurrentRow());
    }

    public void clickRow() {
        if (getRowToShow() != null && getRowToShow().equals(getCurrentRow())) {
            setRowToShow(null);
        } else {
            setRowToShow(getCurrentRow());
            runShowDescription();
        }
    }

    public CertificatesTableModel getCertificatesModel() {
        CertificatesTableModel tableModel = new CertificatesTableModel();
        Set<CertificateDecorator> list = getCertificateManager().listCertificates();
        tableModel.setCertificates(list);
        return tableModel;
    }

    public void importCA() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
        IUploadFile uploadFile = getUploadFile();

        if (uploadFile == null) {
            getValidator().record(new UserException("&error.certificate"), getMessages());
            return;
        }
        String caFileName = uploadFile.getFileName();
        File tmpCAFile = getCertificateManager().getCATmpFile(caFileName);
        getUploadFile().write(tmpCAFile);
        boolean valid = getCertificateManager().validateCertificate(tmpCAFile);
        if (!valid) {
            getValidator().record(new UserException("&error.valid", caFileName), getMessages());
            return;
        }
        setShowCertificate(true);
        String certificateText = getCertificateManager().showCertificate(tmpCAFile);
        setCertificateText(certificateText);
    }

    public void keep() {
        setShowCertificate(false);
        getCertificateManager().copyCRTAuthority();
        getCertificateManager().deleteCRTAuthorityTmpDirectory();
        getCertificateManager().rehashCertificates();
        getCertificateManager().generateKeyStores();
    }

    public void delete() {
        setShowCertificate(false);
        getCertificateManager().deleteCRTAuthorityTmpDirectory();
    }

    public void runShowDescription() {
        File caFile = getCertificateManager().getCAFile(getCurrentRow().getFileName());
        String savedCertificateText = getCertificateManager().showCertificate(caFile);
        setSavedCertificateText(savedCertificateText);
    }

    public void deleteCertificates() {
        Collection<CertificateDecorator> listCert = getSelections().getAllSelected();
        getCertificateManager().deleteCAs(listCert);
        getCertificateManager().generateKeyStores();
    }
}
