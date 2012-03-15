/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
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
