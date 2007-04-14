/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.user.EditPinComponent;
import org.sipfoundry.sipxconfig.site.user.UserForm;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;


public abstract class EditMyInformation extends UserBasePage implements EditPinComponent {
    
    public abstract String getPin();
    
    public abstract User getUserForEditing();
    public abstract void setUserForEditing(User user);
    
    public abstract MailboxPreferences getMailboxPreferences();
    public abstract void setMailboxPreferences(MailboxPreferences preferences);
    
    @InjectObject(value = "spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();
    
    public void save() {
        if (TapestryUtils.isValid(this)) {
            User user = getUserForEditing(); 
            UserForm.updatePin(this, user, getCoreContext().getAuthorizationRealm());
            getCoreContext().saveUser(user);
            
            MailboxManager mailMgr = getMailboxManager();
            if (mailMgr.isEnabled()) {
                Mailbox mailbox = mailMgr.getMailbox(user.getUserName());
                mailMgr.saveMailboxPreferences(mailbox, getMailboxPreferences());
            }
        }
    }
    
    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);
        
        User user = getUserForEditing(); 
        if (user == null) {
            user = getUser();
            setUserForEditing(user);
        }

        if (getPin() == null) {
            UserForm.initializePin(getComponent("pin"), this, user);
        }        
        
        MailboxManager mailMgr = getMailboxManager();
        if (getMailboxPreferences() == null && mailMgr.isEnabled()) {
            Mailbox mailbox = mailMgr.getMailbox(user.getUserName());
            setMailboxPreferences(mailMgr.loadMailboxPreferences(mailbox));            
        }
    }
}
