/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import java.io.File;
import java.util.List;

import org.sipfoundry.sipxconfig.backup.BackupBean;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendant;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;

public class GenericMailboxManagerImpl implements MailboxManager, BeanFactoryAware, DaoEventListener {
    private MailboxManager m_manager;
    private BeanFactory m_beanFactory;

    // DISABLED - Need to clarify w/George if this is still nec. causes great issues trying to execute
    // code when spring is still in the process of initializing.
    public void init() {

        // TODO: Why do we distinguish local and remote,  everything should be considered remote
        // IMHO -- Douglas

        //Location voicemailLocation = m_featureManager.getLocationForEnabledFeature(Ivr.FEATURE);
        AbstractMailboxManager remote = m_beanFactory.getBean(RemoteMailboxManagerImpl.class);
        AbstractMailboxManager local = m_beanFactory.getBean(LocalMailboxManagerImpl.class);
        //
        if (true) { //voicemailLocation != null && !voicemailLocation.isPrimary()) {
            m_manager = (MailboxManager) remote;
            local.setActive(false);
//        } else {
//            m_manager = (MailboxManager) local;
//            remote.setActive(false);
        }
        ((AbstractMailboxManager) m_manager).init();
    }

    @Override
    public boolean isEnabled() {
        return m_manager.isEnabled();
    }

    @Override
    public boolean isSystemCpui() {
        return m_manager.isSystemCpui();
    }

    @Override
    public List<Voicemail> getVoicemail(String userId, String folder) {
        return m_manager.getVoicemail(userId, folder);
    }

    @Override
    public Voicemail getVoicemail(String userId, String folder, String messageId) {
        return m_manager.getVoicemail(userId, folder, messageId);
    }

    @Override
    public String getStdpromptDirectory() {
        return m_manager.getStdpromptDirectory();
    }

    @Override
    public void deleteMailbox(String userId) {
        m_manager.deleteMailbox(userId);
    }

    @Override
    public void renameMailbox(String oldUserId, String newUserId) {
        m_manager.renameMailbox(oldUserId, newUserId);
    }

    @Override
    public void saveDistributionLists(String userId, DistributionList[] lists) {
        m_manager.saveDistributionLists(userId, lists);
    }

    @Override
    public DistributionList[] loadDistributionLists(String userId) {
        return m_manager.loadDistributionLists(userId);
    }

    @Override
    public void markRead(String userId, String messageId) {
        m_manager.markRead(userId, messageId);
    }

    @Override
    public void move(String userId, Voicemail voicemail, String destinationFolderId) {
        m_manager.move(userId, voicemail, destinationFolderId);
    }

    @Override
    public void delete(String userId, Voicemail voicemail) {
        m_manager.delete(userId, voicemail);
    }

    @Override
    public void save(Voicemail voicemail) {
        m_manager.save(voicemail);
    }

    @Override
    public String getMediaFileURL(String userId, String folder, String messageId) {
        return m_manager.getMediaFileURL(userId, folder, messageId);
    }

    @Override
    public PersonalAttendant loadPersonalAttendantForUser(User user) {
        return m_manager.loadPersonalAttendantForUser(user);
    }

    @Override
    public void removePersonalAttendantForUser(User user) {
        m_manager.removePersonalAttendantForUser(user);
    }

    @Override
    public void storePersonalAttendant(PersonalAttendant pa) {
        m_manager.storePersonalAttendant(pa);
    }

    @Override
    public void storePersonalAttendant(PersonalAttendant pa, boolean writeFile) {
        m_manager.storePersonalAttendant(pa, writeFile);
    }

    @Override
    public void clearPersonalAttendants() {
        m_manager.clearPersonalAttendants();
    }

    @Override
    public void updatePersonalAttendantForUser(User user, String operatorValue) {
        m_manager.updatePersonalAttendantForUser(user, operatorValue);
    }

    @Override
    public void writePreferencesFile(User user) {
        m_manager.writePreferencesFile(user);
    }

    @Override
    public List<String> getFolderIds() {
        return m_manager.getFolderIds();
    }

    @Override
    public boolean performBackup(File workingDir) {
        return m_manager.performBackup(workingDir);
    }

    @Override
    public void performRestore(BackupBean archive, boolean verify, boolean noRestart) {
        m_manager.performRestore(archive, verify, noRestart);
    }

    @Override
    public String getMailboxRestoreLog() {
        return m_manager.getMailboxRestoreLog();
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    @Override
    public void onDelete(Object entity) {
        onEvent(entity);
    }

    @Override
    public void onSave(Object entity) {
        onEvent(entity);
    }

    private void onEvent(Object entity) {
        if (entity instanceof Location) {
            init();
        }
    }
}
