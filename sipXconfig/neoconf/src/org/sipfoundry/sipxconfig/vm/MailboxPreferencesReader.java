/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import java.util.List;

import org.apache.commons.codec.binary.Base64;
import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.Node;

public class MailboxPreferencesReader extends XmlReaderImpl<MailboxPreferences> {
    private static final String YES = "yes";

    @Override
    public MailboxPreferences readObject(Document doc) {
        MailboxPreferences prefs = new MailboxPreferences();
        Node root = doc.getRootElement();
        String greetingId = root.valueOf("activegreeting");
        MailboxPreferences.ActiveGreeting greeting = MailboxPreferences.ActiveGreeting.valueOfById(greetingId);
        prefs.setActiveGreeting(greeting);
        Element imap = (Element) root.selectSingleNode("imapserver");
        if (imap != null) {
            prefs.setEmailServerHost(imap.attributeValue("host"));
            prefs.setEmailServerPort(imap.attributeValue("port"));
            prefs.setEmailServerUseTLS(YES.equals(imap.attributeValue("UseTLS")));
        }
        List<Element> contacts = root.selectNodes("notification/contact");
        prefs.setEmailAddress(getEmailAddress(0, contacts));
        prefs.setAlternateEmailAddress(getEmailAddress(1, contacts));
        prefs.setAttachVoicemailToEmail(getAttachVoicemail(0, contacts));
        prefs.setAttachVoicemailToAlternateEmail(getAttachVoicemail(1, contacts));
        String pwd = getPassword(0, contacts);
        String decodedPwd = pwd != null ? new String(Base64.decodeBase64(pwd.getBytes())) : null;
        prefs.setEmailPassword(decodedPwd);
        boolean synchronize = getSynchronize(0, contacts);
        boolean attachFirstVoicemailToEmail = getAttachVoicemail(0, contacts);
        prefs.setSynchronizeWithEmailServer(synchronize);
        if (synchronize) {
            prefs.setVoicemailProperties(prefs.SYNCHRONIZE_WITH_EMAIL_SERVER);
        } else if (attachFirstVoicemailToEmail) {
            prefs.setVoicemailProperties(prefs.ATTACH_VOICEMAIL);
        } else {
            prefs.setVoicemailProperties(prefs.DO_NOT_ATTACH_VOICEMAIL);
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
        return YES.equals(sAttachVm);
    }

    private String getPassword(int index, List<Element> contacts) {
        if (contacts.size() <= index) {
            return null;
        }
        String password = contacts.get(index).attributeValue("password");
        return password;
    }

    private boolean getSynchronize(int index, List<Element> contacts) {
        if (contacts.size() <= index) {
            return false;
        }
        String synchronize = contacts.get(index).attributeValue("synchronize");
        return YES.equals(synchronize);
    }
}
