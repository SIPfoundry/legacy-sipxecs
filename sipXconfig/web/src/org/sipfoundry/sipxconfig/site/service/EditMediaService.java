/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.service;

import org.sipfoundry.sipxconfig.service.SipxMediaService;

public abstract class EditMediaService extends EditSipxService {

    public static final String PAGE = "service/EditMediaService";

    protected String getBeanId() {
        return SipxMediaService.BEAN_ID;
    }
}
