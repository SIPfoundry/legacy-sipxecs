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
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.ComponentClass;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.UserException;
/*sipXecs WEB components API imports */
import org.sipfoundry.sipxconfig.components.SelectMap;

/*sipXecs WEB settings API imports */
import org.sipfoundry.sipxconfig.callqueue.CallQueue;
import org.sipfoundry.sipxconfig.callqueue.CallQueueContext;

@ComponentClass
public abstract class CallQueueQueuesPanel extends BaseComponent {

    @Persist
    @InitialValue(value = "literal:queues")
    public abstract String getTab();

    @InjectObject("spring:callQueueContext")
    public abstract CallQueueContext getCallQueueContext();

    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectPage(CallQueueEditQueue.PAGE)
    public abstract CallQueueEditQueue getEditQueuePage();

    public abstract CallQueue getCurrentRow();

    public abstract void setCurrentRow(CallQueue callQueue);

    public abstract Collection<Integer> getRowsToDuplicate();

    public abstract Collection<Integer> getRowsToDelete();

    @InitialValue(value = "new org.sipfoundry.sipxconfig.components.SelectMap()")
    public abstract SelectMap getSelections();

    public IPage add(IRequestCycle cycle) {
        CallQueueEditQueue editPage = (CallQueueEditQueue) cycle.getPage(CallQueueEditQueue.PAGE);
        editPage.setCallQueue(null);
        editPage.setCallQueueId(null);
        return editPage;
    }

    public IPage edit(IRequestCycle cycle, Integer callQueueId) {
        CallQueueEditQueue editPage = (CallQueueEditQueue) cycle.getPage(CallQueueEditQueue.PAGE);
        editPage.setCallQueue(null);
        editPage.setCallQueueId(callQueueId);
        return editPage;
    }

    public void duplicate() {
        Collection<Integer> selectedRows = getRowsToDuplicate();
        if (null != selectedRows) {
            getCallQueueContext().duplicateCallQueues(selectedRows);
        }
    }

    public void delete() {
        Collection<Integer> selectedRows = getRowsToDelete();
        if (null != selectedRows) {
            for (Integer callqueueid : selectedRows) {
                if (getCallQueueContext().getCallQueueAgentsForQueue(callqueueid).size() > 0) {
                    // TODO: display exception on page
                    throw new UserException(getMessages().getMessage("&error.callQueueBusy"));
                }
            }
            getCallQueueContext().removeCallQueues(selectedRows);
        }
    }
}
