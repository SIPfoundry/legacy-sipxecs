/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.components.Block;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.admin.commserver.ServiceStatusMessage;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public abstract class ServiceStatusDisplay extends BaseComponent {

    @Parameter(defaultValue = "ognl:70")
    public abstract int getMaxLength();

    @Parameter(required = true)
    public abstract void setStatus(ServiceStatus status);

    public abstract ServiceStatus getStatus();

    @Parameter(required = true)
    public abstract int getRowId();

    @Parameter(required = true)
    public abstract Location getServiceLocation();

    @Asset("context:/WEB-INF/admin/commserver/ServiceStatusDisplay.script")
    public abstract IAsset getScript();

    @Asset("/images/error.png")
    public abstract IAsset getErrorIcon();

    @Asset("/images/unknown.png")
    public abstract IAsset getUnknownIcon();

    @Asset("/images/running.png")
    public abstract IAsset getRunningIcon();

    @Asset("/images/disabled.png")
    public abstract IAsset getDisabledIcon();

    @Asset("/images/loading.gif")
    public abstract IAsset getLoadingIcon();

    @InjectObject("spring:sipxProcessContext")
    public abstract SipxProcessContext getSipxProcessContext();

    @InjectObject("spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectPage(ViewStatusMessages.PAGE)
    public abstract ViewStatusMessages getViewStatusMessagesPage();

    public abstract String getStatusMessage();

    public abstract void setStatusMessage(String statusMessage);

    public abstract boolean getShowAllLink();

    public abstract void setShowAllLink(boolean showAllLink);

    public IAsset getStatusIcon(ServiceStatus status) {
        switch (status.getStatus()) {
        case ConfigurationMismatch:
        case ResourceRequired:
        case ConfigurationTestFailed:
        case Failed:
            return getErrorIcon();
        case Running:
            return getRunningIcon();
        case Testing:
        case Starting:
        case Stopping:
        case ShuttingDown:
            return getLoadingIcon();
        case Disabled:
        case ShutDown:
            return getDisabledIcon();
        default:
            return getUnknownIcon();
        }
    }

    public String getLabelClass(ServiceStatus status) {
        switch (status.getStatus()) {
        case Disabled:
            return "service-disabled";
        case ConfigurationTestFailed:
        case Failed:
        case ResourceRequired:
        case ConfigurationMismatch:
            return "service-error";
        default:
            return "";
        }
    }

    public Block getLinkBlock() {
        String blockName = getStatusMessage() == null ? "loadLink" : "toggleLink";
        return (Block) getComponent(blockName);
    }

    public String getDetailStyle() {
        return (getStatusMessage() == null) ? "display: none;" : "display: block;";
    }

    public boolean shouldTruncate() {
        return getStatusMessage().length() > getMaxLength();
    }

    public String getTruncatedStatusMessage() {
        return getStatusMessage().substring(0, getMaxLength());
    }

    public IPage seeAllMessages(Integer locationId, String serviceBeanId) {
        ViewStatusMessages page = getViewStatusMessagesPage();
        page.setLocationId(locationId);
        page.setServiceBeanId(serviceBeanId);
        page.setReturnPage(getPage());
        return page;
    }

    public void fetchDetails(Integer locationId, String serviceBeanId) {
        SipxService service = getSipxServiceManager().getServiceByBeanId(serviceBeanId);
        Location location = getLocationsManager().getLocation(locationId);
        List<String> allMessages = getSipxProcessContext().getStatusMessages(location, service);

        if (!allMessages.isEmpty()) {
            ServiceStatusMessage message = new ServiceStatusMessage(allMessages.get(0));
            setStatusMessage(String.format("%s: %s", getMessages().getMessage(message.getPrefix()), message
                    .getMessage()));
            setShowAllLink((allMessages.size() > 1) || shouldTruncate());
        } else {
            setStatusMessage(getMessages().getMessage("status.emptyStatus"));
        }
    }
}
