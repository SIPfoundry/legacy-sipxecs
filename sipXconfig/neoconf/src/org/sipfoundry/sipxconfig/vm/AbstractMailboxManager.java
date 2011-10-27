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

import static org.springframework.dao.support.DataAccessUtils.singleResult;

import java.io.File;
import java.util.Arrays;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.service.SipxIvrService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.VoicemailTuiType;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendantManager;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendantWriter;

public abstract class AbstractMailboxManager extends PersonalAttendantManager implements DaoEventListener {
    protected static final String PATH_MAILBOX = "/mailbox/";
    protected static final String PATH_MESSAGE = "/message/";
    private File m_mailstoreDirectory;
    private MailboxPreferencesWriter m_mailboxPreferencesWriter;
    private File m_stdpromptDirectory;
    private CoreContext m_coreContext;
    private SipxServiceManager m_sipxServiceManager;
    private LocationsManager m_locationsManager;
    private DistributionListsWriter m_distributionListsWriter;
    private DistributionListsReader m_distributionListsReader;
    private PersonalAttendantWriter m_personalAttendantWriter;
    private String m_host;
    private String m_port;
    private boolean m_active;

    public boolean isSystemCpui() {
        SipxIvrService ivrService = (SipxIvrService) m_sipxServiceManager.getServiceByBeanId(SipxIvrService.BEAN_ID);
        return ivrService.getDefaultTui().equals(VoicemailTuiType.CALLPILOT.getValue());
    }

    public String getStdpromptDirectory() {
        if (m_stdpromptDirectory != null) {
            return m_stdpromptDirectory.getPath();
        }
        return null;
    }

    public String getMediaFileURL(String userId, String folder, String messageId) {
        String url = "https://%s:%s/media/%s/%s/%s";
        return String.format(url, getHost(), getPort(), userId, folder, messageId);
    }

    public List<String> getFolderIds() {
        // to support custom folders, return these names and any additional
        // directories here
        return Arrays.asList(new String[] {
            "inbox", "conference", "deleted", "saved"
        });
    }

    @Override
    public void onDelete(Object entity) {
        if (m_active) {
            if (entity instanceof User) {
                User user = (User) entity;
                removePersonalAttendantForUser(user);
                deleteMailbox(user.getUserName());
            } else if (entity instanceof Location) {
                init();
            }
        }
    }

    public void onSave(Object entity) {
        if (entity instanceof Location) {
            init();
        }
    }

    public void setActive(boolean active) {
        m_active = active;
    }

    public String getMailstoreDirectory() {
        return m_mailstoreDirectory.getPath();
    }

    public void setMailstoreDirectory(String mailstoreDirectory) {
        m_mailstoreDirectory = new File(mailstoreDirectory);
    }

    protected File getMailstoreFileDirectory() {
        return m_mailstoreDirectory;
    }

    public void setStdpromptDirectory(String stdpromptDirectory) {
        m_stdpromptDirectory = new File(stdpromptDirectory);
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    protected CoreContext getCoreContext() {
        return m_coreContext;
    }

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    protected SipxServiceManager getSipxServiceManager() {
        return m_sipxServiceManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    protected LocationsManager getLocationsManager() {
        return m_locationsManager;
    }

    public void setDistributionListsWriter(DistributionListsWriter distributionListsWriter) {
        m_distributionListsWriter = distributionListsWriter;
    }

    protected DistributionListsWriter getDistributionListsWriter() {
        return m_distributionListsWriter;
    }

    public void setDistributionListsReader(DistributionListsReader distributionListsReader) {
        m_distributionListsReader = distributionListsReader;
    }

    protected DistributionListsReader getDistributionListsReader() {
        return m_distributionListsReader;
    }

    public void setMailboxPreferencesWriter(MailboxPreferencesWriter mailboxWriter) {
        m_mailboxPreferencesWriter = mailboxWriter;
    }

    protected MailboxPreferencesWriter getMailboxPreferencesWriter() {
        return m_mailboxPreferencesWriter;
    }

    public void setPersonalAttendantWriter(PersonalAttendantWriter personalAttendantWriter) {
        m_personalAttendantWriter = personalAttendantWriter;
    }

    protected PersonalAttendantWriter getPersonalAttendantWriter() {
        return m_personalAttendantWriter;
    }

    protected String getMailboxServerUrl() {
        return String.format("https://%s:%s", getHost(), getPort());
    }

    protected String getHost() {
        if (m_host == null) {
            init();
        }
        return m_host;
    }

    protected String getPort() {
        if (m_port == null) {
            init();
        }
        return m_port;
    }

    public void init() {
        m_active = true;
        SipxIvrService ivrService = (SipxIvrService) getSipxServiceManager().getServiceByBeanId(
                SipxIvrService.BEAN_ID);

        Location ivrLocation = (Location) singleResult(getLocationsManager().getLocationsForService(ivrService));
        if (ivrLocation != null) {
            m_host = ivrLocation.getFqdn();
            m_port = ivrService.getHttpsPort();
        }
    }

    public abstract void deleteMailbox(String userId);

}
