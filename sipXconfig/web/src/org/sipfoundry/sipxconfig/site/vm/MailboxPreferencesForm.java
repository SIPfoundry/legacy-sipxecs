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
public abstract class MailboxPreferencesForm extends BaseComponent implements PageBeginRenderListener {
    private static final String ATTACH_TYPE = "attachType.";

    @InjectObject(value = "spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();

    public abstract void setActiveGreetingModel(IPropertySelectionModel model);

    public abstract IPropertySelectionModel getActiveGreetingModel();

    public abstract void setVoicemailPropertiesModel(IPropertySelectionModel model);

    public abstract IPropertySelectionModel getVoicemailPropertiesModel();

    public abstract void setMailFormatModel(IPropertySelectionModel model);

    public abstract IPropertySelectionModel getMailFormatModel();

    public abstract void setAlternateEmailNotifyModel(IPropertySelectionModel model);

    public abstract IPropertySelectionModel getAlternateEmailNotifyModel();

    @Parameter(required = true)
    public abstract MailboxPreferences getPreferences();

    @Parameter(required = false, defaultValue = "ognl:false")
    public abstract boolean isAdmin();

    @Parameter(required = false, defaultValue = "ognl:false")
    public abstract boolean isAdvanced();

    public void pageBeginRender(PageEvent event) {
        if (getActiveGreetingModel() == null) {
            NewEnumPropertySelectionModel<ActiveGreeting> rawModel = new NewEnumPropertySelectionModel();
            rawModel.setEnumType(ActiveGreeting.class);
            setActiveGreetingModel(new LocalizedOptionModelDecorator(rawModel, getMessages(), "activeGreeting."));
        }
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
    }
}
