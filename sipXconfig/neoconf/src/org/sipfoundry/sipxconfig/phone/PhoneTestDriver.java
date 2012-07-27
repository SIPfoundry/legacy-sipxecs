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

import static org.easymock.EasyMock.anyObject;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.createNiceControl;
import static org.easymock.EasyMock.createStrictControl;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.sipfoundry.sipxconfig.test.TestHelper.getMockDomainManager;

import java.util.ArrayList;
import java.util.List;
import java.util.TimeZone;

import org.easymock.IMocksControl;
import org.easymock.classextension.EasyMock;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dialplan.EmergencyInfo;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.moh.MohAddressFactory;
import org.sipfoundry.sipxconfig.paging.PagingContext;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.setting.ModelFilesContextImpl;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;
import org.sipfoundry.sipxconfig.sip.SipService;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.sipfoundry.sipxconfig.test.TestHelper;

public final class PhoneTestDriver {
    public static final String SIPFOUNDRY_ORG = "sipfoundry.org";

    private final IMocksControl m_phoneContextControl;

    private final PhoneContext m_phoneContext;

    private final MohAddressFactory m_mohAddresses;

    private final List<Line> m_lines = new ArrayList<Line>();

    private SipService m_sip;

    private IMocksControl m_sipControl;

    private String m_serialNumber = "0004f200e06b";

    private PhoneTestDriver(Phone phone, List<User> users, boolean phonebookManagementEnabled, boolean speedDial,
            boolean sendCheckSyncToMac) {
        m_phoneContextControl = createNiceControl();
        m_phoneContext = m_phoneContextControl.createMock(PhoneContext.class);

        if (speedDial) {
            SpeedDial sd = new SpeedDial();
            sd.setUser(users.get(0));

            m_phoneContext.getSpeedDial(phone);
            m_phoneContextControl.andReturn(sd).anyTimes();
        }

        supplyVitalTestData(m_phoneContextControl, phonebookManagementEnabled, m_phoneContext, phone);

        m_phoneContextControl.replay();

        phone.setSerialNumber(m_serialNumber);

        m_mohAddresses = EasyMock.createMock(MohAddressFactory.class);

        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext(TestHelper.getSystemEtcDir()));

        for (User user : users) {
            Line line = phone.createLine();
            line.setPhone(phone);
            line.setUser(user);
            phone.addLine(line);
            m_lines.add(line);

            if (user != null) {
                user.setPermissionManager(pm);
                m_mohAddresses.getPersonalMohFilesUri(user.getUserName());
                EasyMock.expectLastCall().andReturn("sip:~~mh~" + user.getUserName() + "@" + SIPFOUNDRY_ORG).anyTimes();
                user.setSettingTypedValue("moh/audio-source", "PERSONAL_FILES_SRC");
                user.setMohAddresses(m_mohAddresses);
            }
        }

        EasyMock.replay(m_mohAddresses);

        m_sipControl = createStrictControl();
        m_sip = m_sipControl.createMock(SipService.class);

        if (sendCheckSyncToMac) {
            m_sip.sendCheckSync(phone.getInstrumentAddrSpec());
        } else if (users.size() > 0) {
            String uri = users.get(0).getAddrSpec(SIPFOUNDRY_ORG);
            m_sip.sendCheckSync(uri);
        }

