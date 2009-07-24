/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.vm;

import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.LocalizedOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.NewEnumPropertySelectionModel;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.ActiveGreeting;

public abstract class MailboxPreferencesForm extends BaseComponent implements PageBeginRenderListener {

    @InjectObject(value = "spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();

    public abstract void setActiveGreetingModel(IPropertySelectionModel model);

    public abstract IPropertySelectionModel getActiveGreetingModel();

    @Parameter(required = true)
    public abstract MailboxPreferences getPreferences();

    @Parameter(required = false, defaultValue = "ognl:false")
    public abstract boolean isAdmin();

    public void pageBeginRender(PageEvent event) {
        IPropertySelectionModel model = getActiveGreetingModel();
        if (model == null) {
            NewEnumPropertySelectionModel<MailboxPreferences.ActiveGreeting> rawModel =
                new NewEnumPropertySelectionModel();
            rawModel.setEnumType(ActiveGreeting.class);
            model = new LocalizedOptionModelDecorator(rawModel, getMessages(), "activeGreeting.");
            setActiveGreetingModel(model);
        }
    }

    public IPropertySelectionModel getVoicemailPropertiesModel() {
        List<Group> groups = getPreferences().getUser().getGroupsAsList();
        boolean unifiedMessagingParamsGroup = false;
        for (Group group : groups) {
            String host = group.getSettingValue("unified-messaging/host");
            Integer port = null;
            try {
                // hack get the value of the setting returning default value
                // this is due to inheriting ValueStorage instead of BeanWithSettings
                port = Integer.valueOf(group.inherhitSettingsForEditing(getPreferences().getUser()).getSetting(
                        "unified-messaging/port").getValue());
            } catch (NumberFormatException e) {
                // do nothing ... a senseless thing in order to pass checkstyle
                port = null;
            }
            if (StringUtils.isNotEmpty(host) && port != null) {
                unifiedMessagingParamsGroup = true;
                break;
            }
        }
        String[] options;
        if (unifiedMessagingParamsGroup) {
            options = new String[] {
                getPreferences().ATTACH_VOICEMAIL, getPreferences().DO_NOT_ATTACH_VOICEMAIL,
                getPreferences().SYNCHRONIZE_WITH_EMAIL_SERVER
            };
        } else {
            options = new String[] {
                getPreferences().ATTACH_VOICEMAIL, getPreferences().DO_NOT_ATTACH_VOICEMAIL
            };
        }
        StringPropertySelectionModel model = new StringPropertySelectionModel(options);
        return new LocalizedOptionModelDecorator(model, getMessages(), "voicemailProperties.");
    }
}
