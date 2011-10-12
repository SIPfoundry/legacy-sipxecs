/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.site.about;

import java.io.InputStream;

import org.apache.commons.io.IOUtils;
import org.apache.tapestry.IPage;
import org.sipfoundry.sipxconfig.common.VersionInfo;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.core.io.Resource;

public class AboutBean {
    private VersionInfo m_versionInfo;
    private String m_title;
    private String m_name;
    private String m_details;
    private String m_copyright;
    private String m_text;
    private String m_configurationFile;
    private Resource m_template;

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

    public String getText() {
        m_text = (m_text == null) ? m_aboutPage.getMessages().getMessage("product.license") : m_text;
        return m_text;
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

    @Required
    public void setTemplate(Resource template) {
        m_template = template;
    }

    public String getLicenseText() {
        InputStream in = null;
        try {
            in = m_template.getInputStream();
            return IOUtils.toString(in);
        } catch (Exception ex) {
            return "No license text available";
        } finally {
            IOUtils.closeQuietly(in);
        }
    }
}
