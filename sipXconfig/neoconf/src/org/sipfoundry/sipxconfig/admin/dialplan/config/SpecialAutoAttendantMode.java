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
    private AutoAttendant m_attendant;
    private boolean m_enabled;

    public void generate(boolean enabled, AutoAttendant attendant) {
        m_attendant = attendant;
        m_enabled = enabled;
    }

    public Document getDocument() {
        Document document = FACTORY.createDocument();
        Element op = document.addElement("organizationprefs");
        op.addElement("specialoperation").setText(Boolean.toString(m_enabled));
        Element aa = op.addElement("autoattendant");
        aa.setText(m_attendant.getSystemName());
        return document;
    }
}
