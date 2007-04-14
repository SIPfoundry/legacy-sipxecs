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

import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;

public class SpecialAutoAttendantMode extends XmlFile {
    private final AutoAttendant m_attendant;
    private final boolean m_enabled;

    public SpecialAutoAttendantMode(boolean enabled, AutoAttendant attendant) {
        m_enabled = enabled;
        m_attendant = attendant;
    }

    public Document getDocument() {
        Document document = FACTORY.createDocument();
        Element op = document.addElement("organizationprefs");
        op.addElement("specialoperation").setText(Boolean.toString(m_enabled));
        Element aa = op.addElement("autoattendant");
        aa.setText(m_attendant.getSystemName());
        return document;
    }

    public ConfigFileType getType() {
        return ConfigFileType.ORGANIZATION_PREFS;
    }
}
