/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.user;

import java.util.List;

import org.apache.commons.lang.RandomStringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.FormActions;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.GroupAutoAssign;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.site.admin.ExtensionPoolsPage;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;
import org.sipfoundry.sipxconfig.vm.MailboxManager;

public abstract class NewUser extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "user/NewUser";

    private static final Log LOG = LogFactory.getLog(NewUser.class);
    private static final int SIP_PASSWORD_LEN = 8;

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectObject("spring:forwardingContext")
    public abstract ForwardingContext getForwardingContext();

    @InjectObject("spring:settingDao")
    public abstract SettingDao getSettingDao();

    @InjectObject("spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();

    public abstract User getUser();

    public abstract void setUser(User user);

    public abstract boolean isStay();

    public abstract String getButtonPressed();

    public List<Group> getAvailableGroups() {
        return getCoreContext().getGroups();
    }

    public IPage commit(IRequestCycle cycle) {
        if (!TapestryUtils.isValid(this)) {
            return null;
        }
        // Save the user
        CoreContext core = getCoreContext();
        User user = getUser();
        user.setImId(user.getUserName());
        EditGroup.saveGroups(getSettingDao(), user.getGroups());

        // Execute the automatic assignments for the user.
        GroupAutoAssign groupAutoAssign = new GroupAutoAssign(getConferenceBridgeContext(), getCoreContext(),
                                                              getForwardingContext(), getMailboxManager());
        groupAutoAssign.assignUserData(user);

        // On saving the user, transfer control to edituser page.
        if (FormActions.APPLY.equals(getButtonPressed())) {
            EditUser edit = (EditUser) cycle.getPage(EditUser.PAGE);
            edit.setUserId(user.getId());
            return edit;
        }

        return null;
    }

    public IPage extensionPools(IRequestCycle cycle) {
        ExtensionPoolsPage poolsPage = (ExtensionPoolsPage) cycle.getPage(ExtensionPoolsPage.PAGE);
        poolsPage.setReturnPage(this);
        return poolsPage;
    }

    @Override
    protected ICallback createCallback(String pageName) {
        ICallback createCallback = super.createCallback(pageName);
        return new OptionalStay(createCallback);
    }

    class OptionalStay implements ICallback {
        private final ICallback m_delegate;

        OptionalStay(ICallback delegate) {
            m_delegate = delegate;
        }

        public void performCallback(IRequestCycle cycle) {
            if (isStay() && FormActions.OK.equals(getButtonPressed())) {
                // Explicitly null out information that should not be used for multiple users,
                // otherwise keep form values as is theory that creating users in bulk will want
                // all the same settings by default
                setUser(null);
                cycle.activate(PAGE);
            } else if (m_delegate != null) {
                m_delegate.performCallback(cycle);
            }
        }
    }

    public void pageBeginRender(PageEvent event) {
        User user = getUser();
        if (user == null) {
            user = getCoreContext().newUser();
            user.setSipPassword(RandomStringUtils.randomAlphanumeric(SIP_PASSWORD_LEN));
            setUser(user);
        }
    }
}
