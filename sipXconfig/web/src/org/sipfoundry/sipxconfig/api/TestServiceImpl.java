/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.api;

import java.rmi.RemoteException;

import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public class TestServiceImpl implements TestService {
    private ForwardingContext m_forwardingContext;
    private CallGroupContext m_callGroupContext;
    private CoreContext m_coreContext;
    private ParkOrbitContext m_parkOrbitContext;
    private PhoneContext m_phoneContext;
    private PermissionManager m_permissionManager;

    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }

    public void setCallGroupContext(CallGroupContext callGroupContext) {
        m_callGroupContext = callGroupContext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setParkOrbitContext(ParkOrbitContext parkOrbitContext) {
        m_parkOrbitContext = parkOrbitContext;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void setPermissionManager(PermissionManager permissionManager) {
        m_permissionManager = permissionManager;
    }

    public void resetServices(ResetServices resetServices) throws RemoteException {
        // Clear the call group context not only when requested, but also when the
        // core context is going to be cleared.  Otherwise we get database integrity
        // problems, because the user ring objects in a call group reference users.
        String preserveAdminPintoken = null;
        String adminUsername = "superadmin";
        if (Boolean.TRUE.equals(resetServices.getCallGroup())) {
            m_callGroupContext.clear();
        }
        if (Boolean.TRUE.equals(resetServices.getParkOrbit())) {
            m_parkOrbitContext.clear();
        }
        if (Boolean.TRUE.equals(resetServices.getPhone())) {
            m_phoneContext.clear();
        }
        if (Boolean.TRUE.equals(resetServices.getPermission())) {
            m_permissionManager.clear();
        }
        if (Boolean.TRUE.equals(resetServices.getUser())) {
            // kludge: XCF-1010: have to clear these too, any leftover data
            // from any other tests will cause contraint violations
            // on user context delete
            m_forwardingContext.clear();
            m_forwardingContext.clearSchedules();
            m_callGroupContext.clear();

            org.sipfoundry.sipxconfig.common.User superadmin = m_coreContext.loadUserByUserName(adminUsername);
            if (superadmin != null) {
                preserveAdminPintoken = superadmin.getPintoken();
            }

            m_coreContext.clear();
        }
        if (Boolean.TRUE.equals(resetServices.getSuperAdmin())) {
            m_coreContext.createAdminGroupAndInitialUser("");
            if (preserveAdminPintoken != null) {
                org.sipfoundry.sipxconfig.common.User superadmin = m_coreContext.loadUserByUserName(adminUsername);
                superadmin.setPintoken(preserveAdminPintoken);
                m_coreContext.saveUser(superadmin);
            }
        }
    }
}
