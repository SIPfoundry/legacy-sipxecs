/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.park;

import java.util.Collection;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbit;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.parkorbit.ParkSettings;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.site.setting.GroupSettings;

public abstract class ListParkOrbits extends SipxBasePage implements PageBeginRenderListener {

    public static final String PAGE = "park/ListParkOrbits";

    public abstract ParkOrbitContext getParkOrbitContext();

    public abstract CoreContext getCoreContext();

    public abstract ParkOrbit getCurrentRow();

    public abstract void setCurrentRow(ParkOrbit parkOrbit);

    public abstract Collection getRowsToDelete();

    public abstract ParkSettings getSettings();

    public abstract void setSettings(ParkSettings settings);

    @Persist
    @InitialValue(value = "literal:orbits")
    public abstract String getTab();

    @Override
    public void pageBeginRender(PageEvent arg0) {
        if (getSettings() == null) {
            setSettings(getParkOrbitContext().getSettings());
        }
    }

    public IPage add(IRequestCycle cycle) {
        EditParkOrbit editPage = (EditParkOrbit) cycle.getPage(EditParkOrbit.PAGE);
        editPage.setParkOrbitId(null);
        editPage.setParkOrbit(null);
        return editPage;
    }

    public IPage edit(IRequestCycle cycle) {
        EditParkOrbit editPage = (EditParkOrbit) cycle.getPage(EditParkOrbit.PAGE);
        Integer callGroupId = TapestryUtils.getBeanId(cycle);
        editPage.setParkOrbitId(callGroupId);
        editPage.setParkOrbit(null);
        return editPage;
    }

    public void apply(IRequestCycle cycle) {
        if (!TapestryUtils.validateFDSoftAndHardLimits(this, getSettings(), "resource-limits")) {
            return;
        }
        getParkOrbitContext().saveSettings(getSettings());
    }

    public void formSubmit() {
        delete();
    }

    /**
     * Deletes all selected rows (on this screen deletes call groups).
     */
    private void delete() {
        Collection selectedRows = getRowsToDelete();
        if (null != selectedRows) {
            getParkOrbitContext().removeParkOrbits(selectedRows);
        }
    }

    public IPage defaultGroup(IRequestCycle cycle) {
        // get new orbit and its default group
        BeanWithGroups po = getParkOrbitContext().newParkOrbit();
        Group group = po.getGroups().iterator().next();
        GroupSettings page = (GroupSettings) cycle.getPage(EditParkOrbitDefaults.PAGE);
        page.setReturnPage(PAGE);
        page.editGroup(group.getId(), po, PAGE);
        return page;
    }
}
