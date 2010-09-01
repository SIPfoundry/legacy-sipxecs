/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
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
