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

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionManager;

public abstract class EditPermission extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "permission/EditPermission";

    @InjectObject(value = "spring:permissionManager")
    public abstract PermissionManager getPermissionManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Object getPermissionId();

    public abstract void setPermissionId(Object id);

    public abstract Permission getPermission();

    public abstract void setPermission(Permission permission);

    public void pageBeginRender(PageEvent event_) {
        Permission permission = getPermission();
        if (null != permission) {
            return;
        }
        Object id = getPermissionId();
        if (null != id) {
            PermissionManager pm = getPermissionManager();
            permission = pm.getCallPermission(id);
        } else {
            permission = new Permission();
        }
        setPermission(permission);
    }

    public void commit() {
        if (isValid()) {
            saveValid();
        }
    }

    private boolean isValid() {
        return TapestryUtils.isValid(this);
    }

    private void saveValid() {
        PermissionManager pm = getPermissionManager();
        Permission permission = getPermission();
        if (!permission.isBuiltIn()) {
            pm.addCallPermission(permission);
            setPermissionId(permission.getPrimaryKey());
        }
    }

    public String getLabel() {
        return getPermission().getLabel(getLocale());
    }

    public void setLabel(String label) {
        getPermission().setLabel(label);
    }

    public String getDescription() {
        return getPermission().getDescription(getLocale());
    }

    public void setDescription(String description) {
        getPermission().setDescription(description);
    }
}
