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
import java.util.Iterator;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdProvisioningContext;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class AcdServersPanel extends BaseComponent {
    public abstract AcdContext getAcdContext();

    public abstract AcdProvisioningContext getAcdProvisioningContext();

    public abstract Collection getRowsToDeploy();

    public abstract IActionListener getAction();

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this) && TapestryUtils.isValid(this)) {
            onFormSubmit(cycle);
        }
    }

    private boolean onFormSubmit(IRequestCycle cycle) {
        Collection selectedRows = getRowsToDeploy();
        if (selectedRows != null) {
            for (Iterator i = selectedRows.iterator(); i.hasNext();) {
                Serializable serverId = (Serializable) i.next();
                getAcdProvisioningContext().deploy(serverId);
            }
            String msg = getMessages().format("msg.success.deploy",
                    Integer.toString(selectedRows.size()));
            TapestryUtils.recordSuccess(getPage(), msg);
            return true;
        }
        IActionListener action = getAction();
        if (action != null) {
            action.actionTriggered(this, cycle);
            return true;
        }
        return false;
    }

    public IPrimaryKeyConverter getConverter() {
        return new IPrimaryKeyConverter() {
            public Object getPrimaryKey(Object value) {
                BeanWithId bean = (BeanWithId) value;
                return bean.getId();
            }

            public Object getValue(Object primaryKey) {
                return getAcdContext().loadServer((Serializable) primaryKey);
            }

        };
    }
}
