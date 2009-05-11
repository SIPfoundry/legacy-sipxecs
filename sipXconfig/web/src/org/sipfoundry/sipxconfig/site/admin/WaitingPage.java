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
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEndRenderListener;
import org.sipfoundry.sipxconfig.admin.WaitingListener;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class WaitingPage extends UserBasePage implements PageEndRenderListener {
    public static final String PAGE = "admin/WaitingPage";

    @Asset("/images/loading.gif")
    public abstract IAsset getLoadingImage();

    @Asset("context:/WEB-INF/admin/WaitingPage.script")
    public abstract IAsset getWaitingScript();

    @Persist
    public abstract WaitingListener getWaitingListener();

    public abstract void setWaitingListener(WaitingListener wl);

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    /**
     * This method is called immediately after the waiting page (WaitingPage.html) is loaded in
     * the browser. XTile used to trigger this listener.
     */
    public void handleWaitingListener() {
        WaitingListener waitingListener = getWaitingListener();
        if (waitingListener != null) {
            waitingListener.afterResponseSent();
        }
    }
}
