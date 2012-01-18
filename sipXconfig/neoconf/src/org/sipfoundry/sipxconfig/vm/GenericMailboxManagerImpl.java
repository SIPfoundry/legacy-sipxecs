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
//            getManager() = (MailboxManager) local;
//            remote.setActive(false);
        }
        ((AbstractMailboxManager) m_manager).init();
    }

    MailboxManager getManager() {
        if (m_manager == null) {
            init();
        }
        return m_manager;
    }

    @Override
    public boolean isEnabled() {
        // messed up -- Douglas
        return false;
    }

    @Override
    public boolean isSystemCpui() {
        return getManager().isSystemCpui();
    }

    @Override
    public List<Voicemail> getVoicemail(String userId, String folder) {
        return getManager().getVoicemail(userId, folder);
    }

    @Override
    public Voicemail getVoicemail(String userId, String folder, String messageId) {
        return getManager().getVoicemail(userId, folder, messageId);
    }

    @Override
    public String getStdpromptDirectory() {
        return getManager().getStdpromptDirectory();
    }

    @Override
    public void deleteMailbox(String userId) {
        getManager().deleteMailbox(userId);
    }

    @Override
    public void renameMailbox(String oldUserId, String newUserId) {
        getManager().renameMailbox(oldUserId, newUserId);
    }

    @Override
    public void saveDistributionLists(User user, DistributionList[] lists) {
        m_manager.saveDistributionLists(user, lists);
    }

    @Override
    public DistributionList[] loadDistributionLists(User user) {
        return m_manager.loadDistributionLists(user);
    }

    @Override
    public void markRead(String userId, String messageId) {
        getManager().markRead(userId, messageId);
    }

    @Override
    public void move(String userId, Voicemail voicemail, String destinationFolderId) {
        getManager().move(userId, voicemail, destinationFolderId);
    }

    @Override
    public void delete(String userId, Voicemail voicemail) {
        getManager().delete(userId, voicemail);
    }

    @Override
    public void save(Voicemail voicemail) {
        getManager().save(voicemail);
    }

    @Override
    public String getMediaFileURL(String userId, String folder, String messageId) {
        return getManager().getMediaFileURL(userId, folder, messageId);
    }

    @Override
    public PersonalAttendant loadPersonalAttendantForUser(User user) {
        return getManager().loadPersonalAttendantForUser(user);
    }

    @Override
    public void removePersonalAttendantForUser(User user) {
        getManager().removePersonalAttendantForUser(user);
    }

    @Override
    public void storePersonalAttendant(PersonalAttendant pa) {
        getManager().storePersonalAttendant(pa);
    }


    public void clearPersonalAttendants() {
        getManager().clearPersonalAttendants();
    }

    @Override
    public List<String> getFolderIds() {
        return getManager().getFolderIds();
    }

    @Override
    public boolean performBackup(File workingDir) {
        return getManager().performBackup(workingDir);
    }

    @Override
    public void performRestore(BackupBean archive, boolean verify, boolean noRestart) {
        getManager().performRestore(archive, verify, noRestart);
    }

    @Override
    public String getMailboxRestoreLog() {
        return getManager().getMailboxRestoreLog();
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

    @Override
    public PersonalAttendant getPersonalAttendantForUser(User user) {
        return m_manager.getPersonalAttendantForUser(user);
    }
}
