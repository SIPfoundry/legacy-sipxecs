/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.components;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.components.Block;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.admin.monitoring.MonitoringContext;
import org.sipfoundry.sipxconfig.admin.update.PackageUpdateManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.service.SipxAcdService;
import org.sipfoundry.sipxconfig.service.SipxOpenAcdService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.site.CustomPageManager;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class AdminNavigation extends BaseComponent implements PageBeginRenderListener {

    @InjectObject("spring:monitoringContext")
    public abstract MonitoringContext getMonitoringContext();

    @InjectObject("spring:coreContext")
    public abstract CoreContext getContext();

    @InjectObject("spring:packageUpdateManager")
    public abstract PackageUpdateManager getPackageUpdateManager();

    @InjectObject("spring:sbcDeviceManager")
    public abstract SbcDeviceManager getSbcDeviceManager();

    @InjectObject("spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    public abstract void setPluginMenus(Map menus);

    public abstract Map getPluginMenus();

    public abstract String getBlocklId();

    public abstract void setBlockId(String id);

    @InjectObject("spring:customPageManager")
    public abstract CustomPageManager getCustomPageManager();

    public boolean isOpenFireEnabled() {
        // it uses the service name defined in openfire plugin
        return getSipxServiceManager().isServiceInstalled("sipxOpenfireService");
    }

    public boolean isAcdEnabled() {
        return getSipxServiceManager().isServiceInstalled(SipxAcdService.BEAN_ID);
    }

    public boolean isOpenAcdEnabled() {
        return getSipxServiceManager().isServiceInstalled(SipxOpenAcdService.BEAN_ID);
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        Collection<String> customMenus = getCustomPageManager().getAdminMenuPageIds();
        Map menus = new HashMap();
        for (String pageid : customMenus) {
            IPage pluginMenuPage = getPage().getRequestCycle().getPage("plugin/" + pageid);
            menus.putAll(pluginMenuPage.getComponents());
        }
        setPluginMenus(menus);
    }

    public boolean isPluginMenuAvailable(String menu) {
        if (getPluginMenus() != null && getPluginMenus().containsKey(menu)) {
            return true;
        }
        return false;
    }

    public Collection getMenuBlocks(String menuId) {
        List<String> blocks = new ArrayList<String>();
        for (String pageId : getCustomPageManager().getAdminMenuPageIds()) {
            String qualifiedBlockId = pageId + menuId;
            if (getPluginMenus().containsKey(qualifiedBlockId)) {
                blocks.add(qualifiedBlockId);
            }
        }
        return blocks;
    }

    public Block getPluginMenu(String menu) {
        if (getPluginMenus() != null) {
            return (Block) getPluginMenus().get(menu);
        }
        return null;
    }

}
