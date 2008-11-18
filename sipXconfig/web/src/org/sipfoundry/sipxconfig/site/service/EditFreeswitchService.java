/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.service;

import org.apache.tapestry.event.PageBeginRenderListener;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;

public abstract class EditFreeswitchService extends EditSipxService implements PageBeginRenderListener {
    public static final String PAGE = "service/EditFreeswitchService";

    @Override
    protected String getBeanId() {
        return SipxFreeswitchService.BEAN_ID;
    }
}
