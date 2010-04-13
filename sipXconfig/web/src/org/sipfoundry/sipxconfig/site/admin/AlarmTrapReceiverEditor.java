/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmTrapReceiver;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class AlarmTrapReceiverEditor extends BaseComponent {

    @Parameter(required = true)
    public abstract AlarmTrapReceiver getAlarmTrapReceiver();

    @Parameter(required = true)
    public abstract int getIndex();

    @Parameter(required = true)
    public abstract int getRemoveIndex();
}
