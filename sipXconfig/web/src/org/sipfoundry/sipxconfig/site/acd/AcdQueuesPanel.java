/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.acd;

import java.io.Serializable;
import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IBinding;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class AcdQueuesPanel extends BaseComponent {
    public abstract AcdContext getAcdContext();

    public abstract Collection getRowsToDelete();

    public abstract IActionListener getAction();

    public abstract Serializable getAcdAgentId();

    public abstract Collection getRowsToMoveUp();

    public abstract void setRowsToMoveUp(Collection rowsToMoveUp);

    public abstract Collection getRowsToMoveDown();

    public abstract void setRowsToMoveDown(Collection rowsToMoveDown);

    public abstract Collection getQueues();

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this) && TapestryUtils.isValid(this)) {
            onFormSubmit(cycle);
        }
    }

    private boolean onFormSubmit(IRequestCycle cycle) {
        Collection selectedRows = getRowsToDelete();
        if (selectedRows != null) {
            getAcdContext().removeQueues(selectedRows);
            markChanged();
            return true;
        }
        selectedRows = getRowsToMoveDown();
        int step = 0;
        if (selectedRows != null) {
            step = 1;
        } else {
            selectedRows = getRowsToMoveUp();
            if (selectedRows != null) {
                step = -1;
            }
        }
        if (step != 0) {
            getAcdContext().moveQueuesInAgent(getAcdAgentId(), selectedRows, step);
            markChanged();
            return true;
        }
        IActionListener action = getAction();
        if (action != null) {
            action.actionTriggered(this, cycle);
            return true;
        }
        return false;
    }

    private void markChanged() {
        IBinding changed = getBinding("changed");
        if (changed != null) {
            changed.setObject(Boolean.TRUE);
        }
    }

    public IPrimaryKeyConverter getConverter() {
        return new IPrimaryKeyConverter() {
            public Object getPrimaryKey(Object value) {
                BeanWithId bean = (BeanWithId) value;
                return bean.getId();
            }

            public Object getValue(Object primaryKey) {
                return getAcdContext().loadQueue((Serializable) primaryKey);
            }

        };
    }
}
