/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.gateway;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IForm;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.TapestryUtils;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.components.selection.OptGroup;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.GatewayModel;
import org.sipfoundry.sipxconfig.site.setting.BulkGroupAction;

public abstract class GatewaysPanel extends BaseComponent {

    public abstract Collection<Integer> getRowsToDelete();

    public abstract Collection<Integer> getRowsToMoveUp();

    public abstract Collection<Integer> getRowsToMoveDown();

    public abstract DialingRule getRule();

    public abstract GatewayContext getGatewayContext();

    public abstract DialPlanContext getDialPlanContext();

    public abstract ModelSource<GatewayModel> getGatewayModelSource();

    public abstract Gateway getGateway();

    public abstract void setGateway(Gateway gateway);

    public abstract void setRuleChanged(boolean changed);

    public abstract void setGatewaysToAdd(Collection<Integer> gatewaysToAdd);

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        if (cycle.isRewinding() && TapestryUtils.getForm(cycle, this).isRewinding()) {
            if (onFormSubmit()) {
                setRuleChanged(true);
            }
        }
    }

    public IPropertySelectionModel getActionModel() {
        GatewayContext context = getGatewayContext();
        Collection<OptionAdapter> actions = new ArrayList<OptionAdapter>();
        DialingRule rule = getRule();

        Collection<Gateway> gateways;
        if (rule.isNew()) {
            gateways = context.getGateways();
        } else {
            gateways = context.getAvailableGateways(getRule().getId());
        }
        if (!gateways.isEmpty()) {
            actions.add(new OptGroup(getMessages().getMessage("label.addGateway")));

            for (Gateway gateway : gateways) {
                AddExistingGatewayAction action = new AddExistingGatewayAction(gateway);
                actions.add(action);
            }
        }

        Collection<GatewayModel> models = getGatewayModelSource().getModels();
        actions.add(new OptGroup(getMessages().getMessage("label.createGateway")));
        for (GatewayModel model : models) {
            AddNewGatewayAction action = new AddNewGatewayAction(model);
            actions.add(action);
        }

        AdaptedSelectionModel model = new AdaptedSelectionModel();
        model.setCollection(actions);
        return model;
    }

    class AddNewGatewayAction extends BulkGroupAction {
        private GatewayModel m_model;

        AddNewGatewayAction(GatewayModel model) {
            super(null);
            m_model = model;
        }

        public void actionTriggered(IComponent component, final IRequestCycle cycle) {
            // mark rule to be changed
            setRuleChanged(true);

            // defer activating gateway page
            Runnable action = new Runnable() {
                public void run() {
                    DialingRule rule = getRule();
                    getDialPlanContext().storeRule(rule);
                    EditGateway page = EditGateway.getAddPage(cycle, m_model, cycle.getPage(),
                            rule.getId());
                    cycle.activate(page);
                }
            };

            IForm form = TapestryUtils.getForm(cycle, component);
            form.addDeferredRunnable(action);
        }

        public String getLabel(Object option, int index) {
            return m_model.getLabel();
        }

        public Object getValue(Object option, int index) {
            return m_model;
        }

        public String squeezeOption(Object option, int index) {
            return m_model.getModelId() + m_model.getBeanId();
        }
    }

    class AddExistingGatewayAction extends BulkGroupAction {
        private Integer m_gatewayId;
        private String m_label;

        AddExistingGatewayAction(Gateway gateway) {
            super(null);
            m_gatewayId = gateway.getId();
            m_label = gateway.getName();
        }

        public String getLabel(Object option, int index) {
            return m_label;
        }

        public Object getValue(Object option, int index) {
            return m_gatewayId;
        }

        public String squeezeOption(Object option, int index) {
            return m_gatewayId.toString();
        }

        public void actionTriggered(IComponent component, IRequestCycle cycle) {
            setGatewaysToAdd(Collections.singleton(m_gatewayId));
            setRuleChanged(true);
        }
    }

    private boolean onFormSubmit() {
        DialingRule rule = getRule();
        Collection<Integer> selectedGateways = getRowsToDelete();
        if (null != selectedGateways) {
            rule.removeGateways(selectedGateways);
            return true;
        }
        selectedGateways = getRowsToMoveDown();
        if (null != selectedGateways) {
            rule.moveGateways(selectedGateways, 1);
            return true;
        }
        selectedGateways = getRowsToMoveUp();
        if (null != selectedGateways) {
            rule.moveGateways(selectedGateways, -1);
            return true;
        }
        return false;
    }
}
