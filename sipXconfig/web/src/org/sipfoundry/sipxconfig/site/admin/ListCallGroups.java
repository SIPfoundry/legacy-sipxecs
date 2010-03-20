/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Collection;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class ListCallGroups extends SipxBasePage {

    public static final String PAGE = "admin/ListCallGroups";

    public abstract CallGroupContext getCallGroupContext();

    public abstract CoreContext getCoreContext();

    public abstract CallGroup getCurrentRow();

    public abstract void setCurrentRow(CallGroup cd);

    public abstract Collection getRowsToDelete();

    public abstract Collection getRowsToDuplicate();

    public IPage add(IRequestCycle cycle) {
        EditCallGroup editCallGroup = (EditCallGroup) cycle.getPage(EditCallGroup.PAGE);
        editCallGroup.setCallGroupId(null);
        editCallGroup.setCallGroup(null);
        return editCallGroup;
    }

    public IPage edit(IRequestCycle cycle) {
        EditCallGroup editCallGroup = (EditCallGroup) cycle.getPage(EditCallGroup.PAGE);
        Integer callGroupId = TapestryUtils.getBeanId(cycle);
        editCallGroup.setCallGroupId(callGroupId);
        editCallGroup.setCallGroup(null);
        return editCallGroup;
    }

    public void formSubmit() {
        delete();
        duplicate();
    }

    /**
     * Deletes all selected rows (on this screen deletes call groups).
     */
    private void delete() {
        Collection selectedRows = getRowsToDelete();
        if (null != selectedRows) {
            getCallGroupContext().removeCallGroups(selectedRows);
        }
    }

    /**
     * Duplicates all selected rows (on this screen duplicates call groups).
     */
    private void duplicate() {
        Collection selectedRows = getRowsToDuplicate();
        if (null != selectedRows) {
            getCallGroupContext().duplicateCallGroups(selectedRows);
        }
    }
}
