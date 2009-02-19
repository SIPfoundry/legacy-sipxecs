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

import org.sipfoundry.sipxconfig.service.SipxRlsService;

public abstract class EditResourceListService extends EditSipxService {
    public static final String PAGE = "service/EditResourceListService";

    @Override
    protected String getBeanId() {
        return SipxRlsService.BEAN_ID;
    }
}
