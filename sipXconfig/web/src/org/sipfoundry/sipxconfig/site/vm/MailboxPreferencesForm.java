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

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.ArrayUtils;
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

    private static final String HOST_SETTING = "unified-messaging/host";
    private static final String PORT_SETTING = "unified-messaging/port";

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
            NewEnumPropertySelectionModel<MailboxPreferences.ActiveGreeting> rawModel
                = new NewEnumPropertySelectionModel();
            rawModel.setEnumType(ActiveGreeting.class);
            model = new LocalizedOptionModelDecorator(rawModel, getMessages(), "activeGreeting.");
            setActiveGreetingModel(model);
        }
    }

    public IPropertySelectionModel getVoicemailPropertiesModel() {
        // FIXME: why not just check user properties directly...
        boolean unifiedMessagingParamsGroup = false;
        String host = getPreferences().getUser().getSettingValue(HOST_SETTING);
        Integer port = (Integer) getPreferences().getUser().getSettingTypedValue(PORT_SETTING);
        if (StringUtils.isNotEmpty(host) && port != null) {
            unifiedMessagingParamsGroup = true;
        }
        String[] options = new String[] {
            MailboxPreferences.ATTACH_VOICEMAIL, MailboxPreferences.DO_NOT_ATTACH_VOICEMAIL
        };
        if (unifiedMessagingParamsGroup) {
            options = (String[]) ArrayUtils.add(options, MailboxPreferences.SYNCHRONIZE_WITH_EMAIL_SERVER);
        }
        StringPropertySelectionModel model = new StringPropertySelectionModel(options);
        return new LocalizedOptionModelDecorator(model, getMessages(), "voicemailProperties.");
    }

    public String getInheritedGroup() {
        List<Group> groups = getPreferences().getUser().getGroupsAsList();
        List<Group> unifiedMessagingGroups = new ArrayList<Group>();
        for (Group group : groups) {
            String host = group.getSettingValue(HOST_SETTING);
            if (StringUtils.isNotEmpty(host)) {
                unifiedMessagingGroups.add(group);
            }
        }
        Group inheritedGroup = Group.selectGroupWithHighestWeight(unifiedMessagingGroups);
        return inheritedGroup != null ? inheritedGroup.getName() : "";
    }
}
