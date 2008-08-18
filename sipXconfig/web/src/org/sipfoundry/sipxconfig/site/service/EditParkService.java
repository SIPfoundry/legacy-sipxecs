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

import org.sipfoundry.sipxconfig.service.SipxParkService;


public abstract class EditParkService extends EditSipxService {
    public static final String PAGE = "service/EditParkService";
    
    protected String getBeanId() {
        return SipxParkService.BEAN_ID;
    }
}
