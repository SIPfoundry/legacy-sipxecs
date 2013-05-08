/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.site.admin.time;

import java.util.Collections;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.time.NtpManager;

import com.davekoelle.AlphanumComparator;

public abstract class UserTimeZoneComponent extends BaseComponent implements PageBeginRenderListener {

    @Parameter(required = true)
    public abstract User getUser();

    public abstract void setUser(User user);

    @Parameter(required = true)
    public abstract User getEditedUser();

    public abstract void setEditedUser(User user);

    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectObject("spring:ntpManager")
    public abstract NtpManager getTimeManager();

    public abstract IPropertySelectionModel getTimezoneTypeModel();

    public abstract void setTimezoneTypeModel(IPropertySelectionModel model);

    public abstract String getTimezoneType();

    public abstract void setTimezoneType(String type);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Override
    public void pageBeginRender(PageEvent event) {
        // Init. the timezone dropdown menu.
        List<String> timezoneList = getTimeManager().getAvailableTimezones();

        // Sort list alphanumerically.
        Collections.sort(timezoneList, new AlphanumComparator());
        StringPropertySelectionModel model = new StringPropertySelectionModel(
                timezoneList.toArray(new String[timezoneList.size()]));
        setTimezoneTypeModel(model);
        if (!event.getRequestCycle().isRewinding()) {
            setTimezoneType(getUser().getTimezone().getID());
        }
    }

    public boolean isRenderBranchOption() {
        return getUser().getUserBranch() != null;
    }

    public boolean isRenderTimeZoneDropDown() {
        return getUser().getInheritedBranch() == null
                || !(Boolean) getEditedUser().getSettings().getSetting("timezone/useBranchTimezone").getTypedValue();
    }

    public void onApply() {
        getEditedUser().setSettingValue("timezone/timezone", getTimezoneType());
        getCoreContext().saveUser(getEditedUser());
    }

}
