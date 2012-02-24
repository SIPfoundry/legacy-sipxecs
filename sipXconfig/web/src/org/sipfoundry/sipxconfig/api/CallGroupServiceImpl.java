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

import org.sipfoundry.sipxconfig.callgroup.CallGroupContext;

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
        org.sipfoundry.sipxconfig.callgroup.CallGroup myCg =
            new org.sipfoundry.sipxconfig.callgroup.CallGroup();
        ApiBeanUtil.toMyObject(m_builder, myCg, apiCg);
        m_context.saveCallGroup(myCg);
    }

    public void removeCallGroup(RemoveCallGroup cg) throws RemoteException {
        m_context.removeCallGroupByAlias(cg.getCallGroup());
    }

    public GetCallGroupsResponse getCallGroups() throws RemoteException {
        GetCallGroupsResponse response = new GetCallGroupsResponse();
        Collection callGroupsColl = m_context.getCallGroups();
        org.sipfoundry.sipxconfig.callgroup.CallGroup[] callGroups =
            (org.sipfoundry.sipxconfig.callgroup.CallGroup[]) callGroupsColl
                .toArray(new org.sipfoundry.sipxconfig.callgroup.CallGroup[callGroupsColl.size()]);
        CallGroup[] arrayOfCallGroups =
            (CallGroup[]) ApiBeanUtil.toApiArray(m_builder, callGroups, CallGroup.class);
        response.setCallGroups(arrayOfCallGroups);
        return response;
    }

}
