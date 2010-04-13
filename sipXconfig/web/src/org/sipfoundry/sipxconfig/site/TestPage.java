/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site;

import java.io.File;
import java.io.IOException;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.TimeZone;

import org.acegisecurity.context.SecurityContextHolder;
import org.apache.commons.io.FileUtils;
import org.apache.commons.lang.RandomStringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.engine.IEngineService;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.admin.WaitingListener;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsMigrationTrigger;
import org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContextImpl;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendantManager;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.ResetDialPlanTask;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDescriptor;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.branch.BranchManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapImportManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.device.TimeZoneManager;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.GatewayModel;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversalManager;
import org.sipfoundry.sipxconfig.paging.PagingContext;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.search.IndexManager;
import org.sipfoundry.sipxconfig.security.TestAuthenticationToken;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;
import org.sipfoundry.sipxconfig.service.ServiceManager;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.site.admin.WaitingPage;
import org.sipfoundry.sipxconfig.site.admin.commserver.ReplicationData;
import org.sipfoundry.sipxconfig.site.gateway.EditGateway;
import org.sipfoundry.sipxconfig.site.phone.ManagePhones;
import org.sipfoundry.sipxconfig.site.phone.NewPhone;
import org.sipfoundry.sipxconfig.site.search.EnumEditPageProvider;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;
import org.sipfoundry.sipxconfig.site.upload.EditUpload;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;
import org.sipfoundry.sipxconfig.test.TestUtil;
import org.sipfoundry.sipxconfig.upload.UploadManager;
import org.sipfoundry.sipxconfig.upload.UploadSpecification;
import org.sipfoundry.sipxconfig.vm.MailboxManager;

import static org.sipfoundry.sipxconfig.permission.PermissionName.MUSIC_ON_HOLD;
import static org.sipfoundry.sipxconfig.permission.PermissionName.PERSONAL_AUTO_ATTENDANT;
import static org.sipfoundry.sipxconfig.permission.PermissionName.VOICEMAIL;

/**
 * TestPage page
 */
public abstract class TestPage extends SipxBasePage {
    public static final String PAGE = "TestPage";

    public static final int JOBS = 4;
    public static final String EMPTY_STRING = "";
    public static final int SERIAL_NUM_LEN = 12;

    public static final String TEST_LOCATION_FQDN = "host.example.org";
    public static final String TEST_LOCATION_NAME = "Remote Location";

    // Data for the primary test user
    // Make sure the username matches SiteTestHelper.java
    public static final String TEST_USER_USERNAME = "testuser";
    public static final String TEST_USER_FIRSTNAME = "Test";
    public static final String TEST_USER_LASTNAME = "User";
    public static final String TEST_USER_ALIAS1 = "200";
    public static final String TEST_USER_ALIAS2 = "testy";
    public static final String TEST_USER_ALIASES = TEST_USER_ALIAS1 + " " + TEST_USER_ALIAS2;
    public static final String TEST_USER_PIN = "1234";
    public static final int MANY_USERS = 10000;

    public static final String TEST_PHONE_MODEL_ID = "acmePhoneStandard";

    public static final String USER_WITHOUT_PA_PERMISSION = "testUserWithoutAutoAttendantPermission";
    public static final String USER_WITH_PA_PERMISSION = "testUserWithAutoAttendantPermission";
    public static final String USER_WITHOUT_VM_PERMISSION = "testUserWithoutVoicemailPermission";
    public static final String USER_WITH_MOH_PERMISSION = "testUserWithMoHPermission";
    public static final String USER_WITHOUT_MOH_PERMISSION = "testUserWithoutMoHPermission";

    public static final String SIPX_BRIDGE_MODEL = "sipXbridgeSbcModel";

    private static final Log LOG = LogFactory.getLog(TestPage.class);

    public abstract DialPlanContext getDialPlanContext();

    public abstract GatewayContext getGatewayContext();

    @InjectObject("spring:nakedGatewayModelSource")
    public abstract ModelSource<GatewayModel> getGatewayModels();

    public abstract PhoneContext getPhoneContext();

    public abstract ModelSource<PhoneModel> getPhoneModelSource();

    public abstract ModelSource<UploadSpecification> getUploadSpecificationSource();

    public abstract CallGroupContext getCallGroupContext();

    public abstract ParkOrbitContext getParkOrbitContext();

    public abstract PhonebookManager getPhonebookManager();

    public abstract UploadManager getUploadManager();

    public abstract CoreContext getCoreContext();

    public abstract ForwardingContext getForwardingContext();

    public abstract SipxReplicationContext getSipxReplicationContext();

    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    public abstract JobContext getJobContext();

