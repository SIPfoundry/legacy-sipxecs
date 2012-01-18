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
package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.util.Set;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.logging.AuditLogContext;

public abstract class LocationStatePage extends PageWithCallback {

    public static final String PAGE = "admin/commserver/LocationStatePage";

    public abstract Location getServer();

    public abstract void setServer(Location location);

    @InjectObject("spring:auditLogContext")
    public abstract AuditLogContext getAuditLogContext();

    public abstract String getCurrentName();

    public void returnToServers(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }

    @Override
    public String getBorderTitle() {
        return getServer().getFqdn();
    }

    public boolean isInProgress() {
        return getServer().isInProgressState();
    }

    public boolean isPreviousAttemptNotFinished() {
        return getServer().isInNotFinishedState();
    }

    public boolean isInConfigurationError() {
        return getServer().isInConfigurationErrorState();
    }

    public boolean isConfigured() {
        return getServer().isConfigured();
    }

    public boolean isFailed() {
        return getFailedReplications() != null && !getFailedReplications().isEmpty();
    }

    public boolean isSucceded() {
        return getSuccededReplications() != null && !getSuccededReplications().isEmpty();
    }

    public Set<String> getFailedReplications() {
        return getAuditLogContext().getReplicationFailedList((getServer().getFqdn()));
    }

    public Set<String> getSuccededReplications() {
        return getAuditLogContext().getReplicationSuccededList(getServer().getFqdn());
    }
}
