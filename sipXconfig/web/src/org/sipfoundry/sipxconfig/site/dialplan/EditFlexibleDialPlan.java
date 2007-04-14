/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import java.util.Collection;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleType;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class EditFlexibleDialPlan extends BasePage {
    public static final String PAGE = "EditFlexibleDialPlan";

    @InjectObject(value = "spring:dialPlanContext")
    public abstract DialPlanContext getDialPlanContext();

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract void setCurrentRow(DialingRule plan);

    public abstract Collection getSelectedRows();

    public abstract Collection getRowsToDuplicate();

    public abstract Collection getRowsToMoveUp();

    public abstract Collection getRowsToMoveDown();

    public IPage edit(IRequestCycle cycle, Integer ruleId) {
        DialingRule rule = getDialPlanContext().getRule(ruleId);
        DialingRuleType ruleType = rule.getType();
        return SelectRuleType.getEditDialRulePage(cycle, ruleType, ruleId);
    }

    public void formSubmit() {
        move(getRowsToMoveUp(), -1);
        move(getRowsToMoveDown(), 1);
        delete();
        duplicate();
    }

    /**
     * Moves selected dialing rules up and down.
     * 
     * Pressing moveUp/moveDown buttons sets moveStep property. selectedRows property represents
     * all the rows with checked check boxes.
     */
    private void move(Collection rows, int step) {
        if (null == rows) {
            return;
        }
        DialPlanContext manager = getDialPlanContext();
        manager.moveRules(rows, step);
    }

    public IPage activate(IRequestCycle cycle) {
        DialPlanContext manager = getDialPlanContext();
        manager.generateDialPlan();
        ActivateDialPlan activate = (ActivateDialPlan) cycle.getPage(ActivateDialPlan.PAGE);
        activate.setReturnPage(PAGE);
        return activate;
    }

    public IPage revert(IRequestCycle cycle) {
        ResetDialPlan reset = (ResetDialPlan) cycle.getPage(ResetDialPlan.PAGE);
        reset.setReturnPage(PAGE);
        return reset;
    }

    /**
     * Deletes all selected rows (on this screen deletes rules from flexible dial plan).
     */
    private void delete() {
        Collection selectedRows = getSelectedRows();
        if (null != selectedRows) {
            getDialPlanContext().deleteRules(selectedRows);
        }
    }

    /**
     * Deletes all selected rows (on this screen deletes rules from flexible dial plan).
     */
    private void duplicate() {
        Collection selectedRows = getRowsToDuplicate();
        if (null != selectedRows) {
            getDialPlanContext().duplicateRules(selectedRows);
        }
    }

    public Collection getAllSelected() {
        return getSelections().getAllSelected();
    }
}
