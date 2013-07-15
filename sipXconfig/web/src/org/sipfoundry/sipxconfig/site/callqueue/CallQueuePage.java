/*
 * Copyright (C) 2013 SibTelCom, JSC., certain elements licensed under a Contributor Agreement.
 * Author: Konstantin S. Vishnivetsky
 * E-mail: info@siplabs.ru
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
*/

package org.sipfoundry.sipxconfig.site.callqueue;

import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;

/*sipXecs WEB components API imports */
import org.sipfoundry.sipxconfig.components.PageWithCallback;

public abstract class CallQueuePage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "callqueue/CallQueuePage";

    @Persist
    @InitialValue(value = "literal:queues")
    public abstract String getTab();
    public abstract void setTab(String id);

    public void editQueues() {
        setTab("queues");
    }

    public void editAgents() {
        setTab("agents");
    }

    public void editSettings() {
        setTab("settings");
    }

    @Override
    public void pageBeginRender(PageEvent event) {
    }
}
