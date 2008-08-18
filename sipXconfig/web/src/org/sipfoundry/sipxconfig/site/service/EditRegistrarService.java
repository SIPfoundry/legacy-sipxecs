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

import org.sipfoundry.sipxconfig.service.SipxRegistrarService;

public abstract class EditRegistrarService extends EditSipxService {
    public static final String PAGE = "service/EditRegistrarService";

    protected String getBeanId() {
        return SipxRegistrarService.BEAN_ID;
    }
}
