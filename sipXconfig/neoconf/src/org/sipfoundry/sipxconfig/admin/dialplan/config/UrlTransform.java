/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import org.dom4j.Element;

/**
 * UrilTransfomr
 */
public class UrlTransform extends Transform {
    private String m_url;

    public String getUrl() {
        return m_url;
    }

    public void setUrl(String url) {
        m_url = url;
    }

    protected void addChildren(Element transform) {
        if (null != m_url) {
            Element url = transform.addElement("url");
            url.setText(m_url);
        }
    }
}
