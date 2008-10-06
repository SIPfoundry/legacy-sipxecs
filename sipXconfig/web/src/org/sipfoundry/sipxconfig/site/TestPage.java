/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site;

import java.io.File;
import java.io.IOException;
import java.io.Serializable;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.io.FileUtils;
import org.apache.commons.lang.RandomStringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.engine.IEngineService;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.admin.WaitingListener;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDescriptor;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapImportManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.GatewayModel;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversalManager;
import org.sipfoundry.sipxconfig.paging.PagingContext;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.search.IndexManager;
import org.sipfoundry.sipxconfig.service.ServiceManager;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.admin.WaitingPage;
import org.sipfoundry.sipxconfig.site.admin.commserver.ReplicationData;
import org.sipfoundry.sipxconfig.site.admin.commserver.RestartReminder;
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

/**
 * TestPage page
 */
public abstract class TestPage extends BasePage {
    public static final String PAGE = "TestPage";

    public static final int JOBS = 4;
    public static final String EMPTY_STRING = "";
    public static final int SERIAL_NUM_LEN = 12;

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

    public static final String PA_PERMISSION = "permission/application/personal-auto-attendant";
    public static final String USER_WITHOUT_PA_PERMISSION = "testUserWithoutAutoAttendantPermission";
    public static final String USER_WITH_PA_PERMISSION = "testUserWithAutoAttendantPermission";

    private static final Log LOG = LogFactory.getLog(TestPage.class);

    public abstract DialPlanContext getDialPlanContext();

    public abstract GatewayContext getGatewayContext();

    @InjectObject(value = "spring:nakedGatewayModelSource")
    public abstract ModelSource<GatewayModel> getGatewayModels();

    @InjectObject(value = "spring:permissionManager")
    public abstract PermissionManager getPermissionManager();

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

    @InjectObject(value = "spring:serviceManager")
    public abstract ServiceManager getServiceManager();

    @InjectObject(value = "spring:sbcManager")
    public abstract SbcManager getSbcManager();

    @InjectObject(value = "spring:sbcDeviceManager")
    public abstract SbcDeviceManager getSbcDeviceManager();

    @InjectObject(value = "spring:sipXbridgeSbcModel")
    public abstract SbcDescriptor getSbcDescriptor();

    @InjectObject(value = "spring:speedDialManager")
    public abstract SpeedDialManager getSpeedDialManager();

    @InjectObject(value = "spring:pagingContext")
    public abstract PagingContext getPagingContext();

    @InjectObject(value = "spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectObject(value = "spring:natTraversalManager")
    public abstract NatTraversalManager getNatTraversalManager();


    @InjectPage(value = WaitingPage.PAGE)
    public abstract WaitingPage getWaitingPage();

    public void resetServiceManager() {
        getServiceManager().clear();
    }

    public void initNatTraversal() {
        getNatTraversalManager().saveDefaultNatTraversal();
        getSbcDeviceManager().clear();
        getSbcDescriptor().setModelId("sipXbridgeSbcModel");
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
        getDialPlanContext().clear();
        getGatewayContext().clear();
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

    public void resetPersonalAttendants() {
        getMailboxManager().clearPersonalAttendants();
    }

    public void seedLocationsManager() {
        Location[] existingLocations = getLocationsManager().getLocations();
        for (Location location : existingLocations) {
            getLocationsManager().deleteLocation(location);
        }

        Location remoteLocation = new Location();
        remoteLocation.setName("Remote Location");
        remoteLocation.setFqdn("host.example.org");
        remoteLocation.setAddress("192.168.155.100");
        getLocationsManager().storeLocation(remoteLocation);
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

    public IPage goToRestartReminderPage(IRequestCycle cycle) {
        RestartReminder page = (RestartReminder) cycle.getPage(RestartReminder.PAGE);
        page.setNextPage(PAGE);
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
            userSession.login(userId, admin, supervisor);
        }
    }

    public void seedTestUser() {
        createTestUserIfMissing();
    }

    private User createTestUserIfMissing() {
        String userName = TEST_USER_USERNAME;
        if (null != getCoreContext().loadUserByUserName(TEST_USER_USERNAME)) {
            // we already have test user - get a unique name for a new one
            userName = TEST_USER_USERNAME + System.currentTimeMillis();
        }
        String firstName = TEST_USER_FIRSTNAME;
        User user = new User();
        user.setUserName(userName);
        user.setFirstName(firstName);
        user.setLastName(TEST_USER_LASTNAME);
        user.setAliasesString(userName.equals(TEST_USER_USERNAME) ? TEST_USER_ALIASES : EMPTY_STRING);
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
            User user = new User();
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
        UserSession userSession = getUserSession();
        userSession.login(user.getId(), false, true);
    }

    public void loginTestUserWithAutoAttendantPermission() {
        User user = getCoreContext().loadUserByUserName(USER_WITH_PA_PERMISSION);
        if (user == null) {
            user = new User();
            user.setPermissionManager(getPermissionManager());
            user.setUserName(USER_WITH_PA_PERMISSION);
            user.setPin(TEST_USER_PIN, getCoreContext().getAuthorizationRealm());
            Setting paSetting = user.getSettings().getSetting(PA_PERMISSION);
            paSetting.setValue(Permission.ENABLE);
            getCoreContext().saveUser(user);
        }

        getUserSession().login(user.getId(), false, true);
    }

    public void loginTestUserWithoutAutoAttendantPermission() {
        User user = getCoreContext().loadUserByUserName(USER_WITHOUT_PA_PERMISSION);
        if (user == null) {
            user = new User();
            user.setPermissionManager(getPermissionManager());
            user.setUserName(USER_WITHOUT_PA_PERMISSION);
            user.setPin(TEST_USER_PIN, getCoreContext().getAuthorizationRealm());
            Setting paSetting = user.getSettings().getSetting(PA_PERMISSION);
            paSetting.setValue(Permission.DISABLE);
            getCoreContext().saveUser(user);
        }

        getUserSession().login(user.getId(), false, true);
    }

    public IPage seedFxoGateway() {
        getDialPlanContext().clear();
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

    public void login() {
        User user = createTestUserIfMissing();
        getUserSession().login(user.getId(), true, true);
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
        // reload from db, ensures spring has injected permission manager
        User user = getCoreContext().loadUser(createTestUserIfMissing().getId());
        user.setPermission(PermissionName.VOICEMAIL, false);
        getCoreContext().saveUser(user);
        getUserSession().login(user.getId(), true, true);
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
            throw new RuntimeException("Could not clean existing sample voicemail store " + existing.getAbsolutePath());
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
}
