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

import org.apache.commons.lang.StringUtils;
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
    @Override
    public MailboxPreferences readObject(Document doc) {
        MailboxPreferences prefs = new MailboxPreferences();

        Node root = doc.getRootElement();
        String greetingId = root.valueOf("activegreeting");
        MailboxPreferences.ActiveGreeting greeting = MailboxPreferences.ActiveGreeting.fromId(greetingId);
        prefs.setActiveGreeting(greeting);

        List<Element> contacts = root.selectNodes("notification/contact");
        String emailAddress = getEmailAddress(0, contacts);
        if (StringUtils.isNotBlank(emailAddress)) {
            prefs.setEmailAddress(emailAddress);
            prefs.setAttachVoicemailToEmail(AttachType.YES);
            prefs.setIncludeAudioAttachment(getAttachVoicemail(0, contacts));
        }
        String alternateEmailAdress = getEmailAddress(1, contacts);
        if (StringUtils.isNotBlank(alternateEmailAdress)) {
            prefs.setAlternateEmailAddress(alternateEmailAdress);
            prefs.setVoicemailToAlternateEmailNotification(AttachType.YES);
            prefs.setIncludeAudioAttachmentAlternateEmail(getAttachVoicemail(1, contacts));
        }
        return prefs;
    }

    private String getEmailAddress(int index, List<Element> contacts) {
        if (contacts.size() <= index) {
            return null;
        }
        return contacts.get(index).getText();
    }

    private boolean getAttachVoicemail(int index, List<Element> contacts) {
        if (contacts.size() <= index) {
            return false;
        }
        String sAttachVm = contacts.get(index).attributeValue("attachments");
        return "yes".equals(sAttachVm);
    }
}
