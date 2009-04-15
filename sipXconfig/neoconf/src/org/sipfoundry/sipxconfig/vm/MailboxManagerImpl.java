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

import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.InputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.commons.io.filefilter.SuffixFileFilter;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.UserDeleteListener;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.service.SipxMediaService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendant;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendantWriter;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class MailboxManagerImpl extends HibernateDaoSupport implements MailboxManager {
    private static final String MESSAGE_SUFFIX = "-00.xml";
    private static final FilenameFilter MESSAGE_FILES = new SuffixFileFilter(MESSAGE_SUFFIX);
    private static final Log LOG = LogFactory.getLog(MailboxManagerImpl.class);
    private File m_mailstoreDirectory;
    private MailboxPreferencesReader m_mailboxPreferencesReader;
    private MailboxPreferencesWriter m_mailboxPreferencesWriter;
    private DistributionListsReader m_distributionListsReader;
    private DistributionListsWriter m_distributionListsWriter;
    private PersonalAttendantWriter m_personalAttendantWriter;
    private CoreContext m_coreContext;
    private SipxServiceManager m_sipxServiceManager;
    private LocationsManager m_locationsManager;

    // should only be accessed via accessor method
    private String m_mediaServerCgiUrl;

    public boolean isEnabled() {
        return m_mailstoreDirectory != null && m_mailstoreDirectory.exists();
    }

    public DistributionList[] loadDistributionLists(Mailbox mailbox) {
        File file = mailbox.getDistributionListsFile();
        DistributionList[] lists = m_distributionListsReader.readObject(file);
        if (lists == null) {
            lists = DistributionList.createBlankList();
        }
        return lists;
    }

    public void saveDistributionLists(Mailbox mailbox, DistributionList[] lists) {
        Collection<String> aliases = DistributionList.getUniqueExtensions(lists);
        m_coreContext.checkForValidExtensions(aliases, PermissionName.VOICEMAIL);
        File file = mailbox.getDistributionListsFile();
        m_distributionListsWriter.writeObject(lists, file);
    }

    public List<Voicemail> getVoicemail(Mailbox mbox, String folder) {
        checkMailstoreDirectory();
        File vmdir = new File(mbox.getUserDirectory(), folder);
        String[] wavs = vmdir.list(MESSAGE_FILES);
        if (wavs == null) {
            return Collections.emptyList();
        }
        Arrays.sort(wavs);
        List<Voicemail> vms = new ArrayList(wavs.length);
        for (String wav : wavs) {
            String basename = basename(wav);
            vms.add(new Voicemail(m_mailstoreDirectory, mbox.getUserId(), folder, basename));
        }
        return vms;
    }

    /**
     * tell mediaserver cgi to mark voicemail as heard by using these parameters action =
     * updatestatus mailbox = userid category = inbox messageidlist = space delimited message ids
     */
    public void markRead(Mailbox mailbox, Voicemail voicemail) {
        String request = String.format(
                "action=updatestatus&mailbox=%s&category=%s&messageidlist=%s", mailbox
                        .getUserId(), voicemail.getFolderId(), voicemail.getMessageId());
        // triggers NOTIFY (iff folder is inbox, bug in mediaserver?)
        mediaserverCgiRequest(request);
    }

    public void move(Mailbox mailbox, Voicemail voicemail, String destinationFolderId) {
        File destination = new File(mailbox.getUserDirectory(), destinationFolderId);
        for (File f : voicemail.getAllFiles()) {
            f.renameTo(new File(destination, f.getName()));
        }

        triggerSipNotify(mailbox);
    }

    public void delete(Mailbox mailbox, Voicemail voicemail) {
        for (File f : voicemail.getAllFiles()) {
            f.delete();
        }

        triggerSipNotify(mailbox);
    }

    private void triggerSipNotify(Mailbox mailbox) {
        // reversed engineered this string from using sipx 3.6 system.
        String request = String.format("action=updatestatus&from=gateway&category=inbox&"
                + "mailbox=%s&messageidlist=-2", mailbox.getUserId());
        mediaserverCgiRequest(request);
    }

    public void mediaserverCgiRequest(String cgiRequest) {
        String errMsg = "Cannot contact media server to update voicemail status";
        if (StringUtils.isBlank(getMediaServerCgiUrl())) {
            return;
        }
        String sUpdate = getMediaServerCgiUrl() + '?' + cgiRequest;
        InputStream updateResponse = null;
        try {
            LOG.info(sUpdate);
            updateResponse = new URL(sUpdate).openStream();
            IOUtils.readLines(updateResponse);
        } catch (MalformedURLException e) {
            throw new RuntimeException(errMsg, e);
        } catch (IOException e) {
            // not a fatal exception either. (unfort,. likely if mediaserver cert. isn't valid
            // for multitude of reasons including reverse DNS not resolving)
            LOG.warn(errMsg, e);
        } finally {
            IOUtils.closeQuietly(updateResponse);
        }
    }

    /**
     * Because in HA systems, admin may change mailstore directory, validate it
     */
    void checkMailstoreDirectory() {
        if (m_mailstoreDirectory == null) {
            throw new MailstoreMisconfigured(null);
        }
        if (!m_mailstoreDirectory.exists()) {
            throw new MailstoreMisconfigured(m_mailstoreDirectory.getAbsolutePath());
        }
    }

    static class MailstoreMisconfigured extends UserException {
        MailstoreMisconfigured() {
            super("Mailstore directory configuration setting is missing.");
        }

        MailstoreMisconfigured(String dir) {
            super(String.format("Mailstore directory does not exist '%s'", dir));
        }

        MailstoreMisconfigured(String message, IOException cause) {
            super(message, cause);
        }
    }

    /**
     * extract file name w/o ext.
     */
    static String basename(String filename) {
        int suffix = filename.lastIndexOf(MESSAGE_SUFFIX);
        return suffix >= 0 ? filename.substring(0, suffix) : filename;
    }

    public String getMailstoreDirectory() {
        return m_mailstoreDirectory.getPath();
    }

    public void setMailstoreDirectory(String mailstoreDirectory) {
        m_mailstoreDirectory = new File(mailstoreDirectory);
    }

    public Mailbox getMailbox(String userId) {
        return new Mailbox(m_mailstoreDirectory, userId);
    }

    public void deleteMailbox(String userId) {
        Mailbox mailbox = getMailbox(userId);
        mailbox.deleteUserDirectory();
    }

    public void saveMailboxPreferences(Mailbox mailbox, MailboxPreferences preferences) {
        File file = mailbox.getVoicemailPreferencesFile();
        m_mailboxPreferencesWriter.writeObject(preferences, file);
    }

    public MailboxPreferences loadMailboxPreferences(Mailbox mailbox) {
        File prefsFile = mailbox.getVoicemailPreferencesFile();
        MailboxPreferences preferences = m_mailboxPreferencesReader.readObject(prefsFile);
        if (preferences == null) {
            preferences = new MailboxPreferences();
        }
        return preferences;
    }

    public static class YesNo {
        public String encode(Object o) {
            return Boolean.TRUE.equals(o) ? "yes" : "no";
        }
    }

    /**
     * Returns the media server cgi url string for the primary server using the port
     * defined in the sipx media service.
     * @return
     */
    protected String getMediaServerCgiUrl() {
        if (m_mediaServerCgiUrl == null) {
            SipxMediaService mediaService = (SipxMediaService) m_sipxServiceManager
                    .getServiceByBeanId(SipxMediaService.BEAN_ID);
            Location primaryLocation = m_locationsManager.getPrimaryLocation();

            StringBuffer mediaServerCgiUrlBuffer = new StringBuffer();
            mediaServerCgiUrlBuffer.append("https://");
            mediaServerCgiUrlBuffer.append(primaryLocation.getFqdn());

            int httpsPort = mediaService.getVoicemailHttpsPort();
            if (httpsPort != 0) {
                mediaServerCgiUrlBuffer.append(':');
                mediaServerCgiUrlBuffer.append(httpsPort);
            }

            mediaServerCgiUrlBuffer.append("/cgi-bin/voicemail/mediaserver.cgi");
            m_mediaServerCgiUrl = mediaServerCgiUrlBuffer.toString();
        }

        return m_mediaServerCgiUrl;
    }

    public void setMailboxPreferencesReader(MailboxPreferencesReader mailboxReader) {
        m_mailboxPreferencesReader = mailboxReader;
    }

    public void setMailboxPreferencesWriter(MailboxPreferencesWriter mailboxWriter) {
        m_mailboxPreferencesWriter = mailboxWriter;
    }

    public void setDistributionListsReader(DistributionListsReader distributionListsReader) {
        m_distributionListsReader = distributionListsReader;
    }

    public void setDistributionListsWriter(DistributionListsWriter distributionListsWriter) {
        m_distributionListsWriter = distributionListsWriter;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setPersonalAttendantWriter(PersonalAttendantWriter personalAttendantWriter) {
        m_personalAttendantWriter = personalAttendantWriter;
    }

    public PersonalAttendant loadPersonalAttendantForUser(User user) {
        PersonalAttendant pa = findPersonalAttendant(user);
        if (pa == null) {
            pa = new PersonalAttendant();
            pa.setUser(user);
            getHibernateTemplate().save(pa);
        }
        return pa;
    }

    public void removePersonalAttendantForUser(User user) {
        PersonalAttendant pa = findPersonalAttendant(user);
        if (pa != null) {
            getHibernateTemplate().delete(pa);
        }
    }

    public void storePersonalAttendant(PersonalAttendant pa) {
        getHibernateTemplate().saveOrUpdate(pa);
        writePersonalAttendant(pa);
    }

    public void clearPersonalAttendants() {
        List<PersonalAttendant> allPersonalAttendants = getHibernateTemplate().loadAll(
                PersonalAttendant.class);
        getHibernateTemplate().deleteAll(allPersonalAttendants);
    }

    public void writeAllPersonalAttendants() {
        List<PersonalAttendant> all = getHibernateTemplate().loadAll(PersonalAttendant.class);
        for (PersonalAttendant pa : all) {
            writePersonalAttendant(pa);
        }
    }

    private void writePersonalAttendant(PersonalAttendant pa) {
        Mailbox mailbox = getMailbox(pa.getUser().getUserName());
        m_personalAttendantWriter.write(mailbox, pa);
    }

    private PersonalAttendant findPersonalAttendant(User user) {
        Collection pas = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "personalAttendantForUser", "user", user);
        return (PersonalAttendant) DataAccessUtils.singleResult(pas);
    }

    public UserDeleteListener createUserDeleteListener() {
        return new OnUserDelete();
    }

    private class OnUserDelete extends UserDeleteListener {
        protected void onUserDelete(User user) {
            removePersonalAttendantForUser(user);
        }
    }
}
