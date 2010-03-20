/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.RandomStringUtils;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.intercom.Intercom;
import org.sipfoundry.sipxconfig.admin.intercom.IntercomManager;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.device.DeviceDescriptor;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class IntercomPage extends SipxBasePage implements PageBeginRenderListener {

    public static final String PAGE = "admin/Intercom";
    private static final int CODE_LEN = 8;

    public abstract Intercom getIntercom();

    public abstract void setIntercom(Intercom intercom);

    public abstract String getGroupsString();

    public abstract void setGroupsString(String groupsString);

    public abstract IntercomManager getIntercomManager();

    public abstract PhoneContext getPhoneContext();

    public abstract SettingDao getSettingDao();

    public abstract Collection getGroupsCandidates();

    public abstract void setGroupCandidates(Collection groupsList);

    @InjectObject("spring:phonesWithIntercom")
    public abstract ModelSource<DeviceDescriptor> getPhonesWithIntercom();

    public void buildGroupCandidates(String groupsString) {
        List allGroups = getPhoneContext().getGroups();
        Collection candidates = TapestryUtils.getAutoCompleteCandidates(allGroups, groupsString);
        setGroupCandidates(candidates);
    }

    public void pageBeginRender(PageEvent event) {
        // Look up the Intercom, creating it if necessary
        Intercom intercom = getIntercom();
        if (intercom == null) {
            intercom = getIntercomManager().getIntercom();
            setIntercom(intercom);
        }

        // Initialize the groups string
        if (getGroupsString() == null) {
            setGroupsString(intercom.getGroupsNames());
        }

        // Create a random code if no code has been set
        String code = intercom.getCode();
        if (code == null) {
            intercom.setCode(RandomStringUtils.randomAlphanumeric(CODE_LEN));
        }
    }

    /**
     * Listeners
     */
    public void commit() {
        // Proceed only if Tapestry validation succeeded
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        // If there is an Intercom, then update and save it
        Intercom intercom = getIntercom();
        String groupsString = getGroupsString();
        if (groupsString != null) {
            List groups = getSettingDao()
                    .getGroupsByString(Phone.GROUP_RESOURCE_ID, groupsString, true);
            intercom.setGroupsAsList(groups);
        }

        getIntercomManager().saveIntercom(intercom);
    }
}
