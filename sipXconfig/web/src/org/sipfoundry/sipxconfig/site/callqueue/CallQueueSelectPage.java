/*
 * Copyright (C) 2013 SibTelCom, JSC., certain elements licensed under a Contributor Agreement.
 * Author: Konstantin S. Vishnivetsky
 * E-mail: info@siplabs.ru
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxconfig.site.callqueue;

import java.util.Collection;

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;

import org.sipfoundry.sipxconfig.components.SelectMap;

/*sipXecs WEB components API imports */
import org.sipfoundry.sipxconfig.components.SipxBasePage;

/*sipXecs WEB settings API imports */
import org.sipfoundry.sipxconfig.callqueue.CallQueue;
import org.sipfoundry.sipxconfig.callqueue.CallQueueAgent;
import org.sipfoundry.sipxconfig.callqueue.CallQueueTiers;
import org.sipfoundry.sipxconfig.callqueue.CallQueueContext;

public abstract class CallQueueSelectPage extends SipxBasePage implements PageBeginRenderListener {

    public static final String PAGE = "callqueue/CallQueueSelectPage";

    /* Properties */

    public abstract void setCallQueueAgentId(Integer callqueueagentid);

    public abstract Integer getCallQueueAgentId();

    public abstract void setCallQueueAgent(CallQueueAgent callqueueagent);

    public abstract CallQueueAgent getCallQueueAgent();

    @InjectObject("spring:callQueueContext")
    public abstract CallQueueContext getCallQueueContext();

    public abstract CallQueue getCurrentRow();

    @InitialValue(value = "new org.sipfoundry.sipxconfig.components.SelectMap()")
    public abstract SelectMap getSelections();

    @Override
    public void pageBeginRender(PageEvent event) {
        CallQueueAgent callQueueAgent = getCallQueueAgent();
        if (null == callQueueAgent) {
            setCallQueueAgent(getCallQueueContext().loadCallQueueAgent(getCallQueueAgentId()));
        }
    }

    public IPage select(IRequestCycle cycle) {
        Collection<Integer> selectedRows = getSelections().getAllSelected();
        CallQueueTiers tiers = getCallQueueAgent().getTiers();
        if (null != selectedRows) {
            for (Integer callqeueuid : selectedRows) {
                tiers.addTier(callqeueuid, getCallQueueAgent().getId());
            }
        }

        CallQueueEditAgent editPage = (CallQueueEditAgent) cycle.getPage(CallQueueEditAgent.PAGE);
        editPage.setCallQueueAgent(getCallQueueAgent());
        editPage.setCallQueueAgentId(getCallQueueAgent().getId());
        return editPage;
    }

    public IPage cancel(IRequestCycle cycle) {
        return cycle.getPage(CallQueueEditAgent.PAGE);
    }
}
