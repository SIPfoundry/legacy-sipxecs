/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.html.BasePage;

public abstract class PageWithCallback extends BasePage {
    @Persist
    public abstract ICallback getCallback();

    public abstract void setCallback(ICallback callback);

    /**
     * Set a callback that will navigate back to the named return page on OK or Cancel.
     */
    public void setReturnPage(String returnPageName) {
        ICallback callback = new PageCallback(returnPageName);
        setCallback(callback);
    }

    public void setReturnPage(IPage returnPage) {
        ICallback callback = new PageCallback(returnPage);
        setCallback(callback);
    }
}
