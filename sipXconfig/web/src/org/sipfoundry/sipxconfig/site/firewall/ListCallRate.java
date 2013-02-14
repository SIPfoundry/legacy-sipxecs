/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.site.firewall;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.firewall.CallRateManager;
import org.sipfoundry.sipxconfig.firewall.CallRateRule;

public abstract class ListCallRate extends BaseComponent implements PageBeginRenderListener {

    @InjectObject(value = "spring:callRateManager")
    public abstract CallRateManager getCallRateManager();

    public abstract void setCallRates(List<CallRateRule> rates);

    public abstract List<CallRateRule> getFilters();

    public abstract CallRateRule getCurrentRow();

    public abstract void setCurrentRow(CallRateRule rule);

    public abstract Collection getSelectedRows();

    public abstract Collection getRowsToMoveUp();

    public abstract Collection getRowsToMoveDown();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Bean
    public abstract SelectMap getSelections();

    public void pageBeginRender(PageEvent event) {
        List<CallRateRule> rules = getCallRateManager().getCallRateRules();
        setCallRates(rules);
    }

    public IPage addCallRate(IRequestCycle cycle) {
        EditCallRate editPage = (EditCallRate) cycle.getPage(EditCallRate.PAGE);
        editPage.setRuleId(null);
        editPage.setRule(null);
        editPage.setLimits(null);
        editPage.setReturnPage(getPage().getPageName());
        return editPage;
    }

    public IPage editCallRate(IRequestCycle cycle) {
        EditCallRate editPage = (EditCallRate) cycle.getPage(EditCallRate.PAGE);
        Integer ruleId = TapestryUtils.getBeanId(cycle);
        editPage.setRuleId(ruleId);
        editPage.setRule(null);
        editPage.setLimits(null);
        editPage.setReturnPage(getPage().getPageName());
        return editPage;
    }

    public void formSubmit() {
        move(getRowsToMoveUp(), -1);
        move(getRowsToMoveDown(), 1);
        deleteRates();
    }

    public Collection getAllSelected() {
        return getSelections().getAllSelected();
    }

    private void move(Collection rows, int step) {
        if (null == rows) {
            return;
        }
        List<CallRateRule> rules = getCallRateManager().getCallRateRules();
        DataCollectionUtil.moveByPrimaryKey(rules, rows.toArray(), step);
        getCallRateManager().saveCallRateRules(rules);
    }

    private void deleteRates() {
        Collection selectedRows = getSelectedRows();
        if (null != selectedRows) {
            List<CallRateRule> rules = new ArrayList<CallRateRule>();
            for (Iterator iterator = selectedRows.iterator(); iterator.hasNext();) {
                rules.add(getCallRateManager().getCallRateRule((Integer) iterator.next()));
            }
            getCallRateManager().deleteCallRateRules(rules);
        }
    }
}
