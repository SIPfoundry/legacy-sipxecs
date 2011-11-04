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

import java.io.File;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.BackupBean;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendant;

public interface MailboxManager {

    boolean isEnabled();

    boolean isSystemCpui();

    List<Voicemail> getVoicemail(String userId, String folder);

    Voicemail getVoicemail(String userId, String folder, String messageId);

    String getStdpromptDirectory();

    void deleteMailbox(String userId);

    void renameMailbox(String oldUserId, String newUserId);

    void saveDistributionLists(String userId, DistributionList[] lists);

    DistributionList[] loadDistributionLists(String userId);

    void markRead(String userId, String messageId);

    void move(String userId, Voicemail voicemail, String destinationFolderId);

    void delete(String userId, Voicemail voicemail);

    void save(Voicemail voicemail);

    String getMediaFileURL(String userId, String folder, String messageId);

    PersonalAttendant loadPersonalAttendantForUser(User user);

    void removePersonalAttendantForUser(User user);

    void storePersonalAttendant(PersonalAttendant pa);

    void storePersonalAttendant(PersonalAttendant pa, boolean writeFile);

    void clearPersonalAttendants();

    void updatePersonalAttendantForUser(User user, String operatorValue);

    void writePreferencesFile(User user);

    List<String> getFolderIds();

    boolean performBackup(File workingDir);

    void performRestore(BackupBean archive, boolean validate, boolean noRestart);

    String getMailboxRestoreLog();

}
