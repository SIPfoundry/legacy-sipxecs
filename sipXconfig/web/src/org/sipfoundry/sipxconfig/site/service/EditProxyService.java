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
import org.sipfoundry.sipxconfig.service.SipxProxyService;

public abstract class EditProxyService extends EditSipxService implements PageBeginRenderListener {
    public static final String PAGE = "service/EditProxyService";
    
    protected String getBeanId() {
        return SipxProxyService.BEAN_ID;
    }
}
