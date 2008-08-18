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

import org.apache.tapestry.annotations.InjectObject;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.service.SipxPresenceService;

public abstract class EditPresenceService extends EditSipxService {
    public static final String PAGE = "service/EditPresenceService";

    @InjectObject(value = "spring:sipxReplicationContext")
    public abstract SipxReplicationContext getSipxReplicationContext();
    
    protected String getBeanId() {
        return SipxPresenceService.BEAN_ID;
    }
    
    @Override
    public void apply() {
        super.apply();
        getSipxReplicationContext().generate(DataSet.ALIAS);
    }
}
