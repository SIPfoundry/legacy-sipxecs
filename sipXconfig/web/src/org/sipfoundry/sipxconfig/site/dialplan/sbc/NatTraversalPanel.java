/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan.sbc;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;

import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversal;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversalManager;
import org.sipfoundry.sipxconfig.site.dialplan.ActivateDialPlan;

@ComponentClass
public abstract class NatTraversalPanel extends BaseComponent implements PageBeginRenderListener {

    public abstract NatTraversal getNatTraversal();

    public abstract void setNatTraversal(NatTraversal natTraversal);

    @InjectObject(value = "spring:natTraversalManager")
    public abstract NatTraversalManager getNatTraversalManager();

    @InjectObject(value = "spring:sbcDeviceManager")
    public abstract SbcDeviceManager getSbcDeviceManager();

    public void pageBeginRender(PageEvent event_) {
        NatTraversal natTraversal = getNatTraversal();
        if (natTraversal == null) {
            natTraversal = getNatTraversalManager().getNatTraversal();
            setNatTraversal(natTraversal);
        }
    }

    public IPage activateNatTraversal(IRequestCycle cycle) {
        if (!TapestryUtils.isValid(this)) {
            return null;
        }

        getNatTraversalManager().store(getNatTraversal());

        ActivateDialPlan dialPlans = (ActivateDialPlan) cycle.getPage(ActivateDialPlan.PAGE);
        dialPlans.setReturnPage(InternetCalling.PAGE);
        return dialPlans;
    }
}
