/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import org.apache.hivemind.Messages;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.site.setting.BulkGroupAction;

public class RemoveFromUserGroupAction extends BulkGroupAction {
    private CoreContext m_coreContext;

    public RemoveFromUserGroupAction(Group group, CoreContext coreContext) {
        super(group);
        m_coreContext = coreContext;
    }

    public void actionTriggered(IComponent component_, IRequestCycle cycle_) {
        m_coreContext.removeFromGroup(getGroup().getId(), getIds());
    }

    public String getSuccessMsg(Messages messages) {
        return messages.format("msg.success.removeFromUserGroupAction", Integer.toString(getIds()
                .size()), getGroup().getName());
    }
}
