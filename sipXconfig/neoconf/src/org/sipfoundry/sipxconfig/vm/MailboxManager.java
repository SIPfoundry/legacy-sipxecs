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

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendant;

public interface MailboxManager {

    boolean isEnabled();

    List<Voicemail> getVoicemail(Mailbox mailbox, String folder);

    String getMailstoreDirectory();

    String getStdpromptDirectory();

    Mailbox getMailbox(String userId);

    void deleteMailbox(String userId);

    void renameMailbox(String oldUserId, String newUserId);

    void saveDistributionLists(Mailbox mailbox, DistributionList[] lists);

    DistributionList[] loadDistributionLists(Mailbox mailbox);

    void markRead(Mailbox mailbox, Voicemail voicemail);

    void move(Mailbox mailbox, Voicemail voicemail, String destinationFolderId);

    void delete(Mailbox mailbox, Voicemail voicemail);

    PersonalAttendant loadPersonalAttendantForUser(User user);

    void removePersonalAttendantForUser(User user);

    void storePersonalAttendant(PersonalAttendant pa);

    void clearPersonalAttendants();

    void updatePersonalAttendantForUser(User user, String operatorValue);
}
