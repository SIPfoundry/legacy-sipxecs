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

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.cert.CertificateManager;
import org.sipfoundry.sipxconfig.cert.CertificateRequestGenerator;
import org.sipfoundry.sipxconfig.cert.CertificateSettings;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.setting.Setting;

public abstract class ManageCertificates extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "admin/ManageCertificates";
    public static final String GENERATE_CSR_TAB = "generate";
    public static final String IMPORT_WEB_CERT_TAB = "import";
    public static final String CERT_AUTH_TAB = "authorities";

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

    public Setting getCsrSettings() {
        return getSettings().getSettings().getSetting("csr");
    }

    public abstract void setSettings(CertificateSettings settings);

    public abstract CertificateSettings getSettings();

    public abstract String getCsr();

    public abstract void setCsr(String csr);

    public void pageBeginRender(PageEvent event_) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        CertificateSettings settings = getSettings();
        if (settings == null) {
            settings = getCertificateManager().getSettings();
            setSettings(settings);
        }
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

        String web = mgr.getWebCertificate();
        String key = mgr.getWebPrivateKey();
        Location primary = getLocationsManager().getPrimaryLocation();
        String domain = Domain.getDomain().getName();
        CertificateRequestGenerator csr = new CertificateRequestGenerator(domain, primary.getFqdn());
        settings.updateCertificateDetails(csr);
        setCsr(csr.getCertificateRequestText(web, key));

        validator.recordSuccess(getMessages().getMessage("msg.generateSuccess"));
    }
}
