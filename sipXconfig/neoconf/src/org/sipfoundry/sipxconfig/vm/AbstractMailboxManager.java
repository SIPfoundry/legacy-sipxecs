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
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.service.SipxIvrService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.VoicemailTuiType;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendantManager;

public abstract class AbstractMailboxManager extends PersonalAttendantManager implements DaoEventListener {
    protected static final String PATH_MAILBOX = "/mailbox/";
    protected static final String PATH_MESSAGE = "/message/";
    private File m_mailstoreDirectory;
    private File m_stdpromptDirectory;
    private CoreContext m_coreContext;
    private SipxServiceManager m_sipxServiceManager;
    private LocationsManager m_locationsManager;
    private String m_host;
    private String m_port;
    private boolean m_active;
    private String m_binDir;

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

    public DistributionList[] loadDistributionLists(User user) {
        DistributionList[] lists = new DistributionList[DistributionList.MAX_SIZE];
        //0 is not available
        for (int i = 1; i < lists.length; i++) {
            DistributionList dl = new DistributionList();
            dl.setExtensions(StringUtils.split(StringUtils.defaultIfEmpty(user.getSettingValue(
                    new StringBuilder(DistributionList.SETTING_PATH_DISTRIBUTION_LIST).append(i).toString()), "")));
            lists[i] = dl;
        }

        return lists;
    }


    public void saveDistributionLists(User user, DistributionList[] lists) {
        Collection<String> aliases = DistributionList.getUniqueExtensions(lists);
        getCoreContext().checkForValidExtensions(aliases, PermissionName.VOICEMAIL);
        for (int i = 1; i < lists.length; i++) {
            if (lists[i].getExtensions() != null) {
                user.setSettingValue(new StringBuilder(DistributionList.SETTING_PATH_DISTRIBUTION_LIST).
                        append(i).toString(), joinBySpace(lists[i].getExtensions()));
            }
        }
        getCoreContext().saveUser(user);
    }

    public String joinBySpace(String[] array) {
        String s = null;
        if (array != null) {
            s = StringUtils.join(array, ' ');
        }
        return s;
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

    public void setBinDir(String binDir) {
        m_binDir = binDir;
    }

    protected String getBinDir() {
        return m_binDir;
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
