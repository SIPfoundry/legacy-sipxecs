/*
 * Copyright (C) 2013 SibTelCom, JSC., certain elements licensed under a Contributor Agreement.
 * Author: Konstantin S. Vishnivetsky
 * E-mail: info@siplabs.ru
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
*/

package org.sipfoundry.sipxconfig.site.callqueue;

/* Tapestry 4 page API imports */
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;

/*sipXecs WEB components API imports */
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

/*sipXecs WEB settings API imports */
import org.sipfoundry.sipxconfig.callqueue.CallQueue;
import org.sipfoundry.sipxconfig.callqueue.CallQueueContext;

public abstract class CallQueueEditQueue extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "callqueue/CallQueueEditQueue";

    /* Properties */
    @InjectObject("spring:callQueueContext")
    public abstract CallQueueContext getCallQueueContext();

    @Persist
    public abstract Integer getCallQueueId();

    public abstract void setCallQueueId(Integer id);

    @Persist
    public abstract CallQueue getCallQueue();

    public abstract void setCallQueue(CallQueue callQueue);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    /*  Methods */

    public void pageBeginRender(PageEvent event) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        CallQueue callqueue = getCallQueue();
        if (null != callqueue) {
            return;
        }

        Integer id = getCallQueueId();
        if (null != id) {
            CallQueueContext context = getCallQueueContext();
            callqueue = context.loadCallQueue(id);
        } else {
            callqueue = getCallQueueContext().newCallQueue();
        }
        setCallQueue(callqueue);

        if (getCallback() == null) {
            setReturnPage(CallQueuePage.PAGE);
        }
    }

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
        CallQueue callQueue = getCallQueue();
        context.storeCallQueue(callQueue);
        Integer id = getCallQueue().getId();
        setCallQueueId(id);
    }
}
