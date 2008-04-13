/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import java.util.List;

import org.apache.commons.collections.Closure;
import org.apache.commons.collections.CollectionUtils;
import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.io.OutputFormat;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigFileType;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;

public class ConferenceAdmission extends XmlFile {
    private static final String NAME = "name";

    private Document m_document;

    public Document getDocument() {
        return m_document;
    }

    public void generate(List conferences) {
        m_document = FACTORY.createDocument();
        Element context = m_document.addElement("context");
        context.addAttribute(NAME, "default");
        CollectionUtils.forAllDo(conferences, new BuildConferenceElem(context));
    }

    private static class BuildConferenceElem implements Closure {
        private final Element m_parent;

        public BuildConferenceElem(Element parent) {
            m_parent = parent;
        }

        public void execute(Object input) {
            Conference conference = (Conference) input;
            if (!conference.isEnabled()) {
                return;
            }

            // Application data: name@profile+pin
            String applicationData = conference.getName();
            applicationData = applicationData + "@" + conference.getExtension();
            String accessCode = conference.getParticipantAccessCode();
            if (accessCode != null && accessCode.length() > 0) {
                applicationData = applicationData + "+" + accessCode;
            }

            m_parent.addElement("extension").addAttribute(NAME, conference.getExtension()).addElement(
                    "condition").addAttribute("field", "destination_number").addAttribute("expression",
                    "^" + conference.getName() + "$").addElement("action").addAttribute("application",
                    "conference").addAttribute("data", applicationData);
        }
    }

    @Override
    public OutputFormat createFormat() {
        OutputFormat format = OutputFormat.createPrettyPrint();
        format.setOmitEncoding(true);
        format.setSuppressDeclaration(true);
        return format;
    }

    public ConfigFileType getType() {
        return ConfigFileType.CONFERENCE_ADDMINSION;
    }
}