    public abstract AcdContext getAcdContext();

    public abstract IndexManager getIndexManager();

    public abstract LdapImportManager getLdapImportManager();

    public abstract ApplicationLifecycle getApplicationLifecycle();

    public abstract UserSession getUserSession();

    public abstract IEngineService getRestartService();

    public abstract MailboxManager getMailboxManager();

    @InjectObject("spring:serviceManager")
    public abstract ServiceManager getServiceManager();

    @InjectObject("spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @InjectObject("spring:sbcManager")
    public abstract SbcManager getSbcManager();

    @InjectObject("spring:sbcDeviceManager")
    public abstract SbcDeviceManager getSbcDeviceManager();

    @InjectObject("spring:sipXbridgeSbcModel")
    public abstract SbcDescriptor getSbcDescriptor();

    @InjectObject("spring:speedDialManager")
    public abstract SpeedDialManager getSpeedDialManager();

    @InjectObject("spring:pagingContext")
    public abstract PagingContext getPagingContext();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectObject("spring:natTraversalManager")
    public abstract NatTraversalManager getNatTraversalManager();

    @InjectObject("spring:timeZoneManager")
    public abstract TimeZoneManager getTimeZoneManager();

    @InjectObject("spring:locationsMigrationTrigger")
    public abstract LocationsMigrationTrigger getLocationsMigrationTrigger();

    @InjectObject("spring:autoAttendantManager")
    public abstract AutoAttendantManager getAutoAttendantManager();

    @InjectObject("spring:resetDialPlanTask")
    public abstract ResetDialPlanTask getResetDialPlanTask();

    @InjectObject("spring:sipxProcessContext")
    public abstract SipxProcessContext getSipxProcessContext();

    @InjectObject("spring:branchManager")
    public abstract BranchManager getBranchManager();

    @InjectPage(WaitingPage.PAGE)
    public abstract WaitingPage getWaitingPage();

    public void resetServiceManager() {
        getServiceManager().clear();
    }

    public void initNatTraversal() {
        getSbcDeviceManager().clear();
        getSbcDescriptor().setModelId(SIPX_BRIDGE_MODEL);
        SbcDevice bridge = getSbcDeviceManager().newSbcDevice(getSbcDescriptor());
        bridge.setName("bridge");
        getSbcDeviceManager().storeSbcDevice(bridge);
        getSbcManager().clear();
    }

    public void resetCallForwarding() {
        getForwardingContext().clear();
        getForwardingContext().clearSchedules();
    }

    public void resetDialPlans() {
        getResetDialPlanTask().reset(true);
        getGatewayContext().clear();
        getAutoAttendantManager().clear();
    }

    public void resetInternetCalling() {
        getSbcManager().clear();
    }

    public void resetSbcDevices() {
        getSbcDeviceManager().clear();
    }

    public void resetPhoneContext() {
        getPhoneContext().clear();
    }

    public void resetCallGroupContext() {
        getCallGroupContext().clear();
    }

    public void resetAcdContext() {
        getAcdContext().clear();
    }

    public void resetParkOrbitContext() {
        getParkOrbitContext().clear();
    }

    public void resetConferenceBridgeContext() {
        getConferenceBridgeContext().clear();
    }

    public void resetBranchManager() {
        getBranchManager().clear();
    }

    public void createTestBridge() {
        seedLocationsManager();
        resetConferenceBridgeContext();

        Location location = getLocationsManager().getPrimaryLocation();
        SipxFreeswitchService sipxService = (SipxFreeswitchService) getSipxServiceManager().getServiceByBeanId(
                SipxFreeswitchService.BEAN_ID);

        // Check if we already have a FreeSWITCH service here
        LocationSpecificService service = location.getService(SipxFreeswitchService.BEAN_ID);
        if (service == null) {
            service = new LocationSpecificService(sipxService);
            location.addService(service);
            getLocationsManager().storeLocation(location);
        }

        // Check if the test bridge already exists
        Bridge bridge = getConferenceBridgeContext().getBridgeByServer(location.getFqdn());
        if (bridge == null) {
            bridge = getConferenceBridgeContext().newBridge();
            bridge.setService(service);
            getConferenceBridgeContext().store(bridge);
        }
    }

    public void resetPersonalAttendants() {
        getMailboxManager().clearPersonalAttendants();
    }

    private void resetPrimaryLocation() {
        getLocationsManager().deletePrimaryLocation();
        Location primaryLocation = new Location();
        primaryLocation.setName(TEST_LOCATION_NAME);
        primaryLocation.setFqdn(TEST_LOCATION_FQDN);
        primaryLocation.setAddress("192.168.155.100");
        primaryLocation.setPrimary(true);
        primaryLocation.initBundles(getSipxServiceManager());
        primaryLocation.setRegistered(true);
        getLocationsManager().storeLocation(primaryLocation);
    }

    public void seedLocationsManager() {
        resetConferenceBridgeContext();
        deleteLocations();
        resetPrimaryLocation();
    }

    public void seedService(String beanId) {
        seedLocationsManager();
        Location location = getLocationsManager().getPrimaryLocation();
        Collection<LocationSpecificService> sipxServices = new ArrayList<LocationSpecificService>(
                location.getServices());
        for (LocationSpecificService service : sipxServices) {
            location.removeService(service);
        }
        location.addService(getSipxServiceManager().getServiceByBeanId(beanId));
        getLocationsManager().storeLocation(location);
    }

    public void seedAcdServer() {
        deleteLocations();

        Location location = new Location();
        location.setName("Test Location");
        location.setFqdn("server.example.com");
        location.setAddress("10.1.1.1");
        getLocationsManager().storeLocation(location);

        AcdServer server = getAcdContext().newServer();
        server.setLocation(location);
        getAcdContext().store(server);
    }

    public void seedBridgeSbc() {
        resetSbcDevices();
        seedLocationsManager();

        getSbcDescriptor().setModelId(SIPX_BRIDGE_MODEL);
        SipxProcessContext processContext = new SipxProcessContextImpl() {
            @Override
            public ServiceStatus.Status getStatus(Location location, SipxService service) {
                return ServiceStatus.Status.Disabled;
            }
        };

        BridgeSbc bridgeSbc = (BridgeSbc) getSbcDeviceManager().newSbcDevice(getSbcDescriptor());
        bridgeSbc.setName("sipXbridge");
        bridgeSbc.setLocation(getLocationsManager().getPrimaryLocation());
        bridgeSbc.setAddress(getLocationsManager().getPrimaryLocation().getAddress());
        bridgeSbc.setSettingTypedValue("bridge-configuration/location-id", getLocationsManager()
                .getPrimaryLocation().getId());
        bridgeSbc.setProcessContext(processContext);
        getSbcDeviceManager().storeSbcDevice(bridgeSbc);
    }

    public void deleteBridgeSbc() {
        resetSbcDevices();
        deleteLocations();
    }

    /**
     * Clear all locations. Primary location is not allowed to be deleted
     */
    private void deleteLocations() {
        resetAcdContext();
        Location[] existingLocations = getLocationsManager().getLocations();
        for (Location location : existingLocations) {
            if (!location.isPrimary()) {
                getLocationsManager().deleteLocation(location);
            }
        }
    }

    public String resetCoreContext() {
        // need to reset all data that could potentially have a reference
        // to users
        resetPersonalAttendants();
        getSpeedDialManager().clear();
        getPagingContext().clear();
        resetDialPlans();
        resetCallForwarding();
        resetPhoneContext();
        resetCallGroupContext();
        getCoreContext().clear();
        getApplicationLifecycle().logout();
        // force rendering any new page after logout or infamous "invalid session" after
        // any links are clicked
        return PAGE;
    }

    public void resetPhonebook() {
        getPhonebookManager().reset();
    }

    public String logout() {
        getApplicationLifecycle().logout();
        return PAGE;
    }

    public IPage newGroup(IRequestCycle cycle, String resource) {
        EditGroup page = (EditGroup) cycle.getPage(EditGroup.PAGE);
        page.newGroup(resource, PAGE);
        return page;
    }

    public void toggleNavigation() {
        UserSession userSession = getUserSession();
        userSession.setNavigationVisible(!userSession.isNavigationVisible());
    }

    public void hideNavigation() {
        getUserSession().setNavigationVisible(false);
    }

    public void toggleAdmin() {
        UserSession userSession = getUserSession();
        boolean admin = !userSession.isAdmin();
        boolean supervisor = !userSession.isSupervisor();
        Integer userId = userSession.getUserId();
        if (userId == null) {
            login();
        } else {
            User user = getCoreContext().loadUser(userId);
            login(user, admin, supervisor);
        }
    }

    public void seedTestUser() {
        createUserIfMissing(TEST_USER_USERNAME + System.currentTimeMillis(), VOICEMAIL, true);
        createTestUserIfMissing();
    }

    private User createTestUserIfMissing() {
        User user = getCoreContext().loadUserByUserName(TEST_USER_USERNAME);
        if (user != null) {
            return user;
        }
        user = getCoreContext().newUser();
        user.setUserName(TEST_USER_USERNAME);
        user.setFirstName(TEST_USER_FIRSTNAME);
        user.setLastName(TEST_USER_LASTNAME);
        user.setAliasesString(TEST_USER_ALIASES);
        user.setPin(TEST_USER_PIN, getCoreContext().getAuthorizationRealm());
        getCoreContext().saveUser(user);
        return user;
    }

    public void populateUsers() {
        long l = System.currentTimeMillis();
        CoreContext coreContext = getCoreContext();
        String authorizationRealm = coreContext.getAuthorizationRealm();
        for (int i = 0; i < MANY_USERS; i++) {
            String firstName = TEST_USER_FIRSTNAME + i;
            User user = getCoreContext().newUser();
            user.setUserName("xuser" + (l + i));
            user.setFirstName(firstName);
            user.setLastName(TEST_USER_LASTNAME);
            user.setPin(TEST_USER_PIN, authorizationRealm);
            coreContext.saveUser(user);
        }
    }

    public void loginFirstTestUser() {
        // Find the first test user
        User user = getCoreContext().loadUserByUserName(TEST_USER_USERNAME);
        if (user == null) {
            throw new IllegalStateException("Test user with username = " + TEST_USER_USERNAME
                    + " is not in the database");
        }

        // Log it in
        login(user, false, true);
    }

    public void loginTestUserAutoAttendantPermission(boolean enabled) {
        String name = enabled ? USER_WITH_PA_PERMISSION : USER_WITHOUT_PA_PERMISSION;
        User user = createUserIfMissing(name, PERSONAL_AUTO_ATTENDANT, enabled);
        login(user, false, true);
    }

    public void loginTestUserMoHPermission(boolean enabled) {
        String name = enabled ? USER_WITH_MOH_PERMISSION : USER_WITHOUT_MOH_PERMISSION;
        User user = createUserIfMissing(name, MUSIC_ON_HOLD, enabled);
        login(user, false, true);
    }

    private User createUserIfMissing(String username, PermissionName permissionName, boolean enabled) {
        User user = getCoreContext().loadUserByUserName(username);
        if (user != null) {
            return user;
        }
        user = getCoreContext().newUser();
        user.setUserName(username);
        user.setPin(TEST_USER_PIN, getCoreContext().getAuthorizationRealm());
        user.setPermission(permissionName, enabled);
        getCoreContext().saveUser(user);
        return user;
    }

    public IPage seedFxoGateway() {
        getResetDialPlanTask().reset(true);
        GatewayContext gatewayService = getGatewayContext();
        gatewayService.clear();
        GatewayModel fxoModel = getGatewayModels().getModel("audiocodesTP260");
        Gateway fxo = gatewayService.newGateway(fxoModel);
        fxo.setName("fxo");
        fxo.setAddress("1.1.1.1");
        gatewayService.storeGateway(fxo);
        EditGateway page = (EditGateway) getRequestCycle().getPage(EditGateway.PAGE);
        page.setGatewayId(fxo.getId());
        return page;
    }

    @SuppressWarnings("deprecation")
    public void deleteAllUsers() {
        resetPersonalAttendants();
        List users = getCoreContext().loadUsers();
        for (Iterator iter = users.iterator(); iter.hasNext();) {
            User user = (User) iter.next();
            getCoreContext().deleteUser(user);
        }
    }

    public IPage newPhone() {
        NewPhone newPhone = (NewPhone) getRequestCycle().getPage(NewPhone.PAGE);
        newPhone.setReturnPage(ManagePhones.PAGE);
        PhoneModel model = getPhoneModelSource().getModel(TEST_PHONE_MODEL_ID);
        newPhone.setPhoneModel(model);
        return newPhone;
    }

    public void importLdap() {
        getLdapImportManager().insert();
    }

    public void indexAll() {
        getIndexManager().indexAll();
    }

    public void populateJobs() {
        JobContext jobContext = getJobContext();
        jobContext.clear();
        Serializable[] jobIds = new Serializable[JOBS];
        for (int i = 0; i < jobIds.length; i++) {
            jobIds[i] = jobContext.schedule("test" + i);
            if (i > 0) {
                jobContext.start(jobIds[i]);
            }
        }
        jobContext.success(jobIds[2]);
        jobContext.failure(jobIds[JOBS - 1], "something bad happened", null);
    }

    public void populatePhones() {
        for (PhoneModel model : getPhoneModelSource().getModels()) {
            Phone phone = getPhoneContext().newPhone(model);
            phone.setSerialNumber(RandomStringUtils.randomNumeric(SERIAL_NUM_LEN));
            getPhoneContext().storePhone(phone);
        }
    }

    public void defaultDeviceTimeZone() {
        TimeZone tz = TimeZone.getDefault();
        TimeZone.setDefault(TimeZone.getTimeZone("Europe/Helsinki"));

        getTimeZoneManager().saveDefault();
        // clean-up
        TimeZone.setDefault(tz);
    }

    public void login() {
        User user = createTestUserIfMissing();
        login(user, true, true);
    }

    private void login(User user, boolean isAdmin, boolean isSupervisor) {
        TestAuthenticationToken token = new TestAuthenticationToken(user, isAdmin, isSupervisor);
        SecurityContextHolder.getContext().setAuthentication(token.authenticateToken());
    }

    public void generateDataSet(String setName) {
        SipxReplicationContext sipxReplicationContext = getSipxReplicationContext();
        sipxReplicationContext.generate(DataSet.getEnum(setName));
    }

    public void throwException() {
        throw new IllegalArgumentException("Just testing");
    }

    public void validateEditPageProvider(IRequestCycle cycle) {
        EnumEditPageProvider provider = new EnumEditPageProvider();
        provider.validatePages(cycle);
    }

    public IPage newUpload(IRequestCycle cycle) {
        EditUpload page = (EditUpload) cycle.getPage(EditUpload.PAGE);
        page.setUploadId(null);
        // get first specification, any should do
        page.setUploadSpecification(getUploadSpecificationSource().getModels().iterator().next());
        page.setReturnPage(PAGE);
        return page;
    }

    public void resetUploadManager() {
        getUploadManager().clear();
    }

    public IPage showDataSet(IRequestCycle cycle, String setName) {
        ReplicationData page = (ReplicationData) cycle.getPage(ReplicationData.PAGE);
        page.setDataSetName(setName);
        return page;
    }

    public void loginUserWithDisabledVoicemailPermission() {
        User user = createUserIfMissing(USER_WITHOUT_VM_PERMISSION, PermissionName.VOICEMAIL, false);
        login(user, true, true);
    }

    public void disableVoicemail() {
        MailboxManager mgr = getMailboxManager();
        File existing = new File(mgr.getMailstoreDirectory());
        try {
            FileUtils.deleteDirectory(existing);
        } catch (IOException e) {
            throw new RuntimeException("Could not delete mailstore " + existing.getAbsolutePath(), e);
        }
    }

    public void resetVoicemail() {
        MailboxManager mgr = getMailboxManager();
        File existing = new File(mgr.getMailstoreDirectory());
        if (!existing.exists()) {
            existing.mkdir();
        }
        try {
            FileUtils.cleanDirectory(existing);
        } catch (IOException e) {
            throw new RuntimeException("Could not clean existing sample voicemail store "
                    + existing.getAbsolutePath());
        }

        Class manageVmTestUiClass;
        try {
            manageVmTestUiClass = Class.forName("org.sipfoundry.sipxconfig.site.vm.ManageVoicemailTestUi");
        } catch (ClassNotFoundException e1) {
            throw new RuntimeException("Cannot access ui test directory via test class resource");
        }
        File original = new File(TestUtil.getTestSourceDirectory(manageVmTestUiClass) + "/mailstore");
        try {
            FileUtils.copyDirectory(original, existing);
        } catch (IOException e) {
            String msg = String.format("Could not reset sample voicemail store from original '%s' to '%s'", original
                    .getAbsolutePath(), existing.getAbsolutePath());
            throw new RuntimeException(msg);
        }
    }

    private static final class LogWaitingListener implements WaitingListener, Serializable {
        public void afterResponseSent() {
            LOG.warn("delayed action triggered");
        }
    }

    public IPage testWaitingPage() {
        WaitingPage waitingPage = getWaitingPage();
        WaitingListener waitingListener = new LogWaitingListener();
        waitingPage.setWaitingListener(waitingListener);
        return waitingPage;
    }

    public void markForRestart() {
        Location location = getLocationsManager().getPrimaryLocation();
        SipxService service = getSipxServiceManager().getServiceDefinitions().iterator().next();
        location.addService(service);
        getLocationsManager().storeLocation(location);

        getSipxProcessContext().markServicesForRestart(Arrays.asList(service));
    }

    public void clearRestartNeeded() {
        getSipxProcessContext().clear();
    }

    public void createAdminUserAndAdminGroup() {
        if (getCoreContext().loadUserByUserName(User.SUPERADMIN) == null) {
            getCoreContext().createAdminGroupAndInitialUser("123");
        }
    }
}
