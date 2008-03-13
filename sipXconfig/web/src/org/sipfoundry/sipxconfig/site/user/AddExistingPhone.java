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
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.components.selection.OptGroup;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.search.SearchManager;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.site.phone.AddToPhoneGroupAction;
import org.sipfoundry.sipxconfig.site.phone.PhoneTableModel;
import org.sipfoundry.sipxconfig.site.phone.RemoveFromPhoneGroupAction;
import org.sipfoundry.sipxconfig.site.phone.SearchPhoneTableModel;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class AddExistingPhone extends UserBasePage {
    public static final String PAGE = "user/AddExistingPhone";

    @Bean
    public abstract SelectMap getSelections();

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

    public abstract IPropertySelectionModel getActionModel();

    public abstract void setActionModel(IPropertySelectionModel model);

    @Persist
    public abstract String getQueryText();

    @InitialValue("false")
    @Persist
    public abstract boolean getSearchMode();

    @Persist
    public abstract Collection<Integer> getGenerateProfileIds();

    public IBasicTableModel getTableModel() {
        String queryText = getQueryText();
        if (!getSearchMode() || StringUtils.isBlank(queryText)) {
            return new PhoneTableModel(getPhoneContext(), getGroupId());
        }
        return new SearchPhoneTableModel(getSearchManager(), queryText, getPhoneContext());
    }

    /**
     * called before page is drawn
     */
    public void pageBeginRender(PageEvent event_) {
        initActionsModel();
    }

    private void initActionsModel() {
        Collection<Group> groups = getPhoneContext().getGroups();
        Collection actions = new ArrayList(groups.size());

        Group removeFromGroup = null;
        for (Group g : groups) {
            if (g.getId().equals(getGroupId())) {
                // do not add the "remove from" group...
                removeFromGroup = g;
                continue;
            }
            if (actions.size() == 0) {
                actions.add(new OptGroup(getMessages().getMessage("label.addTo")));
            }
            actions.add(new AddToPhoneGroupAction(g, getPhoneContext()));
        }

        if (removeFromGroup != null) {
            actions.add(new OptGroup(getMessages().getMessage("label.removeFrom")));
            actions.add(new RemoveFromPhoneGroupAction(removeFromGroup, getPhoneContext()));
        }

        AdaptedSelectionModel model = new AdaptedSelectionModel();
        model.setCollection(actions);
        setActionModel(model);
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
