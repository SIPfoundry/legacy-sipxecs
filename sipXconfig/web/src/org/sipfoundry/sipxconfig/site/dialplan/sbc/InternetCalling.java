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

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.Sbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class InternetCalling extends BasePage implements PageBeginRenderListener {
    @InjectObject(value = "spring:sbcManager")
    public abstract SbcManager getSbcManager();

    @Persist
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

    public void save() {

    }

    public void addDomain() {
        if (TapestryUtils.isValid(this)) {
            getSbc().getRoutes().addDomain();
        }
    }

    public void addSubnet() {
        if (TapestryUtils.isValid(this)) {
            getSbc().getRoutes().addSubnet();
        }
    }

    public void deleteDomain(int index) {
        getSbc().getRoutes().removeDomain(index);
    }

    public void deleteSubnet(int index) {
        getSbc().getRoutes().removeSubnet(index);
    }
}
