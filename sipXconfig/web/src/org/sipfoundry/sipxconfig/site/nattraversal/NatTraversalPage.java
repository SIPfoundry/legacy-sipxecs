/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.nattraversal;

import java.util.List;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;

import org.sipfoundry.sipxconfig.admin.dialplan.sbc.Sbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversal;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversalManager;
import org.sipfoundry.sipxconfig.site.dialplan.ActivateDialPlan;

public abstract class NatTraversalPage extends BasePage implements PageBeginRenderListener {

    public abstract NatTraversal getNatTraversal();

    public abstract void setNatTraversal(NatTraversal natTraversal);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject(value = "spring:sbcManager")
    public abstract SbcManager getSbcManager();

    @InjectObject(value = "spring:natTraversalManager")
    public abstract NatTraversalManager getNatTraversalManager();

    public abstract Sbc getSbc();

    public abstract void setSbc(Sbc sbc);

    public abstract List<String> getDomains();

    public abstract void setDomains(List<String> domains);

    public abstract List<String> getSubnets();

    public abstract void setSubnets(List<String> subnets);

    public abstract String getDomain();

    public abstract void setDomain(String domain);

    public abstract String getSubnet();

    public abstract void setSubnet(String subnet);

    public void pageBeginRender(PageEvent event_) {
        Sbc sbc = getSbc();
        if (sbc == null) {
            sbc = getSbcManager().loadDefaultSbc();
            setSbc(sbc);
            setDomains(sbc.getRoutes().getDomains());
            setSubnets(sbc.getRoutes().getSubnets());
        }
        NatTraversal natTraversal = getNatTraversal();
        if (natTraversal == null) {
            natTraversal = getNatTraversalManager().getNatTraversal();
            setNatTraversal(natTraversal);
        }

        if (getNatTraversal().isBehindnat()) {
            getNatTraversal().getInfoPublicAddress().setEnabled(true);
        } else {
            getNatTraversal().getInfoPublicAddress().setEnabled(false);
            getNatTraversal().getInfoPublicAddress().setValue("");
        }
    }

    public IPage activate(IRequestCycle cycle) {
        if (!TapestryUtils.isValid(this)) {
            return null;
        }
        getNatTraversalManager().store(getNatTraversal());
        ActivateDialPlan dialPlans = (ActivateDialPlan) cycle.getPage(ActivateDialPlan.PAGE);
        dialPlans.setReturnPage(this);
        return dialPlans;
    }

    public void formSubmit() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
    }
}
