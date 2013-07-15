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

/* Tapestry 4 page API imports */
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;

import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.ComponentClass;

import org.sipfoundry.sipxconfig.common.CoreContext;
/*sipXecs WEB components API imports */
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.SelectMap;

/*sipXecs WEB settings API imports */
import org.sipfoundry.sipxconfig.callqueue.CallQueueContext;
import org.sipfoundry.sipxconfig.callqueue.CallQueueAgent;

@ComponentClass
public abstract class CallQueueAgentsPanel extends BaseComponent {

    @Persist
    @InitialValue(value = "literal:agents")
    public abstract String getTab();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:callQueueContext")
    public abstract CallQueueContext getCallQueueContext();

    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectPage(CallQueueEditAgent.PAGE)
    public abstract CallQueueEditAgent getEditAgentPage();

    public abstract CallQueueAgent getCurrentRow();

    public abstract void setCurrentRow(CallQueueAgent callQueueAgent);

    public abstract Collection<Integer> getRowsToDuplicate();

    public abstract Collection<Integer> getRowsToDelete();

    @InitialValue(value = "new org.sipfoundry.sipxconfig.components.SelectMap()")
    public abstract SelectMap getSelections();

    public IPage add(IRequestCycle cycle) {
        CallQueueEditAgent editPage = (CallQueueEditAgent) cycle.getPage(CallQueueEditAgent.PAGE);
        editPage.setCallQueueAgent(null);
        editPage.setCallQueueAgentId(null);
        return editPage;
    }

    public IPage edit(IRequestCycle cycle, Integer agentId) {
        CallQueueEditAgent editPage = (CallQueueEditAgent) cycle.getPage(CallQueueEditAgent.PAGE);
        editPage.setCallQueueAgent(null);
        editPage.setCallQueueAgentId(agentId);
        return editPage;
    }

    public void duplicate() {
        Collection<Integer> selectedRows = getRowsToDuplicate();
        if (null != selectedRows) {
            getCallQueueContext().duplicateCallQueueAgents(selectedRows);
        }
    }

    public void delete() {
        Collection<Integer> selectedRows = getRowsToDelete();
        if (null != selectedRows) {
            getCallQueueContext().removeCallQueueAgents(selectedRows);
        }
    }
}
