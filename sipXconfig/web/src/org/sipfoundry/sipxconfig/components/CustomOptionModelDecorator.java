/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.hivemind.Messages;
import org.apache.tapestry.form.IPropertySelectionModel;

public abstract class CustomOptionModelDecorator implements IPropertySelectionModel {
    private Messages m_messages;
    private IPropertySelectionModel m_model;

    public void setMessages(Messages messages) {
        m_messages = messages;
    }

    public int getOptionCount() {
        return m_model.getOptionCount();
    }

    public Object getOption(int index) {
        return m_model.getOption(index);
    }

    public String getValue(int index) {
        return m_model.getValue(index);
    }

    public Object translateValue(String value) {
        return m_model.translateValue(value);
    }

    public void setModel(IPropertySelectionModel model) {
        m_model = model;
    }

    public String getRawLabel(int index) {
        return getModel().getLabel(index);
    }

    public String getLabel(int index) {
        String rawLabel = getModel().getLabel(index);
        if (getMessages() == null) {
            return rawLabel;
        }
        return getLabel(rawLabel);
    }

    public abstract String getLabel(String rawLabel);

    public boolean isDisabled(int index) {
        return m_model.isDisabled(index);
    }

    protected Messages getMessages() {
        return m_messages;
    }

    protected IPropertySelectionModel getModel() {
        return m_model;
    }
}
