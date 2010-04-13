/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import java.util.Collection;

import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.AbstractPage;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.callback.ICallback;

public class SelectUsersCallback implements ICallback {

    private final String m_pageName;

    private String m_idsPropertyName;

    private transient Collection m_ids;

    public SelectUsersCallback(IPage page) {
        this(page.getPageName());
    }

    public SelectUsersCallback(String pageName) {
        m_pageName = pageName;
    }

    public void setIds(Collection ids) {
        m_ids = ids;
    }

    public void setIdsPropertyName(String idsPropertyName) {
        m_idsPropertyName = idsPropertyName;
    }

    public void performCallback(IRequestCycle cycle) {
        AbstractPage page = (AbstractPage) cycle.getPage(m_pageName);
        PropertyUtils.write(page, m_idsPropertyName, m_ids);
        beforeActivation(page);
        cycle.activate(page);
    }

    /**
     * Overwrite to set additional page properties
     *
     * @param page_ to be activated
     */
    protected void beforeActivation(AbstractPage page_) {
        // do nothing by default
    }
}
