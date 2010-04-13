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
import java.util.HashMap;
import java.util.Map;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.TapestryUtils;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleFactory;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleType;

/**
 * SelectRuleType
 */
public abstract class SelectRuleType extends BaseComponent {
    public static final Map<DialingRuleType, String> TYPE_2_PAGE = new HashMap<DialingRuleType, String>();

    static {
        TYPE_2_PAGE.put(DialingRuleType.CUSTOM, EditDialRule.CUSTOM);
        TYPE_2_PAGE.put(DialingRuleType.INTERNAL, EditDialRule.INTERNAL);
        TYPE_2_PAGE.put(DialingRuleType.ATTENDANT, EditDialRule.ATTENDANT);
        TYPE_2_PAGE.put(DialingRuleType.LOCAL, EditDialRule.LOCAL);
        TYPE_2_PAGE.put(DialingRuleType.LONG_DISTANCE, EditDialRule.LONG_DISTANCE);
        TYPE_2_PAGE.put(DialingRuleType.EMERGENCY, EditDialRule.EMERGENCY);
        TYPE_2_PAGE.put(DialingRuleType.INTERNATIONAL, EditDialRule.INTERNATIONAL);
        TYPE_2_PAGE.put(DialingRuleType.TOLL_FREE, EditDialRule.LONG_DISTANCE);
        TYPE_2_PAGE.put(DialingRuleType.RESTRICTED, EditDialRule.LONG_DISTANCE);
        TYPE_2_PAGE.put(DialingRuleType.SITE_TO_SITE, EditDialRule.SITE_TO_SITE);
    }

    public abstract String getRuleBeanId();

    public abstract void setRuleBeanId(String ruleBeanId);

    public abstract DialingRuleFactory getDialingRuleFactory();

    public IPropertySelectionModel getRawRuleModel() {
        Collection<String> beanIds = getDialingRuleFactory().getBeanIds();
        String[] options = beanIds.toArray(new String[beanIds.size()]);
        return new StringPropertySelectionModel(options);
    }

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        if (cycle.isRewinding() && TapestryUtils.getForm(cycle, this).isRewinding()) {
            activateNewRulePage(cycle);
        }
    }

    public void activateNewRulePage(IRequestCycle cycle) {
        String beanId = getRuleBeanId();
        if (beanId == null) {
            // nothing to do
            return;
        }
        DialingRule rule = getDialingRuleFactory().create(beanId);
        EditDialRule editDialRulePage = getEditDialRulePage(cycle, rule.getType(), null);
        editDialRulePage.setRule(rule);
        editDialRulePage.setCallback(new PageCallback(getPage()));
        cycle.activate(editDialRulePage);
    }

    public static EditDialRule getEditDialRulePage(IRequestCycle cycle, DialingRuleType ruleType,
            Integer ruleId) {
        String pageName = TYPE_2_PAGE.get(ruleType);
        EditDialRule page = (EditDialRule) cycle.getPage(pageName);
        page.setRuleId(ruleId);
        page.setRuleType(ruleType);
        page.setRule(null);
        return page;
    }
}
