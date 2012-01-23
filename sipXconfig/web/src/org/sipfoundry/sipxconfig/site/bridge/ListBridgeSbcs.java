/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.bridge;

import java.util.ArrayList;
import java.util.List;

import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.site.sbc.EditSbcDevice;

public abstract class ListBridgeSbcs extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "bridge/ListBridgeSbcs";

    public abstract BridgeSbcRecord getCurrentRow();

    public abstract void setCurrentRow(BridgeSbcRecord e);

    public abstract void setSource(Object sbcs);

    @InjectObject("spring:sbcDeviceManager")
    public abstract SbcDeviceManager getSbcDeviceManager();

    public void pageBeginRender(PageEvent event) {
        List<BridgeSbcRecord> records = new ArrayList<BridgeSbcRecord>();
        for (BridgeSbc sbc : getSbcDeviceManager().getBridgeSbcs()) {
            records.add(new BridgeSbcRecord(sbc.getId(), sbc.getName(), sbc.getDescription(), sbc.getAddress()));
        }
        setSource(records);
    }

    public IPage editBridge(int id) {
        return EditSbcDevice.getEditPage(getPage().getRequestCycle(), id, getPage());
    }

}
