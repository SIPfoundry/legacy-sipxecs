/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.service;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.annotations.InjectObject;
import org.sipfoundry.sipxconfig.acccode.AccCodeContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivationManager;
import org.sipfoundry.sipxconfig.service.SipxAccCodeService;

public abstract class EditAccCodeService extends EditSipxService {
    @SuppressWarnings("hiding")
    public static final String PAGE = "service/EditAccCodeService";

    private static final Log LOG = LogFactory.getLog(EditAccCodeService.class);

    @InjectObject(value = "spring:accCodeContext")
    public abstract AccCodeContext getAccCodeContext();

    @InjectObject(value = "spring:dialPlanActivationManager")
    public abstract DialPlanActivationManager getDialPlanActivationManager();

    @Override
    public String getMyBeanId() {
        return SipxAccCodeService.BEAN_ID;
    }

    /*
     * Override apply method to manually replicate the dialplan so that the new Authentication Code
     * Prefix is set in the mapping rules.
     */
    @Override
    public void apply() {
        super.apply();
        getDialPlanActivationManager().replicateDialPlan(true);
    }
}
