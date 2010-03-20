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
import java.util.Iterator;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.components.selection.OptGroup;
import org.sipfoundry.sipxconfig.setting.Group;

public abstract class ManageUsers extends SipxBasePage {

    public static final String PAGE = "user/ManageUsers";
    private static final String USER_TABLE_COMPONENT_ID = "userTable";

    public abstract CoreContext getCoreContext();

    public abstract Integer getGroupId();

    public abstract void setGroupId(Integer groupId);

    public IPage addUser(IRequestCycle cycle) {
        NewUser page = (NewUser) cycle.getPage(NewUser.PAGE);
        page.setReturnPage(PAGE);
        return page;
    }

    public IPage editUser(IRequestCycle cycle, Integer userId) {
        EditUser page = (EditUser) cycle.getPage(EditUser.PAGE);
        page.setUserId(userId);
        page.setReturnPage(PAGE);
        return page;
    }

    public void deleteUsers() {
        UserTable table = (UserTable) getComponent(USER_TABLE_COMPONENT_ID);
        SelectMap selections = table.getSelections();
        Collection selected = selections.getAllSelected();
        getCoreContext().deleteUsers(selected);
    }

    public IPropertySelectionModel getActionModel() {
        Collection groups = getCoreContext().getGroups();
        Collection actions = new ArrayList(groups.size());

        Group removeFromGroup = null;
        for (Iterator i = groups.iterator(); i.hasNext();) {
            Group g = (Group) i.next();
            if (g.getId().equals(getGroupId())) {
                // do not add the "remove from" group...
                removeFromGroup = g;
                continue;
            }
            if (actions.size() == 0) {
                actions.add(new OptGroup(getMessages().getMessage("label.addTo")));
            }
            actions.add(new AddToUserGroupAction(g, getCoreContext()));
        }

        if (removeFromGroup != null) {
            actions.add(new OptGroup(getMessages().getMessage("label.removeFrom")));
            actions.add(new RemoveFromUserGroupAction(removeFromGroup, getCoreContext()));
        }

        AdaptedSelectionModel model = new AdaptedSelectionModel();
        model.setCollection(actions);
        return model;
    }
}
