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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.components.Block;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.FaxServicePanel;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.imbot.ImBot;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.user.EditPinComponent;
import org.sipfoundry.sipxconfig.site.user.UserForm;
import org.sipfoundry.sipxconfig.update.XmppContactInformationUpdate;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendant;

public abstract class EditMyInformation extends UserBasePage implements EditPinComponent {

    public static final String TAB_CONFERENCES = "conferences";

    @InjectObject(value = "spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();

    @InjectObject(value = "spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    @InjectObject(value = "spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    @InjectObject("spring:xmppContactInformationUpdate")
    public abstract XmppContactInformationUpdate getXmppContactInformationUpdate();

    public abstract String getPin();

    public abstract String getVoicemailPin();

    public abstract Conference getCurrentRow();

    public abstract void setCurrentRow(Conference currentRow);

    public abstract User getUserForEditing();

    public abstract void setUserForEditing(User user);

    public abstract MailboxPreferences getMailboxPreferences();

    public abstract void setMailboxPreferences(MailboxPreferences preferences);

    public abstract IPropertySelectionModel getLanguageList();

    public abstract void setLanguageList(IPropertySelectionModel languageList);

    @Persist
    public abstract PersonalAttendant getPersonalAttendant();

    public abstract void setPersonalAttendant(PersonalAttendant pa);

    public abstract Collection<String> getAvailableTabNames();

    public abstract void setAvailableTabNames(Collection<String> tabNames);

    @Persist
    @InitialValue(value = "literal:extendedInfo")
    public abstract String getTab();

    public abstract Block getActionBlockForConferencesTab();

    public abstract void setActionBlockForConferencesTab(Block b);

    public abstract Setting getImNotificationSettings();

    public abstract void setImNotificationSettings(Setting paSetting);

    public abstract Setting getParentSetting();
    public abstract void setParentSetting(Setting setting);

    public abstract User getLoadedUser();
    public abstract void setLoadedUser(User user);

    public void save() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        User user = getUserForEditing();
        UserForm.updatePin(this, user, getCoreContext().getAuthorizationRealm());
        UserForm.updateVoicemailPin(this, user);

        FaxServicePanel fs = (FaxServicePanel) getComponent("faxServicePanel");
        fs.update(user);
        getMailboxPreferences().updateUser(user);
        getMailboxManager().storePersonalAttendant(getPersonalAttendant());
        getCoreContext().saveUser(user);
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);

        User user = getUserForEditing();
        if (user == null) {
            user = getUser();
            setLoadedUser(user);
        }

        if (getAvailableTabNames() == null) {
            initAvailableTabs();
        }

        Setting personalAttendantSetting = user.getSettings().getSetting("personal-attendant");
        setParentSetting(personalAttendantSetting);
        setUserForEditing(user);

        UserForm.initializePin(getComponent("pin"), this, user);
        UserForm.initializeVoicemailPin(getComponent("voicemail_pin"), this, user);

        MailboxManager mailMgr = getMailboxManager();
        if (getMailboxPreferences() == null && mailMgr.isEnabled()) {
            setMailboxPreferences(new MailboxPreferences(user));
        }

        if (getImNotificationSettings() == null) {
            setImNotificationSettings(user.getSettings().getSetting("im_notification"));
        }

        PersonalAttendant personalAttendant = getPersonalAttendant();
        if (personalAttendant == null) {
            PersonalAttendant pa = mailMgr.loadPersonalAttendantForUser(user);
            setPersonalAttendant(pa);
        }

        if (getTab().equals(TAB_CONFERENCES)) {
            Block b = (Block) getComponent("userConferencesPanel").getComponent("conferencesPanel").getComponent(
                    "conferenceActions");
            setActionBlockForConferencesTab(b);
        }

    }

    public void syncXmppContacts() {
        try {
            getXmppContactInformationUpdate().notifyChange(getUser());
        } catch (Exception e) {
            throw new UserException(getMessages().getMessage("xmpp.sync.error"), e.getMessage());
        }
    }

    private void initAvailableTabs() {
        List<String> tabNames = new ArrayList<String>();
        tabNames.add("extendedInfo");
        tabNames.add("info");
        if (isVoicemailEnabled()) {
            tabNames.add("distributionLists");
        }
        tabNames.add(TAB_CONFERENCES);
        tabNames.add("openfire");

        String mohPermissionValue = getLoadedUser().getSettingValue("permission/application/music-on-hold");
        if (isVoicemailEnabled() && Permission.isEnabled(mohPermissionValue)) {
            tabNames.add("moh");
        }

        String paPermissionValue = getLoadedUser().getSettingValue("permission/application/personal-auto-attendant");
        if (isVoicemailEnabled() && Permission.isEnabled(paPermissionValue)) {
            tabNames.add("menu");
        }

        if (getFeatureManager().isFeatureEnabled(ImBot.FEATURE)) {
            tabNames.add("myAssistant");
        }

        setAvailableTabNames(tabNames);
    }

    public boolean isVoicemailEnabled() {
        return (getFeatureManager().isFeatureEnabled(Ivr.FEATURE) ? true : false);
    }
}
