/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.setting.BulkGroupAction;

public abstract class GroupActions extends BaseComponent {
    public abstract Collection getSelectedIds();

    public abstract IActionListener getSelectedAction();

    public abstract void setSelectedAction(IActionListener action);

    public abstract TapestryContext getTapestry();

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        setSelectedAction(null);
        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this) && TapestryUtils.isValid(this)) {
            triggerAction(cycle);
        }
    }

    public IPropertySelectionModel decorateModel(IPropertySelectionModel model) {
        return getTapestry().addExtraOption(model, getMessages(), "label.moreActions");
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
