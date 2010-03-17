/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import java.util.ArrayList;
import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.components.selection.OptGroup;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.search.SearchManager;
import org.sipfoundry.sipxconfig.setting.Group;

/**
 * List all the phones/phones for management and details drill-down
 */
public abstract class ManagePhones extends BasePage implements PageBeginRenderListener {
    public static final String PAGE = "phone/ManagePhones";

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
}
