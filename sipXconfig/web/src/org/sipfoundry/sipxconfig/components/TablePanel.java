/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IBinding;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.common.CoreContext;

public abstract class TablePanel extends BaseComponent {

    public abstract Collection getRowsToDelete();

    public abstract IActionListener getAction();

    public abstract CoreContext getCoreContext();

    /**
     * Overwrite to implement row removal
     *
     * @param selectedRows
     */
    protected abstract void removeRows(Collection selectedRows);

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this) && TapestryUtils.isValid(this)) {
            onFormSubmit(cycle);
        }
    }

    private boolean onFormSubmit(IRequestCycle cycle) {
        Collection selectedRows = getRowsToDelete();
        if (selectedRows != null) {
            removeRows(selectedRows);
            safeSetChanged();
            return true;
        }
        IActionListener action = getAction();
        if (action != null) {
            action.actionTriggered(this, cycle);
            return true;
        }
        return false;
    }

    /**
     * Sets changed to true - only if 'changed' parameter was provided
     */
    private void safeSetChanged() {
        IBinding changedBinding = getBinding("changed");
        if (changedBinding != null) {
            changedBinding.setObject(Boolean.TRUE);
        }
    }
}
