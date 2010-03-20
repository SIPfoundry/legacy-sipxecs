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
import java.util.List;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleType;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.forwarding.GeneralSchedule;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.site.setting.EditSchedule;

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class EditFlexibleDialPlan extends SipxBasePage {
    public static final String PAGE = "dialplan/EditFlexibleDialPlan";

    @InjectObject(value = "spring:dialPlanContext")
    public abstract DialPlanContext getDialPlanContext();

    @InjectObject(value = "spring:forwardingContext")
    public abstract ForwardingContext getForwardingContext();

    @Persist
    @InitialValue(value = "literal:dialingRules")
    public abstract String getTab();

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract void setCurrentRow(DialingRule plan);

    public abstract Collection getSelectedRows();

    public abstract Collection getRowsToDuplicate();

    public abstract Collection getRowsToMoveUp();

    public abstract Collection getRowsToMoveDown();

    public abstract boolean getChanged();

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

    public List<GeneralSchedule> getGeneralSchedules() {
        return getForwardingContext().getAllGeneralSchedules();
    }

    public IPage editSchedule(IRequestCycle cycle, Integer scheduleId) {
        EditSchedule page = (EditSchedule) cycle.getPage(EditSchedule.PAGE);
        page.editSchedule(scheduleId, PAGE);
        return page;
    }

    public IPage addSchedule(IRequestCycle cycle) {
        EditSchedule page = (EditSchedule) cycle.getPage(EditSchedule.PAGE);
        page.setUserId(null);
        page.setUserGroup(null);
        page.newSchedule("general_sch", PAGE);
        return page;
    }
}