        m_sipControl.replay();
        phone.setSipService(m_sip);

    }

    public static PhoneTestDriver supplyTestData(Phone phone) {
        return supplyTestData(phone, true);
    }

    public static PhoneTestDriver supplyTestData(Phone phone, boolean phonebookManagementEnabled) {
        return supplyTestData(phone, phonebookManagementEnabled, false, false, false);
    }

    public static PhoneTestDriver supplyTestData(Phone phone, List<User> users) {
        return new PhoneTestDriver(phone, users, true, false, false);
    }

    public static PhoneTestDriver supplyTestData(Phone phone, List<User> users, boolean sendCheckSyncToMac) {
        return new PhoneTestDriver(phone, users, true, false, sendCheckSyncToMac);
    }

    public static PhoneTestDriver supplyTestData(Phone phone, boolean phonebookManagementEnabled,
            boolean speedDial, boolean includeSharedLine, boolean sendCheckSyncToMac) {
        List<User> users = new ArrayList<User>();

        User firstUser = new User();
        firstUser.setUserName("juser");
        firstUser.setFirstName("Joe");
        firstUser.setLastName("User");
        firstUser.setSipPassword("1234");
        firstUser.setIsShared(false);
        users.add(firstUser);

        if (includeSharedLine) {
            User secondUser = new User();
            secondUser.setUserName("sharedUser");
            secondUser.setFirstName("Shared");
            secondUser.setLastName(firstUser.getLastName());
            secondUser.setSipPassword(firstUser.getSipPassword());
            secondUser.setIsShared(true);
            users.add(secondUser);
        }

        return new PhoneTestDriver(phone, users, phonebookManagementEnabled, speedDial, sendCheckSyncToMac);
    }

    public SipService getSip() {
        return m_sip;
    }

    public void setSip(SipService sip) {
        m_sip = sip;
    }

    public IMocksControl getSipControl() {
        return m_sipControl;
    }

    public void setSipControl(IMocksControl sipControl) {
        m_sipControl = sipControl;
    }

    public String getSerialNumber() {
        return m_serialNumber;
    }

    public void setSerialNumber(String serialNumber) {
        m_serialNumber = serialNumber;
    }

    public Line getPrimaryLine() {
        return m_lines.get(0);
    }

    public static DeviceDefaults getDeviceDefaults() {
        DeviceDefaults defaults = new DeviceDefaults() {
            // This is a hack to workaround the problem that polycom plugin
            // cannot access sipxregistrar.xml from unit tests.
            @Override
            public String getDirectedCallPickupCode() {
                return "*78";
            }

            @Override
            public String getCallRetrieveCode() {
                return "*4";
            }
        };

        Location defaultLocation = new Location();
        defaultLocation.setFqdn("pbx." + SIPFOUNDRY_ORG);
        defaultLocation.setAddress("192.168.1.1");
        LocationsManager locationsManager = createMock(LocationsManager.class);
        locationsManager.getPrimaryLocation();
        expectLastCall().andReturn(defaultLocation).anyTimes();
        replay(locationsManager);
        defaults.setLocationsManager(locationsManager);

        TimeZone tz = TimeZone.getTimeZone("Etc/GMT+5");
        DeviceTimeZone dtz = new DeviceTimeZone();
        dtz.setTimeZone(tz);
        defaults.setTimeZoneManager(TestHelper.getTimeZoneManager(dtz)); // no
        // DST
        // for
        // consistent
        // results
        defaults.setDomainManager(TestHelper.getTestDomainManager(SIPFOUNDRY_ORG));

        DomainManager domainManager = getMockDomainManager();
        replay(domainManager);
        domainManager.getDomain().setSipRealm("realm." + SIPFOUNDRY_ORG);
        domainManager.getDomain().setName(SIPFOUNDRY_ORG);
        defaults.setDomainManager(domainManager);

        MohAddressFactory mohAddresses = EasyMock.createMock(MohAddressFactory.class);
        mohAddresses.getDefaultMohUri();
        EasyMock.expectLastCall().andReturn("sip:~~mh~@" + SIPFOUNDRY_ORG).anyTimes();
        defaults.setMohAddressFactory(mohAddresses);
        EasyMock.replay(mohAddresses);
        defaults.setLogDirectory("/var/log/sipxpbx");

        AddressManager addressManager = EasyMock.createMock(AddressManager.class);
        addressManager.getSingleAddress((AddressType) anyObject());
        AddressType t = new AddressType("test");
        expectLastCall().andReturn(new Address(t, "testdriver.example.org", 1234)).anyTimes();
        replay(addressManager);
        defaults.setAddressManager(addressManager);

//        SipxService registrarService = new SipxRegistrarService();
//        registrarService.setModelFilesContext(TestHelper.getModelFilesContext());
//        registrarService.setBeanId(SipxRegistrarService.BEAN_ID);
//        registrarService.setModelName("sipxregistrar.xml");
//        registrarService.setModelDir("sipxregistrar");
//
//        SipxService proxyService = new SipxProxyService();
//        proxyService.setModelFilesContext(TestHelper.getModelFilesContext());
//        proxyService.setBeanId(SipxProxyService.BEAN_ID);
//        proxyService.setModelName("sipxproxy.xml");
//        proxyService.setModelDir("sipxproxy");
//        proxyService.setSipPort("5555");

        return defaults;
    }

    public static void supplyVitalEmergencyData(Phone phone, String emergencyValue) {
        DeviceDefaults defaults = phone.getPhoneContext().getPhoneDefaults();
        IMocksControl dpControl = createNiceControl();
        DialPlanContext dpContext = dpControl.createMock(DialPlanContext.class);
        dpContext.getLikelyEmergencyInfo();
        EmergencyInfo emergency = new EmergencyInfo("emergency.example.org", 8060, emergencyValue) {
        };
        dpControl.andReturn(emergency).anyTimes();
        dpContext.getVoiceMail();
        dpControl.andReturn("101").anyTimes();
        dpControl.replay();
        defaults.setDialPlanContext(dpContext);
    }

    public static void supplyVitalTestData(IMocksControl control, PhoneContext phoneContext, Phone phone) {
        supplyVitalTestData(control, true, phoneContext, phone);
    }

    public static void supplyVitalTestData(IMocksControl control, boolean phonebookManagementEnabled,
            PhoneContext phoneContext, Phone phone) {
        DeviceDefaults defaults = getDeviceDefaults();

        IMocksControl phonebookManagerControl = createNiceControl();
        PhonebookManager phonebookManager = phonebookManagerControl.createMock(PhonebookManager.class);
        phonebookManager.getPhonebookManagementEnabled();
        phonebookManagerControl.andReturn(phonebookManagementEnabled);
        phonebookManagerControl.replay();
        phone.setPhonebookManager(phonebookManager);
        phoneContext.getSystemDirectory();
        control.andReturn(TestHelper.getSystemEtcDir()).anyTimes();

        phoneContext.getPhoneDefaults();
        control.andReturn(defaults).anyTimes();

        ModelFilesContextImpl mfContext = new ModelFilesContextImpl();
        mfContext.setConfigDirectory(TestHelper.getEtcDir());
        mfContext.setModelBuilder(new XmlModelBuilder(TestHelper.getSystemEtcDir()));
        phone.setModelFilesContext(mfContext);

        IMocksControl pagingContextControl = createNiceControl();
        PagingContext pagingContext = pagingContextControl.createMock(PagingContext.class);
        pagingContextControl.replay();
        defaults.setPagingContext(pagingContext);
        phone.setPhoneContext(phoneContext);
    }
}
