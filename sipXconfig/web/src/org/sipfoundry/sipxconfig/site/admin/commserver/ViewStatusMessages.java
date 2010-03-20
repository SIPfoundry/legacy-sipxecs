/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.util.List;
import java.util.regex.Pattern;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.ServiceStatusMessage;
import org.sipfoundry.sipxconfig.admin.commserver.ServiceStatusMessageHolder;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.components.LocalizationUtils;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public abstract class ViewStatusMessages extends PageWithCallback implements PageBeginRenderListener {

    public static final Pattern PATTERN_STDERR = Pattern.compile("^stderr.msg-(\\d)*:");
    public static final Pattern PATTERN_STDOUT = Pattern.compile("^stdout.msg-(\\d)*:");
    public static final Pattern PATTERN_MESSAGE_PREFIX = Pattern.compile("^.*:");
    public static final Pattern PATTERN_MESSAGE_DELIMITER = Pattern.compile("(\\-[0-9]*)?:");

    public static final String PAGE = "admin/commserver/ViewStatusMessages";

    public abstract ServiceStatusMessage getCurrentMessage();
    public abstract void setCurrentMessage(ServiceStatusMessage currentMessage);

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectObject("spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @InjectObject("spring:sipxProcessContext")
    public abstract SipxProcessContext getSipxProcessContext();

    public abstract Location getServiceLocation();
    public abstract void setServiceLocation(Location serviceLocation);

    public abstract SipxService getService();
    public abstract void setService(SipxService service);

    @Persist
    public abstract String getServiceBeanId();
    public abstract void setServiceBeanId(String serviceBeanId);

    @Persist
    public abstract int getLocationId();
    public abstract void setLocationId(int locationId);

    public abstract void setStatusMessages(ServiceStatusMessageHolder messages);

    private String getServiceLabel() {
        String serviceBeanId = getServiceBeanId();
        String key = "label." + serviceBeanId;
        return LocalizationUtils.getMessage(getMessages(), key, serviceBeanId);
    }

    public void pageBeginRender(PageEvent event) {
        setServiceLocation(getLocationsManager().getLocation(getLocationId()));
        setService(getSipxServiceManager().getServiceByBeanId(getServiceBeanId()));

        ServiceStatusMessageHolder messages = new ServiceStatusMessageHolder();
        setStatusMessages(messages);

        List<String> allMessages = getSipxProcessContext().getStatusMessages(getServiceLocation(), getService());
        for (String message : allMessages) {
            messages.addMessage(message);
        }
    }

    public void returnToServices(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }

    @Override
    public String getBorderTitle() {
        return getPage().getMessages().getMessage("title") + " " + getServiceLabel();
    }
}
