/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import org.apache.hivemind.Messages;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.site.setting.BulkGroupAction;

public class AddToPhoneGroupAction extends BulkGroupAction {
    private PhoneContext m_phoneContext;

    public AddToPhoneGroupAction(Group group, PhoneContext phoneContext) {
        super(group);
        m_phoneContext = phoneContext;
    }

    public void actionTriggered(IComponent component_, IRequestCycle cycle_) {
        m_phoneContext.addToGroup(getGroup().getId(), getIds());
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public String getSuccessMsg(Messages messages) {
        return messages.format("msg.success.addToPhoneGroupAction", Integer.toString(getIds()
                .size()), getGroup().getName());
    }
}
