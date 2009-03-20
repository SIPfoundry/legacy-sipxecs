/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.service;

import org.sipfoundry.sipxconfig.service.SipxRelayService;

public abstract class EditRelayService extends EditSipxService {
    public static final String PAGE = "service/EditRelayService";

    protected String getBeanId() {
        return SipxRelayService.BEAN_ID;
    }
}
