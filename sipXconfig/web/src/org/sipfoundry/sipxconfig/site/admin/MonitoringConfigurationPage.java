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

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.bean.EvenOdd;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.monitoring.Host;
import org.sipfoundry.sipxconfig.admin.monitoring.MonitoringContext;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class MonitoringConfigurationPage extends PageWithCallback implements
        PageBeginRenderListener {
    public static final String PAGE = "admin/MonitoringConfigurationPage";

    @InjectObject(value = "spring:monitoringContext")
    public abstract MonitoringContext getMonitoringContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Bean
    public abstract EvenOdd getRowClass();

    @InjectPage(value = MonitoringTargetsConfigurationPage.PAGE)
    public abstract MonitoringTargetsConfigurationPage getTargetsPage();

    public abstract Host getCurrentRow();

    public abstract void setHosts(List<Host> hosts);

    public abstract List<Host> getHosts();

    public void apply() {
        // empty
    }

    public void pageBeginRender(PageEvent event) {
        List<String> hostsNames = getMonitoringContext().getAvailableHosts();
        Collections.sort(hostsNames);
        List<Host> hosts = new ArrayList<Host>();
        for (String hostName : hostsNames) {
            Host host = new Host();
            host.setHostName(hostName);
            hosts.add(host);
        }
        setHosts(hosts);
    }

    public IPage editHostConfiguration(String host) {
        MonitoringTargetsConfigurationPage page = getTargetsPage();
        page.editTargets(host, PAGE);
        return page;
    }
}
