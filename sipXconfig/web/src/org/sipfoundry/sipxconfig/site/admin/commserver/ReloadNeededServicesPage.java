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

import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class ReloadNeededServicesPage extends SipxBasePage implements PageBeginRenderListener {
    public static final Object PAGE = "admin/commserver/ReloadNeededServicesPage";

//    @InjectObject("spring:sipxProcessContext")
//    public abstract SipxProcessContext getSipxProcessContext();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

//    @InjectObject("spring:sipxServiceManager")
//    public abstract SipxServiceManager getSipxServiceManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Bean
    public abstract SelectMap getSelections();

    @Asset("/images/server.png")
    public abstract IAsset getServerIcon();

    @Asset("/images/service_restart.png")
    public abstract IAsset getRestartIcon();

//    public abstract ReloadNeededService getCurrentRow();
//
//    public abstract Collection<ReloadNeededService> getReloadNeededServices();
//
//    public abstract void setReloadNeededServices(Collection<ReloadNeededService> reloadNeededServices);

    public void pageBeginRender(PageEvent event) {
//        if (getReloadNeededServices() == null) {
//            setReloadNeededServices(getSipxProcessContext().getReloadNeededServices());
//        }
    }

//    public String getServiceLabel() {
//        String serviceBeanId = getCurrentRow().getServiceBeanId();
//        String key = "label." + serviceBeanId;
//        return getMessage(getMessages(), key, serviceBeanId);
//    }

    public void reload() {
//        Collection<ReloadNeededService> beans = getSelections().getAllSelected();
//        if (beans == null) {
//            return;
//        }
//
//        getSipxProcessContext()
//                .manageServices(createLocationToServiceMap(beans), SipxProcessContext.Command.RELOAD);
//
//        // Forces a page refresh
//        setReloadNeededServices(null);
    }

//    public void ignore() {
//        Collection<ReloadNeededService> beans = getSelections().getAllSelected();
//        if (beans == null) {
//            return;
//        }
//
//        getSipxProcessContext().unmarkServicesToReload(beans);
//
//        // Forces a page refresh
//        setReloadNeededServices(null);
//    }
//
//    private Map<Location, List<SipxService>> createLocationToServiceMap(Collection<ReloadNeededService> beans) {
//        Map<Location, List<SipxService>> map = new HashMap<Location, List<SipxService>>();
//        for (ReloadNeededService bean : beans) {
//            Location location = getLocationsManager().getLocationByFqdn(bean.getLocation());
//            if (location == null) {
//                continue;
//            }
//            List<SipxService> services = map.get(location);
//            if (services == null) {
//                services = new ArrayList<SipxService>();
//                map.put(location, services);
//            }
//            SipxService service = getSipxServiceManager().getServiceByBeanId(bean.getServiceBeanId());
//            services.add(service);
//        }
//        return map;
//    }
}
