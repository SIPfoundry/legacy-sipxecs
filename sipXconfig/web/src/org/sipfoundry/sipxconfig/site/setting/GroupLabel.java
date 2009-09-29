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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.site.phone.PhoneModels;

public abstract class GroupLabel extends BaseComponent {

    public abstract Group getGroup();

    public IPage viewGroup(IRequestCycle cycle, Integer groupId, String resourceId) {
        // HACK: Enumerate all possible group types, cannot think a better way w/o
        // introducing interfaces
        if (resourceId.equals("phone")) {
            PhoneModels page = (PhoneModels) cycle.getPage(PhoneModels.PAGE);
            page.setGroupId(groupId);
            return page;
        }
        return null;
    }
}
