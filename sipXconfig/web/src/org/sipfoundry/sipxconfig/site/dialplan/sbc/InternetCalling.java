/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * 
 */
package org.sipfoundry.sipxconfig.site.dialplan.sbc;

import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.Sbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.dialplan.ActivateDialPlan;

public abstract class InternetCalling extends BasePage implements PageBeginRenderListener {
    @InjectObject(value = "spring:sbcManager")
    public abstract SbcManager getSbcManager();

    @InjectPage(value = ActivateDialPlan.PAGE)
    public abstract ActivateDialPlan getActivateDialPlan();

    public abstract Sbc getSbc();

    public abstract void setSbc(Sbc sbc);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public void pageBeginRender(PageEvent event_) {
        Sbc sbc = getSbc();
        if (sbc == null) {
            sbc = getSbcManager().loadDefaultSbc();
            setSbc(sbc);
        }
    }

    public IPage save() {
        if (!TapestryUtils.isValid(this)) {
            return null;
        }
        getSbcManager().saveDefaultSbc(getSbc());

        ActivateDialPlan dialPlans = getActivateDialPlan();
        dialPlans.setReturnPage(this);
        return dialPlans;
    }
}
