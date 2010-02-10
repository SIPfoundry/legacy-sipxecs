/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.vm;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.LocalizedOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.NewEnumPropertySelectionModel;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.ActiveGreeting;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.AttachType;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.MailFormat;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.VoicemailTuiType;

public abstract class MailboxPreferencesForm extends BaseComponent implements PageBeginRenderListener {
    private static final String ATTACH_TYPE = "attachType.";
    private static final String VOICEMAIL_TUI_TYPE = "voicemailTuiType.";
    private static final String ACTIVE_GREETING_TYPE = "activeGreeting.";

    @InjectObject(value = "spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();

    public abstract void setVoicemailPropertiesModel(IPropertySelectionModel model);

    public abstract IPropertySelectionModel getVoicemailPropertiesModel();

    public abstract void setMailFormatModel(IPropertySelectionModel model);

    public abstract IPropertySelectionModel getMailFormatModel();

    public abstract void setAlternateEmailNotifyModel(IPropertySelectionModel model);

    public abstract IPropertySelectionModel getAlternateEmailNotifyModel();

    public abstract void setVoicemailTuiModel(IPropertySelectionModel model);

    public abstract IPropertySelectionModel getVoicemailTuiModel();

    @Parameter(required = true)
    public abstract MailboxPreferences getPreferences();

    @Parameter(required = false, defaultValue = "ognl:false")
    public abstract boolean isAdmin();

    @Parameter(required = false, defaultValue = "ognl:false")
    public abstract boolean isAdvanced();

    public IPropertySelectionModel getActiveGreetingModel() {
        NewEnumPropertySelectionModel<ActiveGreeting> rawModel = new NewEnumPropertySelectionModel();
        rawModel.setOptions(getPreferences().getOptionsForActiveGreeting());
        return (new LocalizedOptionModelDecorator(rawModel, getMessages(), ACTIVE_GREETING_TYPE));
    }

    public void pageBeginRender(PageEvent event) {
        if (getVoicemailPropertiesModel() == null) {
            NewEnumPropertySelectionModel<AttachType> rawModel = new NewEnumPropertySelectionModel<AttachType>();
            rawModel.setOptions(getPreferences().getAttachOptions(isAdmin()));
            setVoicemailPropertiesModel(new LocalizedOptionModelDecorator(rawModel, getMessages(), ATTACH_TYPE));
        }
        if (getMailFormatModel() == null) {
            NewEnumPropertySelectionModel<MailFormat> rawModel = new NewEnumPropertySelectionModel<MailFormat>();
            rawModel.setEnumType(MailFormat.class);
            setMailFormatModel(new LocalizedOptionModelDecorator(rawModel, getMessages(), "mailFormat."));
        }
        if (getAlternateEmailNotifyModel() == null) {
            NewEnumPropertySelectionModel<AttachType> rawModel = new NewEnumPropertySelectionModel<AttachType>();
            rawModel.setOptions(getPreferences().getAttachOptionsForAlternateEmail());
            setAlternateEmailNotifyModel(new LocalizedOptionModelDecorator(rawModel, getMessages(), ATTACH_TYPE));
        }
        if (getVoicemailTuiModel() == null) {
            NewEnumPropertySelectionModel<VoicemailTuiType> rawModel =
                new NewEnumPropertySelectionModel<VoicemailTuiType>();
            String promptDir = getMailboxManager().getStdpromptDirectory();
            rawModel.setOptions(getPreferences().getOptionsForVoicemailTui(promptDir));
            setVoicemailTuiModel(new LocalizedOptionModelDecorator(rawModel, getMessages(), VOICEMAIL_TUI_TYPE));
        }
    }
}
