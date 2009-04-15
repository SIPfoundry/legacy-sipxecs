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

public abstract class MailboxPreferencesForm extends BaseComponent implements PageBeginRenderListener {
    
    @InjectObject(value = "spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();
    
    public abstract void setActiveGreetingModel(IPropertySelectionModel model);
    public abstract IPropertySelectionModel getActiveGreetingModel();

    @Parameter(required = true)
    public abstract MailboxPreferences getPreferences();
    
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
}
