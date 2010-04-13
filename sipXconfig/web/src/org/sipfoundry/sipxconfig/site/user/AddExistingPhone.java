/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.search.SearchManager;
import org.sipfoundry.sipxconfig.site.phone.PhoneTableModel;
import org.sipfoundry.sipxconfig.site.phone.PhoneWithNoLinesTableModel;
import org.sipfoundry.sipxconfig.site.phone.SearchPhoneTableModel;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class AddExistingPhone extends UserBasePage {
    public static final String PAGE = "user/AddExistingPhone";

    @Bean
    public abstract SelectMap getSelections();

    @Override
    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    @InjectObject(value = "spring:searchManager")
    public abstract SearchManager getSearchManager();

    @InjectObject(value = "spring:phoneProfileManager")
    public abstract ProfileManager getPhoneProfileManager();

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    @Persist
    public abstract Integer getGroupId();

    public abstract void setGroupId(Integer groupId);

    @Persist
    public abstract String getQueryText();

    @InitialValue("false")
    @Persist
    public abstract boolean getSearchMode();

    @Persist
    public abstract Collection<Integer> getGenerateProfileIds();

    @InitialValue("false")
    @Persist
    public abstract boolean getUnassignedMode();

    public IBasicTableModel getTableModel() {
        String queryText = getQueryText();
        if (getUnassignedMode()) {
            return new PhoneWithNoLinesTableModel(getPhoneContext());
        } else if (!getSearchMode() || StringUtils.isBlank(queryText)) {
            return new PhoneTableModel(getPhoneContext(), getGroupId());
        }
        return new SearchPhoneTableModel(getSearchManager(), queryText, getPhoneContext());
    }

    public void select(IRequestCycle cycle) {
        List<Integer> users = new ArrayList<Integer>();
        users.add(getUserId());
        Collection selectedPhones = getSelections().getAllSelected();
        for (Object selected : selectedPhones) {
            getPhoneContext().addUsersToPhone((Integer) selected, users);
        }
        getCallback().performCallback(cycle);
    }

    public void cancel(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }
}
