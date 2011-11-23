/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.voicemail.mailbox;

import java.io.File;
import java.util.List;

import org.sipfoundry.commons.userdb.User;

public interface MailboxManager {

    MailboxDetails getMailboxDetails(String username);

    List<VmMessage> getMessages(String username, Folder folder);

    TempMessage createTempMessage(String username, String fromUri, boolean addAudio);

    void deleteTempMessage(TempMessage message);

    void storeInInbox(User destUser, TempMessage message);

    void copyMessage(User destUser, TempMessage message);

    PersonalAttendant getPersonalAttendant(String username);

    VmMessage getVmMessage(String username, String messageId, boolean loadAudio);

    VmMessage getVmMessage(String username, Folder folder, String messageId, boolean loadAudio);

    void markMessageHeard(User user, VmMessage message);

    void saveMessage(User user, VmMessage message);

    void deleteMessage(User user, VmMessage message);

    void removeDeletedMessages(String username);

    void forwardMessage(User destUser, VmMessage message, TempMessage comments);

    File getRecordedName(String username);

    void saveRecordedName(TempMessage message);

    void saveCustomAutoattendantPrompt(TempMessage message);

    void saveGreetingFile(GreetingType type, TempMessage message);

    void saveActiveGreeting(User user, GreetingType type);

    String getGreetingPath(User user, GreetingType type);

    Distributions getDistributions(User user);

    MailboxPreferences getMailboxPreferences(User user);

    boolean changePin(User user, String newPin);

    boolean manageSpecialMode(User user, boolean enable);

    void markMessageUnheard(User user, String messageId);

    void markMessageHeard(User user, String messageId);

    boolean isMessageUnHeard(User user, String messageId);

    void deleteMessage(User user, String messageId);

    void updateMessageSubject(User user, String messageId, String subject);

    void moveMessageToFolder(User user, String messageId, String destination);

    void savePersonalAttendant(User user, String content);

    void saveMailboxPrefs(User user, String content);

    void deleteMailbox(User user);

    void renameMailbox(User user, String oldUser);

    String getDistributionListContent(User user);

    void saveDistributionList(User user, String content);
}
