/*
 * Copyright (C) 2013 SibTelCom, JSC., certain elements licensed under a Contributor Agreement.
 * Author: Konstantin S. Vishnivetsky
 * E-mail: info@siplabs.ru
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
*/

package org.sipfoundry.sipxconfig.site.callqueue;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;

import org.sipfoundry.sipxconfig.callqueue.CallQueueContext;
import org.sipfoundry.sipxconfig.callqueue.CallQueueSettings;

@ComponentClass
public abstract class CallQueueEditSettingsPanel
    extends BaseComponent
    implements PageBeginRenderListener {

    @Persist
    @InitialValue(value = "literal:settings")
    public abstract String getTab();

    @Persist
    public abstract CallQueueSettings getSettings();

    public abstract void setSettings(CallQueueSettings settings);

    public abstract CallQueueContext getCallQueueContext();

    @Override
    public void pageBeginRender(PageEvent event) {
        if (null == getSettings()) {
            setSettings(getCallQueueContext().getSettings());
        }
    }

    public void apply() {
        if (null != getCallQueueContext()) {
            if (null != getSettings()) {
                getCallQueueContext().saveSettings(getSettings());
            }
        }
    }

}
