/*
 * Copyright (C) 2013 SibTelCom, JSC., certain elements licensed under a Contributor Agreement.
 * Author: Konstantin S. Vishnivetsky
 * E-mail: info@siplabs.ru
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
*/

package org.sipfoundry.sipxconfig.site.callqueue;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

import org.sipfoundry.sipxconfig.site.user.UserTable;

import org.sipfoundry.sipxconfig.callqueue.CallQueueContext;
import org.sipfoundry.sipxconfig.callqueue.CallQueueAgent;

public abstract class CallQueueAgentSelectUser extends SipxBasePage implements PageBeginRenderListener {

    public static final String PAGE = "callqueue/CallQueueAgentSelectUser";

    /* Properties */

    public abstract void setCallQueueAgentId(Integer callqueueagentid);

    public abstract Integer getCallQueueAgentId();

    public abstract void setCallQueueAgent(CallQueueAgent callqueueagent);

    public abstract CallQueueAgent getCallQueueAgent();

    public abstract CoreContext getCoreContext();

    public abstract CallQueueContext getCallQueueContext();

    public abstract SipxValidationDelegate getValidator();

    public abstract void setReturnToEditAgent(boolean rtn);

    public abstract boolean getReturnToEditAgent();

    public IPage select(IRequestCycle cycle) {
        UserTable table = (UserTable) getComponent("searchResults");
        SelectMap selections = table.getSelections();
        if (selections.getAllSelected().size() > 0) {
            Integer selectedUserId = (Integer) selections.getAllSelected().toArray()[0];
            User selectedUser = getCoreContext().loadUser(selectedUserId);
            if (null != selectedUser) {
                getCallQueueAgent().setExtension(selectedUser.getUserName());
            }
        }
        CallQueueEditAgent editPage = (CallQueueEditAgent) cycle.getPage(CallQueueEditAgent.PAGE);
        editPage.setCallQueueAgent(getCallQueueAgent());
        editPage.setCallQueueAgentId(getCallQueueAgentId());
        return editPage;
    }

    public IPage cancel(IRequestCycle cycle) {
        CallQueueEditAgent editPage = (CallQueueEditAgent) cycle.getPage(CallQueueEditAgent.PAGE);
        return editPage;
    }

    public void pageBeginRender(PageEvent event_) {
        CallQueueAgent callQueueAgent = getCallQueueAgent();
        if (null == callQueueAgent) {
            setCallQueueAgent(getCallQueueContext().loadCallQueueAgent(getCallQueueAgentId()));
        }
    }
}
