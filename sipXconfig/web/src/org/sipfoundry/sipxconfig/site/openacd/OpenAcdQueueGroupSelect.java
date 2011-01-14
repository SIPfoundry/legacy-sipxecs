/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
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
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.components.selection.OptGroupPropertySelectionRenderer;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdQueueGroup;

public abstract class OpenAcdQueueGroupSelect extends BaseComponent {

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    @Bean
    public abstract OptGroupPropertySelectionRenderer getRender();

    @Parameter(required = true)
    public abstract void setSelectedQueueGroup(OpenAcdQueueGroup selectedQueueGroup);

    public abstract OpenAcdQueueGroup getSelectedQueueGroup();

    public abstract void setSelectedAction(IActionListener selectedAction);

    public abstract IActionListener getSelectedAction();

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        if (getSelectedQueueGroup() != null) {
            setSelectedAction(new AddExistingQueueGroupAction(getSelectedQueueGroup()));
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
        if (!(a instanceof OpenAcdQueueGroupAction)) {
            return;
        }

        OpenAcdQueueGroupAction action = (OpenAcdQueueGroupAction) a;
        OpenAcdQueueGroup queueGroup = action.getQueueGroup();
        if (queueGroup != null) {
            action.setId(queueGroup.getId());
        }
        action.actionTriggered(this, cycle);
    }

    public IPropertySelectionModel decorateModel(IPropertySelectionModel model) {
        return getTapestry().addExtraOption(model, getMessages(), "label.select");
    }

    public IPropertySelectionModel getModel() {
        Collection<OptionAdapter> actions = new ArrayList<OptionAdapter>();
        Collection<OpenAcdQueueGroup> queueGroups = getOpenAcdContext().getQueueGroups();
        if (!queueGroups.isEmpty()) {
            for (OpenAcdQueueGroup queueGroup : queueGroups) {
                AddExistingQueueGroupAction action = new AddExistingQueueGroupAction(queueGroup);
                actions.add(action);
            }
        }

        AdaptedSelectionModel model = new AdaptedSelectionModel();
        model.setCollection(actions);
        return model;
    }

    private class AddExistingQueueGroupAction extends OpenAcdQueueGroupAction {

        public AddExistingQueueGroupAction(OpenAcdQueueGroup queueGroup) {
            super(queueGroup);
        }

        public void actionTriggered(IComponent component, final IRequestCycle cycle) {
            setSelectedQueueGroup(getQueueGroup());
        }

        @Override
        public Object getValue(Object option, int index) {
            return this;
        }

        @Override
        public String squeezeOption(Object option, int index) {
            return getQueueGroup().getId().toString();
        }

        @Override
        public boolean equals(Object obj) {
            return ObjectUtils.equals(this.getQueueGroup(), ((AddExistingQueueGroupAction) obj).getQueueGroup());
        }

        @Override
        public int hashCode() {
            return this.getQueueGroup().hashCode();
        }
    }
}
