/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.setting;

import java.util.Collection;

import org.apache.hivemind.Messages;
import org.apache.tapestry.IActionListener;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;
import org.sipfoundry.sipxconfig.setting.Group;

public abstract class BulkGroupAction implements OptionAdapter, IActionListener {

    private Collection m_ids;

    private Group m_group;

    public BulkGroupAction(Group group) {
        m_group = group;
    }

    public Collection getIds() {
        return m_ids;
    }

    public void setIds(Collection ids) {
        m_ids = ids;
    }

    public Group getGroup() {
        return m_group;
    }

    public void setGroup(Group group) {
        m_group = group;
    }

    public Object getValue(Object option_, int index_) {
        return this;
    }

    public String getLabel(Object option_, int index_) {
        return m_group.getName();
    }

    public String squeezeOption(Object option_, int index_) {
        return getClass().getName() + m_group.getId();
    }

    public String getSuccessMsg(Messages messages) {
        return messages.getMessage("msg.actionSuccess");
    }

    public String getMethodName() {
        return null;
    }
}
