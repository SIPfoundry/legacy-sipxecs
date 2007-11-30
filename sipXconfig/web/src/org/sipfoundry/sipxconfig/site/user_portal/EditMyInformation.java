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

import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.admin.ModelWithDefaults;
import org.sipfoundry.sipxconfig.site.user.EditPinComponent;
import org.sipfoundry.sipxconfig.site.user.UserForm;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendant;

public abstract class EditMyInformation extends UserBasePage implements EditPinComponent {
    private static final String OPERATOR_SETTING = 
        "personal-attendant" + Setting.PATH_DELIM + "operator";
    
    public abstract String getPin();

    public abstract User getUserForEditing();

    public abstract void setUserForEditing(User user);

    public abstract MailboxPreferences getMailboxPreferences();

    public abstract void setMailboxPreferences(MailboxPreferences preferences);
    
    public abstract IPropertySelectionModel getLanguageList();
    public abstract void setLanguageList(IPropertySelectionModel languageList);

    @Persist
    public abstract PersonalAttendant getPersonalAttendant();
    public abstract void setPersonalAttendant(PersonalAttendant pa);
    
    @Persist
    @InitialValue(value = "literal:info")
    public abstract String getTab();

    @InjectObject(value = "spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();
    
    @InjectObject(value = "spring:localizationContext")
    public abstract LocalizationContext getLocalizationContext();

    public void save() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        User user = getUserForEditing();
        UserForm.updatePin(this, user, getCoreContext().getAuthorizationRealm());
        getCoreContext().saveUser(user);

        savePersonalAttendant(user);
    }

    private void savePersonalAttendant(User user) {
        MailboxManager mailMgr = getMailboxManager();
        if (mailMgr.isEnabled()) {
            Mailbox mailbox = mailMgr.getMailbox(user.getUserName());
            mailMgr.saveMailboxPreferences(mailbox, getMailboxPreferences());
        }

        mailMgr.storePersonalAttendant(getPersonalAttendant());
        
        user.getSettings().getSetting(OPERATOR_SETTING).setValue(getPersonalAttendant().getOperator());
        getCoreContext().saveUser(user);
    }

    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);
        
        if (getLanguageList() == null) {
            initLanguageList();
        }

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

        PersonalAttendant personalAttendant = getPersonalAttendant();
        if (personalAttendant == null) {
            PersonalAttendant pa = mailMgr.loadPersonalAttendantForUser(user);
            setPersonalAttendant(pa);
        }
    }
    
    private void initLanguageList() {
        String[] languages = getLocalizationContext().getInstalledLanguages();
        IPropertySelectionModel model = new ModelWithDefaults(getMessages(), languages);
        setLanguageList(model);
    }
}
