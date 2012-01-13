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
 * Pull strings from an optional IMessages class, available from all components via "messages".
 */
public class LocalizedOptionModelDecorator extends CustomOptionModelDecorator {
    private String m_prefix;

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

    @Override
    public String getLabel(String rawLabel) {
        String label = LocalizationUtils.localize(getMessages(), m_prefix, rawLabel);
        return label;
    }
}
