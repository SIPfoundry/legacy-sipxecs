/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.vm;

import java.util.List;

import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.Node;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.AttachType;

/**
 * This class is used only for the migration task sipXconfig should never read the preferences
 * file except for this this purpose.
 *
 * It only imports preferences compatible with sipXconfig 4.0 format.
 */
public class MailboxPreferencesReader extends XmlReaderImpl<MailboxPreferences> {
    private static final String YES = "yes";
    private static final String ATTACHMENTS_ATTR_VALUE = "attachments";

    @Override
    public MailboxPreferences readObject(Document doc) {
        MailboxPreferences prefs = new MailboxPreferences();

        Node root = doc.getRootElement();
        String greetingId = root.valueOf("activegreeting");
        MailboxPreferences.ActiveGreeting greeting = MailboxPreferences.ActiveGreeting.fromId(greetingId);
        prefs.setActiveGreeting(greeting);

        List<Element> contacts = root.selectNodes("notification/contact");
        prefs.setEmailAddress(getEmailAddress(0, contacts));
        prefs.setAlternateEmailAddress(getEmailAddress(1, contacts));

        prefs.setAttachVoicemailToEmail(getAttachVoicemail(0, contacts));

        prefs.setAttachVoicemailToAlternateEmail(getAttachVoicemailAlternate(1, contacts));

        return prefs;
    }

    private String getEmailAddress(int index, List<Element> contacts) {
        if (contacts.size() <= index) {
            return null;
        }
        return contacts.get(index).getText();
    }

    private AttachType getAttachVoicemail(int index, List<Element> contacts) {
        if (contacts.size() <= index) {
            return AttachType.NO;
        }
        String sAttachVm = contacts.get(index).attributeValue(ATTACHMENTS_ATTR_VALUE);
        return YES.equals(sAttachVm) ? AttachType.YES : AttachType.NO;
    }

    private boolean getAttachVoicemailAlternate(int index, List<Element> contacts) {
        if (contacts.size() <= index) {
            return false;
        }
        String sAttachVm = contacts.get(index).attributeValue(ATTACHMENTS_ATTR_VALUE);
        return YES.equals(sAttachVm);
    }
}
