/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.site.common;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;

@ComponentClass(allowBody = true, allowInformalParameters = false)
public abstract class AutoRefreshForm extends BaseComponent implements PageBeginRenderListener {
    @Parameter(defaultValue = "30")
    public abstract int getInterval();

    @Persist
    public abstract int getCurrentInterval();

    public abstract void setCurrentInterval(int interval);

    public void pageBeginRender(PageEvent event) {
        if (0 == getCurrentInterval()) {
            setCurrentInterval(getInterval());
        }
    }
}
