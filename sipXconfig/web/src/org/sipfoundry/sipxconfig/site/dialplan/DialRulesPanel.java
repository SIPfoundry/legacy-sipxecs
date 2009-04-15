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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.components.selection.OptGroup;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.site.setting.BulkGroupAction;

/**
 * Component that shows all the dialing rules associated with a particular
 * gateway, with controls to add or remove rules from the gateway.
 */
public abstract class DialRulesPanel extends BaseComponent {

    @InjectObject(value = "spring:dialPlanContext")
    public abstract DialPlanContext getDialPlanContext();

    /**
     * Gets the {@code Gateway} being edited.
     */
    @Parameter(required = true)
    public abstract Gateway getGateway();

    /**
     * Gets the IDs of the rows that have been marked for deletion.
     */
    public abstract Collection<Integer> getRowsToDelete();

    /**
     * Sets the IDs of the rows that should be marked for deletion.
     * @param rowsToDelete the row IDs
     */
    public abstract void setRowsToDelete(Collection<Integer> rowsToDelete);

    /**
     * Gets the dialing rules that should be displayed in the table.
     */
    public List<DialingRule> getDialingRules() {
        DialPlanContext context = getDialPlanContext();
        return context.getRulesForGateway(getGateway().getId());
    }

    /**
     * Removes the selected dialing rule(s) from the gateway when the
     * "Remove" button is clicked in the UI.
     */
    public void removeRules() {
        DialPlanContext context = getDialPlanContext();
        Collection<Integer> ruleIds = getRowsToDelete();
        Gateway gateway = getGateway();
        Collection<Integer> gatewayId = Collections.singletonList(gateway.getId());

        for (Integer ruleId : ruleIds) {
            DialingRule rule = context.getRule(ruleId);
            if (rule != null) {
                rule.removeGateways(gatewayId);

                if (!rule.isInternal() && rule.getGateways().isEmpty()) {
                    rule.setEnabled(false);
                }
                context.storeRule(rule);
            }
        }
    }

    /**
     * Gets the {@code IPropertySelectionModel} for the drop-down list
     * containing the available dialing rules to add.
     */
    public IPropertySelectionModel getActionModel() {
        DialPlanContext context = getDialPlanContext();
        Collection<OptionAdapter> actions = new ArrayList<OptionAdapter>();
        Gateway gateway = getGateway();
        List<DialingRule> rules = context.getAvailableRules(gateway.getId());

        if (!rules.isEmpty()) {
            actions.add(new OptGroup(getMessages().getMessage("label.addDialingRule")));

            for (DialingRule rule : rules) {
                actions.add(new AddExistingDialingRuleAction(rule));
            }
        }

        AdaptedSelectionModel model = new AdaptedSelectionModel();
        model.setCollection(actions);

        return model;
    }

    /**
     * Adds an existing dialing rule to the gateway being edited.
     */
    class AddExistingDialingRuleAction extends BulkGroupAction {

        /** The {@code DialingRule} being added. */
        private final DialingRule m_rule;

        /**
         * Creates a new {@code AddExistingDialingRuleAction}.
         * @param rule the {@code DialingRule} being added.
         */
        AddExistingDialingRuleAction(DialingRule rule) {
            super(null);
            m_rule = rule;
        }

        /**
         * Gets the label (the rule name) to be displayed in the drop-down list.
         */
        @Override
        public String getLabel(Object option, int index) {
            return m_rule.getName();
        }

        /**
         * Gets the underlying value (the rule ID) to be used in the drop-down list.
         */
        @Override
        public Object getValue(Object option, int index) {
            return m_rule.getId();
        }

        @Override
        public String squeezeOption(Object option, int index) {
            return m_rule.getId().toString();
        }

        /**
         * Performs the action.
         * The gateway will be added to the selected dialing rule, and the
         * rule will be saved to the database.
         */
        public void actionTriggered(IComponent component, IRequestCycle cycle) {
            m_rule.addGateway(getGateway());
            getDialPlanContext().storeRule(m_rule);
        }
    }

}
