/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.phone;

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.components.selection.OptGroupPropertySelectionRenderer;
import org.sipfoundry.sipxconfig.site.setting.BulkGroupAction;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class GroupActions extends BaseComponent {

    @InjectObject("spring:tapestry")
    public abstract TapestryContext getTapestry();

    @Bean
    public abstract OptGroupPropertySelectionRenderer getPropertyRenderer();

    @Parameter(required = true)
    public abstract IPropertySelectionModel getActionModel();

    @Parameter
    public abstract Collection getSelectedIds();

    public abstract IActionListener getSelectedAction();

    public abstract void setSelectedAction(IActionListener action);

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        setSelectedAction(null);
        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this) && TapestryUtils.isValid(this)) {
            triggerAction(cycle);
        }
    }

    public IPropertySelectionModel getDecoratedModel() {
        return getTapestry().addExtraOption(getActionModel(), getMessages(), "label.moreActions");
    }

    private void triggerAction(IRequestCycle cycle) {
        IActionListener a = getSelectedAction();
        if (!(a instanceof BulkGroupAction)) {
            return;
        }
        Collection selectedIds = getSelectedIds();
        if (selectedIds != null && selectedIds.size() == 0) {
            // nothing to do
            return;
        }

        BulkGroupAction action = (BulkGroupAction) a;
        action.setIds(selectedIds);

        action.actionTriggered(this, cycle);

        String successMsg = action.getSuccessMsg(getContainer().getMessages());
        TapestryUtils.recordSuccess(this, successMsg);
    }
}
