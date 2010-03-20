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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.callback.PageCallback;
import org.sipfoundry.sipxconfig.site.common.BreadCrumb;

public abstract class PageWithCallback extends SipxBasePage {
    @Persist
    public abstract ICallback getCallback();

    public abstract void setCallback(ICallback callback);

    @Persist
    public abstract List<BreadCrumb> getBreadCrumbsHistory();

    public abstract void setBreadCrumbsHistory(List<BreadCrumb> breadCrumbs);

    /**
     * Set a callback that will navigate back to the named return page on OK or Cancel.
     */
    public final void setReturnPage(String returnPageName) {
        ICallback callback = createCallback(returnPageName);
        setCallback(callback);
    }

    public final void setReturnPage(IPage returnPage) {
        ICallback callback = createCallback(returnPage.getPageName());
        setCallback(callback);
    }

    /**
     * Set a callback that will navigate back to the named return page on OK or Cancel and set the
     * associated bread crumb history
     */
    public final void setReturnPage(String returnPageName, List<BreadCrumb> breadCrumbs) {
        setReturnPage(returnPageName);
        setBreadCrumbsHistory(breadCrumbs);
    }

    public final void setReturnPage(IPage returnPage, List<BreadCrumb> breadCrumbs) {
        setReturnPage(returnPage);
        setBreadCrumbsHistory(breadCrumbs);
    }

    /**
     * Return the bread crumb history up until and including this page.
     */
    public final List<BreadCrumb> getBreadCrumbs() {
        BreadCrumb crumb = new BreadCrumb(getPageName(), getBreadCrumbTitle(), getMessages());
        List<BreadCrumb> history = getBreadCrumbsHistory();
        if (history == null) {
            return Arrays.asList(crumb);
        }
        List<BreadCrumb> breadCrumbs = new ArrayList<BreadCrumb>(history);
        breadCrumbs.add(crumb);
        return breadCrumbs;
    }

    /**
     * Override this method to set the title of this page. This title will be used in the
     * bread crumbs navigation
     */
    public String getBreadCrumbTitle() {
        return getPageName();
    }

    protected ICallback createCallback(String pageName) {
        return new PageCallback(pageName);
    }
}
