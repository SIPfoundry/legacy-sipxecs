/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.cdr;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.cdr.CdrSearch.Mode;
import org.sipfoundry.sipxconfig.components.LocalizedOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.NewEnumPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryContext;

public abstract class CdrFilter extends BaseComponent {

    public abstract boolean getSearchMode();

    public abstract TapestryContext getTapestry();

    public abstract void setSelectionModel(IPropertySelectionModel model);

    public abstract IPropertySelectionModel getSelectionModel();

    protected void prepareForRender(IRequestCycle cycle) {
        if (getSelectionModel() == null) {
            NewEnumPropertySelectionModel model = new NewEnumPropertySelectionModel();
            model.setEnumType(Mode.class);

            LocalizedOptionModelDecorator decoratedModel = new LocalizedOptionModelDecorator();
            decoratedModel.setMessages(getMessages());
            decoratedModel.setModel(model);
            decoratedModel.setResourcePrefix("filter.");

            setSelectionModel(getTapestry().addExtraOption(decoratedModel, getMessages(),
                    "label.filter"));
        }
    }
}
