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
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Lifecycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.monitoring.Host;
import org.sipfoundry.sipxconfig.admin.monitoring.MonitoringContext;
import org.sipfoundry.sipxconfig.components.RowInfo;
import org.sipfoundry.sipxconfig.components.SelectMap;

public abstract class MonitoringConfigurationPage extends BasePage implements
        PageBeginRenderListener {
    public static final String PAGE = "admin/MonitoringConfigurationPage";

    @InjectObject(value = "spring:monitoringContext")
    public abstract MonitoringContext getMonitoringContext();

    @Bean
    public abstract SelectMap getSelections();

    @Bean(lifecycle = Lifecycle.PAGE)
    public abstract HostRowInfo getRowInfo();

    public abstract Host getCurrentRow();

    public abstract void setHosts(List<Host> hosts);

    public abstract List<Host> getHosts();

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

    public IPage editHostConfiguration(IRequestCycle cycle, String host) {

        MonitoringTargetsConfigurationPage page = (MonitoringTargetsConfigurationPage) cycle
                .getPage(MonitoringTargetsConfigurationPage.PAGE);
        page.editTargets(host, PAGE);
        return page;
    }

    /**
     * Only custom permissions are selectable.
     */
    public static class HostRowInfo implements RowInfo<Host> {
        public boolean isSelectable(Host row) {
            return false;
        }

        public Object getSelectId(Host row) {
            return null;
        }
    }
}
