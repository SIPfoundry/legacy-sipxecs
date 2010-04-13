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
import java.util.Collection;

import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;

public class CallGroupServiceImpl implements CallGroupService {
    private CallGroupContext m_context;
    private CallGroupBuilder m_builder;

    public void setCallGroupContext(CallGroupContext context) {
        m_context = context;
    }

    public void setCallGroupBuilder(CallGroupBuilder builder) {
        m_builder = builder;
    }

    public void addCallGroup(AddCallGroup acg) throws RemoteException {
        CallGroup apiCg = acg.getCallGroup();
        org.sipfoundry.sipxconfig.admin.callgroup.CallGroup myCg =
            new org.sipfoundry.sipxconfig.admin.callgroup.CallGroup();
        ApiBeanUtil.toMyObject(m_builder, myCg, apiCg);
        m_context.storeCallGroup(myCg);
    }

    public GetCallGroupsResponse getCallGroups() throws RemoteException {
        GetCallGroupsResponse response = new GetCallGroupsResponse();
        Collection callGroupsColl = m_context.getCallGroups();
        org.sipfoundry.sipxconfig.admin.callgroup.CallGroup[] callGroups =
            (org.sipfoundry.sipxconfig.admin.callgroup.CallGroup[]) callGroupsColl
                .toArray(new org.sipfoundry.sipxconfig.admin.callgroup.CallGroup[callGroupsColl.size()]);
        CallGroup[] arrayOfCallGroups =
            (CallGroup[]) ApiBeanUtil.toApiArray(m_builder, callGroups, CallGroup.class);
        response.setCallGroups(arrayOfCallGroups);
        return response;
    }

}
