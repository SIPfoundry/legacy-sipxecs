/*
 * Copyright (C) 2013 SibTelCom, JSC., certain elements licensed under a Contributor Agreement.
 * Author: Konstantin S. Vishnivetsky
 * E-mail: info@siplabs.ru
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxconfig.site.callqueue;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.callqueue.CallQueueAgent;
import org.sipfoundry.sipxconfig.callqueue.CallQueueContext;
import org.sipfoundry.sipxconfig.callqueue.CallQueueTier;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class CallQueueEditAgent extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "callqueue/CallQueueEditAgent";
    private static final Log LOG = LogFactory.getLog(CallQueueEditAgent.class);

    /* Properties */
    @InjectObject("spring:callQueueContext")
    public abstract CallQueueContext getCallQueueContext();

    @Persist
    public abstract Integer getCallQueueAgentId();

    public abstract void setCallQueueAgentId(Integer id);

    @Persist
    public abstract CallQueueAgent getCallQueueAgent();

    public abstract void setCallQueueAgent(CallQueueAgent callQueueAgent);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    /* Tiers */

    public abstract CallQueueTier getCallQueueTier();

    public abstract void setCallQueueTier(CallQueueTier tier);

    public abstract int getIndex();

    public abstract void setIndex(int i);

    public void pageBeginRender(PageEvent event) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        CallQueueAgent callqueueagent = getCallQueueAgent();
        if (null != callqueueagent) {
            return;
        }

        Integer id = getCallQueueAgentId();
        if (null != id) {
            CallQueueContext context = getCallQueueContext();
            callqueueagent = context.loadCallQueueAgent(id);
        } else {
            callqueueagent = getCallQueueContext().newCallQueueAgent();
        }
        setCallQueueAgent(callqueueagent);

        if (getCallback() == null) {
            setReturnPage(CallQueuePage.PAGE);
        }
    }

    /* Action listeners */

    public void commit() {
        if (isValid()) {
            saveValid();
        }
    }

    private boolean isValid() {
        return TapestryUtils.isValid(this);
    }

    private void saveValid() {
        CallQueueContext context = getCallQueueContext();
        CallQueueAgent callQueueAgent = getCallQueueAgent();
        context.storeCallQueueAgent(callQueueAgent);
        Integer id = getCallQueueAgent().getId();
        setCallQueueAgent(null);
        setCallQueueAgentId(id);
    }

    // TODO: commit only on Apply or OK submit button pressed
    public IPage addQueues(IRequestCycle cycle) {
        CallQueueSelectPage queuesSelectPage = (CallQueueSelectPage) cycle.getPage(CallQueueSelectPage.PAGE);
        queuesSelectPage.setCallQueueAgentId(getCallQueueAgentId());
        return queuesSelectPage;
    }

    public IPage removeQueue(IRequestCycle cycle, Integer callqueueid) {
        CallQueueAgent callQueueAgent = getCallQueueAgent();
        callQueueAgent.removeFromQueue(callqueueid);
        CallQueueEditAgent editAgentPage = (CallQueueEditAgent) cycle.getPage(CallQueueEditAgent.PAGE);
        return editAgentPage;
    }

    public IPage setUser(IRequestCycle cycle) {
        CallQueueAgentSelectUser userSelectPage = (CallQueueAgentSelectUser) cycle
                .getPage(CallQueueAgentSelectUser.PAGE);
        userSelectPage.setCallQueueAgentId(getCallQueueAgentId());
        return userSelectPage;
    }
}
