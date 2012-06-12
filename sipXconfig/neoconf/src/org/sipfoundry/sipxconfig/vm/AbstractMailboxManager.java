/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.vm;

import java.io.File;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendantManager;

public abstract class AbstractMailboxManager extends PersonalAttendantManager implements DaoEventListener {
    protected static final String PATH_MAILBOX = "/mailbox/";
    protected static final String PATH_MESSAGE = "/message/";
    private File m_stdpromptDirectory;
    private CoreContext m_coreContext;
    private LocationsManager m_locationsManager;
    private AddressManager m_addressManager;
    private FeatureManager m_featureManager;

    public boolean isSystemCpui() {
        return m_featureManager.isFeatureEnabled(Ivr.CALLPILOT);
    }

    public String getStdpromptDirectory() {
        if (m_stdpromptDirectory != null) {
            return m_stdpromptDirectory.getPath();
        }
        return null;
    }

    public String getMediaFileURL(String userId, String folder, String messageId) {
        String url = "http://%s:%s/media/%s/%s/%s";
        Address ivrAddress = m_addressManager.getSingleAddress(Ivr.REST_API);
        if (ivrAddress != null) {
            return String.format(url, ivrAddress.getAddress(), ivrAddress.getPort(), userId, folder, messageId);
        }
        return null;
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
        if (entity instanceof User) {
            User user = (User) entity;
            removePersonalAttendantForUser(user);
            if (m_featureManager.isFeatureEnabled(Ivr.FEATURE)) {
                deleteMailbox(user.getUserName());
            }
        }
    }

    @Override
    public void onSave(Object entity) {
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


    public void saveDistributionLists(Integer userId, DistributionList[] lists) {
        Collection<String> aliases = DistributionList.getUniqueExtensions(lists);
        getCoreContext().checkForValidExtensions(aliases, PermissionName.VOICEMAIL);
        User user = getCoreContext().getUser(userId);
        for (int i = 1; i < lists.length; i++) {
            user.setSettingValue(new StringBuilder(DistributionList.SETTING_PATH_DISTRIBUTION_LIST).
                        append(i).toString(), joinBySpace(lists[i].getExtensions()));
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

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    protected LocationsManager getLocationsManager() {
        return m_locationsManager;
    }

    protected String getMailboxServerUrl() {
        Address ivrAddress = m_addressManager.getSingleAddress(Ivr.REST_API);
        return ivrAddress.toString();
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public abstract void deleteMailbox(String userId);

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    public AddressManager getAddressManager() {
        return m_addressManager;
    }

    public FeatureManager getFeatureManager() {
        return m_featureManager;
    }
}
