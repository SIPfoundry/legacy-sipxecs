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

import org.sipfoundry.sipxconfig.backup.BackupBean;
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

    void saveDistributionLists(Integer userId, DistributionList[] lists);

    DistributionList[] loadDistributionLists(User users);

    void markRead(String userId, String messageId);

    void move(String userId, Voicemail voicemail, String destinationFolderId);

    void delete(String userId, Voicemail voicemail);

    void save(Voicemail voicemail);

    String getMediaFileURL(String userId, String folder, String messageId);

    /**
     * Load a PA for a user, create it if it does not exist
     * @param user
     * @return
     */
    PersonalAttendant loadPersonalAttendantForUser(User user);
    /**
     * Load a PA for a user, return null if it does not exist
     * @param user
     * @return
     */
    PersonalAttendant getPersonalAttendantForUser(User user);

    void removePersonalAttendantForUser(User user);

    void storePersonalAttendant(PersonalAttendant pa);

    void clearPersonalAttendants();

    List<String> getFolderIds();

    boolean performBackup(File workingDir);

    void performRestore(BackupBean archive, boolean noRestart);

    String getMailboxRestoreLog();

}
