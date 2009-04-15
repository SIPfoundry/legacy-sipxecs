/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.common;

import org.apache.hivemind.Messages;
import org.apache.tapestry.IPage;
import static org.sipfoundry.sipxconfig.components.LocalizationUtils.localizeString;

public class BreadCrumb {

    private IPage m_directLink;
    private String m_pageLink;
    private String m_crumbName;
    private boolean m_usePageLinks;

    public BreadCrumb(IPage directLink, String crumbName) {
        m_directLink = directLink;
        m_crumbName = crumbName;
        m_usePageLinks = false;
    }

    public BreadCrumb(String pageLink, String crumbName) {
        m_pageLink = pageLink;
        m_crumbName = crumbName;
        m_usePageLinks = true;
    }

    public BreadCrumb(IPage directLink, String crumbName, Messages messages) {
        m_directLink = directLink;
        m_crumbName = localizeString(messages, crumbName);
        m_usePageLinks = false;
    }

    public BreadCrumb(String pageLink, String crumbName, Messages messages) {
        m_pageLink = pageLink;
        m_crumbName = localizeString(messages, crumbName);
        m_usePageLinks = true;
    }

    public IPage getDirectLink() {
        return m_directLink;
    }

    public String getPageLink() {
        return m_pageLink;
    }

    public String getCrumbName() {
        return m_crumbName;
    }

    public boolean isUsePageLinks() {
        return m_usePageLinks;
    }
}
