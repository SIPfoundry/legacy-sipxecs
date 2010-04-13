/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.conference;

import java.util.Collection;

import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.site.user.SelectUsers;
import org.sipfoundry.sipxconfig.site.user.SelectUsersCallback;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class ConferenceOwnerPanel extends BaseComponent {

    private static final String CALLBACK_PROPERTY_NAME = "selectedUsers";

    @Parameter(required = true)
    public abstract void setConference(Conference conference);
    public abstract Conference getConference();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @Asset("/images/user.png")
    public abstract IAsset getUserIcon();

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        Conference conference = getConference();
        Collection<Integer> selectedUsers = (Collection<Integer>) PropertyUtils.read(getPage(), CALLBACK_PROPERTY_NAME);
        if (selectedUsers != null && !selectedUsers.isEmpty()) {
            Integer newOwnerId = selectedUsers.iterator().next();
            if (!conference.hasOwner() || !conference.getOwner().getId().equals(newOwnerId)) {
                CoreContext coreContext = getCoreContext();
                User newOwner = coreContext.loadUser(newOwnerId);
                conference.setOwner(newOwner);
            }
        }

        super.renderComponent(writer, cycle);
    }

    public IPage changeOwner(IRequestCycle cycle) {
        IPage page = getPage();
        PropertyUtils.write(page, "transientConference", getConference());
        SelectUsers selectUsersPage = (SelectUsers) cycle.getPage(SelectUsers.PAGE);
        SelectUsersCallback callback = new SelectUsersCallback(getPage());
        callback.setIdsPropertyName(CALLBACK_PROPERTY_NAME);
        selectUsersPage.setCallback(callback);
        selectUsersPage.setTitle(getMessages().getMessage("title.selectRings"));
        selectUsersPage.setPrompt(getMessages().getMessage("prompt.selectRings"));
        return selectUsersPage;
    }

    public void unassign() {
        getConference().setOwner(null);
    }
}
