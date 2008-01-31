/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEndRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.web.WebRequest;
import org.sipfoundry.sipxconfig.admin.WaitingListener;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class WaitingPage extends UserBasePage implements PageEndRenderListener {
    public static final String PAGE = "admin/WaitingPage";

    @Asset("/images/loading.gif")
    public abstract IAsset getLoadingImage();

    @Persist
    public abstract WaitingListener getWaitingListener();

    public abstract void setWaitingListener(WaitingListener wl);

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    @InjectObject(value = "infrastructure:request")
    public abstract WebRequest getWebRequest();

    @InitialValue(value = "false")
    public abstract boolean isPageRendered();

    public abstract void setPageRendered(boolean pageRendered);

    /**
     * This method is called immediately after the waiting page (WaitingPage.html) is loaded in
     * the browser using Tacos mechanism
     */
    public void onLoad() {
        WaitingListener waitingListener = getWaitingListener();
        if (waitingListener != null) {
            waitingListener.afterResponseSent();
            setWaitingListener(null);
        }
    }

    public void pageEndRender(PageEvent e) {
        setPageRendered(true);
    }
}
