/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.site.openacd;

import java.util.ArrayList;
import java.util.Collection;

import org.apache.commons.lang.ObjectUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.components.selection.OptGroupPropertySelectionRenderer;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;
import org.sipfoundry.sipxconfig.openacd.OpenAcdAgentGroup;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;

@ComponentClass
public abstract class OpenAcdAgentGroupSelect extends BaseComponent {

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    @Bean
    public abstract OptGroupPropertySelectionRenderer getRender();

    @Parameter(required = true)
    public abstract void setSelectedAgentGroup(OpenAcdAgentGroup selectedAgentGroup);

    public abstract OpenAcdAgentGroup getSelectedAgentGroup();

    @Parameter(defaultValue = "false")
    public abstract boolean getSubmitOnChange();

    public abstract void setSelectedAction(IActionListener selectedAction);

    public abstract IActionListener getSelectedAction();

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        if (getSelectedAgentGroup() != null) {
            setSelectedAction(new AddExistingAgentGroupAction(getSelectedAgentGroup()));
        } else {
            setSelectedAction(null);
        }

        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this) && TapestryUtils.isValid(this)) {
            triggerAction(cycle);
        }
    }

    private void triggerAction(IRequestCycle cycle) {
        IActionListener a = getSelectedAction();
        if (!(a instanceof OpenAcdAgentGroupAction)) {
            return;
        }

        OpenAcdAgentGroupAction action = (OpenAcdAgentGroupAction) a;
        OpenAcdAgentGroup agentGroup = action.getAgentGroup();
        if (agentGroup != null) {
            action.setId(agentGroup.getId());
        }
        action.actionTriggered(this, cycle);
    }

    public IPropertySelectionModel decorateModel(IPropertySelectionModel model) {
        return getTapestry().addExtraOption(model, getMessages(), "label.select");
    }

    public IPropertySelectionModel getModel() {
        Collection<OptionAdapter> actions = new ArrayList<OptionAdapter>();
        Collection<OpenAcdAgentGroup> agentGroups = getOpenAcdContext().getAgentGroups();
        if (!agentGroups.isEmpty()) {
            for (OpenAcdAgentGroup agentGroup : agentGroups) {
                AddExistingAgentGroupAction action = new AddExistingAgentGroupAction(agentGroup);
                actions.add(action);
            }
        }

        AdaptedSelectionModel model = new AdaptedSelectionModel();
        model.setCollection(actions);
        return model;
    }

    private class AddExistingAgentGroupAction extends OpenAcdAgentGroupAction {

        public AddExistingAgentGroupAction(OpenAcdAgentGroup agentGroup) {
            super(agentGroup);
        }

        @Override
        public void actionTriggered(IComponent component, final IRequestCycle cycle) {
            setSelectedAgentGroup(getAgentGroup());
        }

        @Override
        public Object getValue(Object option, int index) {
            return this;
        }

        @Override
        public String squeezeOption(Object option, int index) {
            return getAgentGroup().getId().toString();
        }

        @Override
        public boolean equals(Object obj) {
            return ObjectUtils.equals(this.getAgentGroup(), ((AddExistingAgentGroupAction) obj).getAgentGroup());
        }

        @Override
        public int hashCode() {
            return this.getAgentGroup().hashCode();
        }
    }
}
