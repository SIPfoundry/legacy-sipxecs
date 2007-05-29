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

public interface MailboxManager {

    public boolean isEnabled();

    public List<Voicemail> getVoicemail(Mailbox mailbox, String folder);

    public String getMailstoreDirectory();

    public Mailbox getMailbox(String userId);

    public void deleteMailbox(String userId);

    public void saveMailboxPreferences(Mailbox mailbox, MailboxPreferences preferences);

    public MailboxPreferences loadMailboxPreferences(Mailbox mailbox);

    public void saveDistributionLists(Mailbox mailbox, DistributionList[] lists);

    public DistributionList[] loadDistributionLists(Mailbox mailbox);

    public void markRead(Mailbox mailbox, Voicemail voicemail);

    public void move(Mailbox mailbox, Voicemail voicemail, String destinationFolderId);

    public void delete(Mailbox mailbox, Voicemail voicemail);
}
