/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.voicemail.mailbox;

import java.io.File;
import java.util.List;

import org.sipfoundry.commons.userdb.User;

public interface MailboxManager {

    public static final String VOICEMAIL_SUBJECT = "Voice Message ";
    public static final String CONFERENCE_CALL = "Conference Call ";

    MailboxDetails getMailboxDetails(String username);

    List<VmMessage> getMessages(String username, Folder folder);

    TempMessage createTempMessage(String username, String fromUri, boolean addAudio);

    TempMessage createTempMessage(String username, String fromUri, String extension, boolean addAudio);

    void deleteTempMessage(TempMessage message);

    void store(User destUser, Folder folder, TempMessage message, String subject);

    void storeInInbox(User destUser, TempMessage message);

    void copyMessage(User destUser, TempMessage message);

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

    boolean changePin(User user, String newPin);

    boolean manageSpecialMode(User user, boolean enable);

    void markMessageUnheard(User user, String messageId);

    void markMessageHeard(User user, String messageId);

    boolean isMessageUnHeard(User user, String messageId);

    void deleteMessage(User user, String messageId);

    void updateMessageSubject(User user, String messageId, String subject);

    void moveMessageToFolder(User user, String messageId, String destination);

    void deleteMailbox(String username);

    void renameMailbox(User user, String oldUser);

}
