/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.branch;

import java.util.Map;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.branch.BranchManager;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.search.SearchManager;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class BranchTable extends BaseComponent implements PageBeginRenderListener {

    @InjectObject("spring:branchManager")
    public abstract BranchManager getBranchManager();

    @InjectObject("spring:settingDao")
    public abstract SettingDao getSettingContext();

    @InjectObject("spring:searchManager")
    public abstract SearchManager getSearchManager();

    @Bean
    public abstract SelectMap getSelections();

    @Parameter(required = true)
    public abstract IActionListener getViewBranchListener();

    @Parameter
    public abstract String getSearchString();

    @Parameter
    public abstract boolean getSearchMode();

    public abstract Branch getCurrentRow();

    public abstract Map<Integer, Long> getUserCounts();

    public abstract void setUserCounts(Map<Integer, Long> counts);

    public abstract Map<Integer, Long> getGatewayCounts();

    public abstract void setGatewayCounts(Map<Integer, Long> counts);

    public abstract Map<Integer, Long> getSbcDeviceCounts();

    public abstract void setSbcDeviceCounts(Map<Integer, Long> counts);

    public abstract Map<Integer, Long> getServerCounts();

    public abstract void setServerCounts(Map<Integer, Long> counts);

    @Override
    public void pageBeginRender(PageEvent event) {
        if (getUserCounts() == null) {
            setUserCounts(getSettingContext().getAllBranchMemberCountIndexedByBranchId(User.class));
        }
        if (getServerCounts() == null) {
            setServerCounts(getSettingContext().getBranchMemberCountIndexedByBranchId(Location.class));
        }
        if (getGatewayCounts() == null) {
            setGatewayCounts(getSettingContext().getBranchMemberCountIndexedByBranchId(Gateway.class));
        }
        if (getSbcDeviceCounts() == null) {
            setSbcDeviceCounts(getSettingContext().getBranchMemberCountIndexedByBranchId(SbcDevice.class));
        }
    }

    public IBasicTableModel getBranchTableModel() {
        BranchTableModel branchTableModel = new BranchTableModel(getSearchManager(), getBranchManager());
        branchTableModel.setQueryText(getSearchString());
        branchTableModel.setSearchMode(getSearchMode());

        return branchTableModel;
    }

    public void deleteBranch() {
        BranchManager branchManager = getBranchManager();
        branchManager.deleteBranches(getSelections().getAllSelected());
    }
}
