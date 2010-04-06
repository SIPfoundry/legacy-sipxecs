/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.service.SipxConfigService;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxService;

import static org.easymock.EasyMock.createStrictMock;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class RestartListenerTest extends TestCase {

    public void testRestart() {
        RestartListener restartListener = new RestartListener();

        Location location1 = new Location();
        location1.setUniqueId();
        location1.setName("location1");
        List<SipxService> list1 = new ArrayList<SipxService>();
        list1.add(new SipxConfigService());
        list1.add(new SipxRegistrarService());

        Location location2 = new Location();
        location2.setUniqueId();
        location2.setName("location2");
        List<SipxService> list2 = new ArrayList<SipxService>();
        list2.add(new SipxProxyService());

        Map<Location, List<SipxService>> servicesMap = new HashMap<Location, List<SipxService>>();
        servicesMap.put(location1, list1);
        servicesMap.put(location2, list2);

        SipxProcessContext sipxProcessContext = createStrictMock(SipxProcessContext.class);
        sipxProcessContext.manageServices(servicesMap, SipxProcessContext.Command.RESTART);

        replay(sipxProcessContext);
        restartListener.setSipxProcessContext(sipxProcessContext);
        restartListener.setServicesMap(servicesMap);
        restartListener.afterResponseSent();
        verify(sipxProcessContext);
    }
}
