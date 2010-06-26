/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.about;

import org.apache.tapestry.IPage;
import org.sipfoundry.sipxconfig.common.VersionInfo;

public class AboutBean {
    private VersionInfo m_versionInfo;
    private String m_title;
    private String m_name;
    private String m_details;
    private String m_copyright;
    private String m_configurationFile;

    private IPage m_aboutPage;

    public void setAboutPage(IPage aboutPage) {
        m_aboutPage = aboutPage;
    }

    public IPage getAboutPage() {
        return m_aboutPage;
    }

    public void setVersionInfo(VersionInfo versionInfo) {
        m_versionInfo = versionInfo;
    }

    public String getTitle() {
        m_title = (m_title == null) ? m_aboutPage.getMessages().getMessage("titleAbout") : m_title;
        return m_title;
    }

    public String getName() {
        m_name = (m_name == null) ? m_aboutPage.getMessages().getMessage("product.name") : m_name;
        return m_name;
    }

    public String getDetails() {
        m_details = (m_details == null) ? m_aboutPage.getMessages().
                format("product.details", m_versionInfo.getVersionDetails()) : m_details;
        return m_details;
    }

    public String getCopyright() {
        m_copyright = (m_copyright == null) ? m_aboutPage.getMessages().getMessage("product.copyright") : m_copyright;
        return m_copyright;
    }

    public void setConfigurationFile(String configurationFile) {
        m_configurationFile = configurationFile;
    }

    public String getConfigurationFile() {
        return m_configurationFile;
    }
}
