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
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.EmergencyInfo;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.service.ServiceDescriptor;
import org.sipfoundry.sipxconfig.service.ServiceManager;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.UnmanagedService;
import org.sipfoundry.sipxconfig.setting.ModelFilesContextImpl;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;
import org.sipfoundry.sipxconfig.sip.SipService;

public class PhoneTestDriver {

    public String serialNumber = "0004f200e06b";

    private final IMocksControl m_phoneContextControl;

    private final PhoneContext m_phoneContext;

    public Phone phone;

    public SipService sip;

    public IMocksControl sipControl;

    private final List<Line> m_lines = new ArrayList<Line>();

    public static PhoneTestDriver supplyTestData(Phone _phone) {
        return supplyTestData(_phone, true);
    }

    public static PhoneTestDriver supplyTestData(Phone _phone, boolean phonebookManagementEnabled) {
        User user = new User();
        user.setUserName("juser");
        user.setFirstName("Joe");
        user.setLastName("User");
        user.setSipPassword("1234");

        return new PhoneTestDriver(_phone, Collections.singletonList(user), phonebookManagementEnabled);
    }

    public static PhoneTestDriver supplyTestData(Phone _phone, List<User> users) {
        return new PhoneTestDriver(_phone, users, true);
    }

    public Line getPrimaryLine() {
        return m_lines.get(0);
    }

    private PhoneTestDriver(Phone _phone, List<User> users, boolean phonebookManagementEnabled) {
        m_phoneContextControl = EasyMock.createNiceControl();
        m_phoneContext = m_phoneContextControl.createMock(PhoneContext.class);

        supplyVitalTestData(m_phoneContextControl, phonebookManagementEnabled, m_phoneContext, _phone);

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
            String uri = users.get(0).getAddrSpec("sipfoundry.org");
            sip.sendCheckSync(uri);
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
        defaults.setLogDirectory("/var/log/sipxpbx");

        SipxRegistrarService registrarService = new SipxRegistrarService();
        registrarService.setModelName("sipxregistrar.xml");
        registrarService.setModelDir("sipxregistrar");
        registrarService.setModelFilesContext(TestHelper.getModelFilesContext());

        SipxServiceManager sipxServiceManager = EasyMock.createNiceMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxRegistrarService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(registrarService);
        EasyMock.replay(sipxServiceManager);
        defaults.setSipxServiceManager(sipxServiceManager);

        ServiceManager serviceManager = EasyMock.createNiceMock(ServiceManager.class);
        EasyMock.replay(serviceManager);
        defaults.setServiceManager(serviceManager);

        return defaults;
    }

    public static void supplyVitalEmergencyData(Phone phone) {
        DeviceDefaults defaults = phone.getPhoneContext().getPhoneDefaults();
        IMocksControl dpControl = EasyMock.createNiceControl();
        DialPlanContext dpContext = dpControl.createMock(DialPlanContext.class);
        dpContext.getLikelyEmergencyInfo();
        EmergencyInfo emergency = new EmergencyInfo("emergency.example.org", 8060, "sos") {
        };
        dpControl.andReturn(emergency).anyTimes();
        dpContext.getVoiceMail();
        dpControl.andReturn("101").anyTimes();
        dpControl.replay();
        defaults.setDialPlanContext(dpContext);
    }

    private static Map<ServiceDescriptor, String> SERVICES;
    static {
        SERVICES = new HashMap<ServiceDescriptor, String>();
        SERVICES.put(UnmanagedService.NTP, "ntp.example.org");
        SERVICES.put(UnmanagedService.DNS, "10.4.5.1");
        SERVICES.put(UnmanagedService.SYSLOG, "10.4.5.2");
    }

    public static void supplyVitalTestData(IMocksControl control, PhoneContext phoneContext, Phone phone) {
        supplyVitalTestData(control, true, phoneContext, phone);
    }

    public static void supplyVitalTestData(IMocksControl control, boolean phonebookManagementEnabled,
            PhoneContext phoneContext, Phone phone) {
        DeviceDefaults defaults = getDeviceDefaults();

        IMocksControl phonebookManagerControl = EasyMock.createNiceControl();
        PhonebookManager phonebookManager = phonebookManagerControl.createMock(PhonebookManager.class);
        phonebookManager.getPhonebookManagementEnabled();
        phonebookManagerControl.andReturn(phonebookManagementEnabled);
        phonebookManagerControl.replay();
        phone.setPhonebookManager(phonebookManager);

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
