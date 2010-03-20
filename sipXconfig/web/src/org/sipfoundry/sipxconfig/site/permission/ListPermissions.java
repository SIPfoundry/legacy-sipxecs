/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.permission;

import java.util.Collection;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Lifecycle;
import org.apache.tapestry.callback.PageCallback;
import org.sipfoundry.sipxconfig.components.RowInfo;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionManager;

public abstract class ListPermissions extends SipxBasePage {

    public static final String PAGE = "permission/ListPermissions";

    @InjectObject(value = "spring:permissionManager")
    public abstract PermissionManager getPermissionManager();

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Bean(lifecycle = Lifecycle.PAGE)
    public abstract PermissionRowInfo getRowInfo();

    public abstract Collection getRowsToDelete();

    public abstract Permission getCurrentRow();

    public IPage add(IRequestCycle cycle) {
        EditPermission editPage = (EditPermission) cycle.getPage(EditPermission.PAGE);
        editPage.setPermissionId(null);
        editPage.setCallback(new PageCallback(this));
        return editPage;
    }

    public IPage edit(IRequestCycle cycle, Object permissionId) {
        EditPermission editPage = (EditPermission) cycle.getPage(EditPermission.PAGE);
        editPage.setPermissionId(permissionId);
        editPage.setCallback(new PageCallback(this));
        return editPage;
    }

    public void formSubmit() {
        delete();
    }

    /**
     * Deletes all selected rows (on this screen deletes call groups).
     */
    private void delete() {
        Collection selectedRows = getRowsToDelete();
        if (null != selectedRows) {
            getPermissionManager().removeCallPermissions(selectedRows);
        }
    }

    public String getCurrentLabel() {
        return getCurrentRow().getLabel(getLocale());
    }

    public Collection getAllSelected() {
        return getSelections().getAllSelected();
    }

    /**
     * Only custom permissions are selectable.
     */
    public static class PermissionRowInfo implements RowInfo {
        public boolean isSelectable(Object row) {
            return !((Permission) row).isBuiltIn();
        }

        public Object getSelectId(Object row) {
            return ((Permission) row).getPrimaryKey();
        }
    }
}
