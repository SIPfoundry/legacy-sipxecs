/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.hivemind.Messages;
import org.apache.tapestry.form.IPropertySelectionModel;

/**
 * Pull strings from an optional IMessages class, available from all components via "messages"
 */
public class LocalizedOptionModelDecorator implements IPropertySelectionModel {

    private Messages m_messages;

    private String m_prefix;

    private IPropertySelectionModel m_model;

    public LocalizedOptionModelDecorator() {
    }

    public LocalizedOptionModelDecorator(IPropertySelectionModel model, Messages messages, String prefix) {
        setModel(model);
        setMessages(messages);
        setResourcePrefix(prefix);
    }

    public void setResourcePrefix(String prefix) {
        m_prefix = prefix;
    }

    public void setMessages(Messages messages) {
        m_messages = messages;
    }

    public String getLabel(int index) {
        String rawLabel = m_model.getLabel(index);
        if (m_messages == null) {
            return rawLabel;
        }

        String label = LocalizationUtils.localize(m_messages, m_prefix, rawLabel);
        return label;
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

    public boolean isDisabled(int index) {
        return m_model.isDisabled(index);
    }
}
