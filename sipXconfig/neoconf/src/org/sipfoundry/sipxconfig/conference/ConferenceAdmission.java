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

import java.io.File;
import java.io.IOException;
import java.util.List;

import org.apache.commons.collections.Closure;
import org.apache.commons.collections.CollectionUtils;
import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigFileType;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;

public class ConferenceAdmission extends XmlFile {

    private String m_configDirectory;

    private Document m_document;

    public Document getDocument() {
        return m_document;
    }

    public void generate(List conferences) {
        m_document = FACTORY.createDocument();
        Element parent = m_document.addElement("conferences");
        CollectionUtils.forAllDo(conferences, new BuildConferenceElem(parent));
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
            Element confElem = m_parent.addElement("conference");
            confElem.addElement("id").setText(conference.getName());
            confElem.addElement("extension").setText(conference.getExtension());
            confElem.addElement("value").setText(conference.getRemoteAdmitSecret());
            confElem.addElement("organizer-access-code").setText(
                    conference.getOrganizerAccessCode());
            confElem.addElement("participant-access-code").setText(
                    conference.getParticipantAccessCode());
            confElem.addElement("conference-uri").setText(conference.getUri());
        }
    }

    /**
     * Writes to file in a specified directory
     * 
     * @throws IOException
     */
    public void writeToFile() throws IOException {
        File parent = new File(m_configDirectory);
        writeToFile(parent, getType().getName());
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    public ConfigFileType getType() {
        return ConfigFileType.CONFERENCES;
    }
}
