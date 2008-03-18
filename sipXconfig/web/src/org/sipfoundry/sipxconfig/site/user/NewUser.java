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

import org.apache.commons.lang.RandomStringUtils;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.FormActions;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.site.admin.ExtensionPoolsPage;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

public abstract class NewUser extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "user/NewUser";
    private static final int SIP_PASSWORD_LEN = 8;

    public abstract CoreContext getCoreContext();

    public abstract SettingDao getSettingDao();

    public abstract User getUser();

    public abstract void setUser(User user);

    public abstract boolean isStay();

    public abstract String getButtonPressed();

    public abstract MailboxManager getMailboxManager();

    public IPage onCommit(IRequestCycle cycle) {
        if (!TapestryUtils.isValid(this)) {
            return null;
        }
        // Save the user
        CoreContext core = getCoreContext();
        User user = getUser();
        EditGroup.saveGroups(getSettingDao(), user.getGroups());
        core.saveUser(user);

        MailboxManager mmgr = getMailboxManager();
        if (mmgr.isEnabled()) {
            String userName = user.getUserName();
            mmgr.deleteMailbox(userName);
            Mailbox mailbox = mmgr.getMailbox(userName);
            mmgr.saveMailboxPreferences(mailbox, getMailboxPreferences());
        }

        // On saving the user, transfer control to edituser page.
        if (FormActions.APPLY.equals(getButtonPressed())) {
            EditUser edit = (EditUser) cycle.getPage(EditUser.PAGE);
            edit.setUserId(user.getId());
            return edit;
        }

        return null;
    }

    private MailboxPreferences getMailboxPreferences() {
        return (MailboxPreferences) getBeans().getBean("mailboxPreferences");
    }

    public IPage extensionPools(IRequestCycle cycle) {
        ExtensionPoolsPage poolsPage = (ExtensionPoolsPage) cycle
                .getPage(ExtensionPoolsPage.PAGE);
        poolsPage.setReturnPage(this);
        return poolsPage;
    }

    public void setReturnPage(String returnPageName) {
        super.setReturnPage(returnPageName);
        setCallback(new OptionalStay(getCallback()));
    }

    class OptionalStay implements ICallback {
        private ICallback m_delegate;

        OptionalStay(ICallback delegate) {
            m_delegate = delegate;
        }

        public void performCallback(IRequestCycle cycle) {
            if (isStay() && FormActions.OK.equals(getButtonPressed())) {
                // Explicitly null out information that should not be used for multiple users,
                // otherwise keep form values as is theory that creating users in bulk will want
                // all the same settings by default
                setUser(null);
                
                MailboxPreferences mailboxPrefs = getMailboxPreferences();
                // Clear the email addresses
                mailboxPrefs.setEmailAddress(null); // XCF-1523
                mailboxPrefs.setAlternateEmailAddress(null);
                
                // Reset (clear) the voicemail checkboxes
                mailboxPrefs.setAttachVoicemailToEmail(false);
                mailboxPrefs.setAttachVoicemailToAlternateEmail(false);
                
                cycle.activate(PAGE);
            } else if (m_delegate != null) {
                m_delegate.performCallback(cycle);
            }
        }
    }

    public void pageBeginRender(PageEvent event) {
        User user = getUser();
        if (user == null) {
            user = new User();
            user.setSipPassword(RandomStringUtils.randomAlphanumeric(SIP_PASSWORD_LEN));
            setUser(user);
        }
    }
}
