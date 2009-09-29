/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.monitoring;

public class MonitoringBean {

    private String m_targetTitle;
    private String m_imageLink;
    private String m_htmlDetails;

    public MonitoringBean(String title, String link, String details) {
        m_targetTitle = title;
        m_imageLink = link;
        m_htmlDetails = details;
    }

    public String getHtmlDetails() {
        return m_htmlDetails;
    }

    public void setHtmlDetails(String htmlDetails) {
        m_htmlDetails = htmlDetails;
    }

    public String getImageLink() {
        return m_imageLink;
    }

    public void setImageLink(String imageLink) {
        m_imageLink = imageLink;
    }

    public String getTargetTitle() {
        return m_targetTitle;
    }

    public void setTargetTitle(String targetTitle) {
        m_targetTitle = targetTitle;
    }

}
