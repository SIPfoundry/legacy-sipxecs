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

import org.sipfoundry.sipxconfig.service.SipxStatusService;

public abstract class EditStatusService extends EditSipxService {
    public static final String PAGE = "service/EditStatusService";
    
    @Override
    protected String getBeanId() {
        return SipxStatusService.BEAN_ID;
    }
}
