/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.SipxServerTest;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.service.ServiceDescriptor;
import org.sipfoundry.sipxconfig.service.ServiceManager;
import org.sipfoundry.sipxconfig.service.UnmanagedService;
import org.sipfoundry.sipxconfig.setting.ModelFilesContextImpl;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

public class PhoneTestDriver {

    public String serialNumber = "0004f200e06b";

    private IMocksControl m_phoneContextControl;

    private PhoneContext m_phoneContext;

    public Phone phone;

    public SipService sip;

    public IMocksControl sipControl;

    private List<Line> m_lines = new ArrayList<Line>();

    public static PhoneTestDriver supplyTestData(Phone _phone) {
        User user = new User();
        user.setUserName("juser");
        user.setFirstName("Joe");
        user.setLastName("User");
        user.setSipPassword("1234");

        return new PhoneTestDriver(_phone, Collections.singletonList(user));
    }

    public static PhoneTestDriver supplyTestData(Phone _phone, List<User> users) {
        return new PhoneTestDriver(_phone, users);
    }

    public Line getPrimaryLine() {
        return m_lines.get(0);
    }

    private PhoneTestDriver(Phone _phone, List<User> users) {

        m_phoneContextControl = EasyMock.createNiceControl();
        m_phoneContext = m_phoneContextControl.createMock(PhoneContext.class);
        
        supplyVitalTestData(m_phoneContextControl, m_phoneContext, _phone);

        m_phoneContextControl.replay();

        this.phone = _phone;

        _phone.setSerialNumber(serialNumber);

        for (User user : users) {
            Line line = _phone.createLine();
            line.setPhone(_phone);
            line.setUser(user);
            _phone.addLine(line);
            m_lines.add(line);
        }

        sipControl = EasyMock.createStrictControl();
        sip = sipControl.createMock(SipService.class);

        if (users.size() > 0) {
            String uri = users.get(0).getUri("sipfoundry.org");
            sip.sendCheckSync(uri, "sipfoundry.org", "5555");
        }
        sipControl.replay();
        _phone.setSipService(sip);

    }

    public static DeviceDefaults getDeviceDefaults() {
        DeviceDefaults defaults = new DeviceDefaults();
        defaults.setDeviceTimeZone(new DeviceTimeZone("Etc/GMT+5")); // no DST for consistent
        // results
        defaults.setDomainManager(TestHelper.getTestDomainManager("sipfoundry.org"));
        defaults.setFullyQualifiedDomainName("pbx.sipfoundry.org");
        defaults.setTftpServer("tftp.sipfoundry.org");
        defaults.setProxyServerAddr("10.1.2.3");
        defaults.setProxyServerSipPort("5555");
        defaults.setAuthorizationRealm("realm.sipfoundry.org");
        defaults.setSipxServer(SipxServerTest.setUpSipxServer());
        ServiceManager serviceManager = EasyMock.createNiceMock(ServiceManager.class);
        EasyMock.replay(serviceManager);
        defaults.setServiceManager(serviceManager);
        
        return defaults;
    }
    
    private static Map<ServiceDescriptor, String> SERVICES;
    static {
        SERVICES = new HashMap<ServiceDescriptor, String>();
        SERVICES.put(UnmanagedService.NTP, "ntp.example.org");
        SERVICES.put(UnmanagedService.DNS, "10.4.5.1");
        SERVICES.put(UnmanagedService.SYSLOG, "10.4.5.2");
    }
    
    public static void supplyVitalTestData(IMocksControl control, PhoneContext phoneContext, Phone phone) {
        DeviceDefaults defaults = getDeviceDefaults();

        String sysdir = TestHelper.getSysDirProperties().getProperty("sysdir.etc");
        phoneContext.getSystemDirectory();
        control.andReturn(sysdir).anyTimes();

        phoneContext.getPhoneDefaults();
        control.andReturn(defaults).anyTimes();

        ModelFilesContextImpl mfContext = new ModelFilesContextImpl();
        mfContext.setConfigDirectory(sysdir);
        mfContext.setModelBuilder(new XmlModelBuilder(sysdir));
        phone.setModelFilesContext(mfContext);
        
        IMocksControl serviceManagerControl = EasyMock.createNiceControl();
        ServiceManager serviceManager = serviceManagerControl.createMock(ServiceManager.class);
        for (Map.Entry<ServiceDescriptor, String> entry : SERVICES.entrySet()) {
            ServiceDescriptor sd = entry.getKey();
            String addr = entry.getValue();
            serviceManager.getEnabledServicesByType(sd);
            UnmanagedService us = new UnmanagedService();
            us.setDescriptor(sd);
            us.setAddress(addr);
            serviceManagerControl.andReturn(Collections.singletonList(us)).anyTimes();        
        }        
        serviceManagerControl.replay();        
        defaults.setServiceManager(serviceManager);
        
        phone.setPhoneContext(phoneContext);
    }
}
