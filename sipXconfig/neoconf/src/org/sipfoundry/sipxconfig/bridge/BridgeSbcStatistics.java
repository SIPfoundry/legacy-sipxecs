/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.bridge;

import java.util.List;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.springframework.beans.factory.annotation.Required;

public class BridgeSbcStatistics {
    private ApiProvider<BridgeSbcXmlRpcApi> m_bridgeSbcApiProvider;
    private SnmpManager m_snmpManager;

    /**
     * Get a count of the number of ongoing calls.
     *
     * @return the number of ongoing calls.
     * @throws Exception
     */
    public int getCallCount(BridgeSbc bridgeSbc) throws Exception {
        BridgeSbcXmlRpcApi api = getApi(bridgeSbc);
        return (api == null ? 0 : api.getCallCount());
    }

    boolean isOk(BridgeSbc bridgeSbc) {
        // Not sure what to check? process configed? running? ---Douglas
        List<ServiceStatus> stats = m_snmpManager.getServicesStatuses(bridgeSbc.getLocation());
        for (ServiceStatus status : stats) {
            if (status.getServiceBeanId().equals("sipxbridge")
                    && status.getStatus().equals(ServiceStatus.Status.Running)) {
                return true;
            }
        }
        return false;
    }

    BridgeSbcXmlRpcApi getApi(BridgeSbc bridgeSbc) {
        if (!isOk(bridgeSbc)) {
            return null;
        }
        Address address = BridgeSbcContext.newSbcAddress(bridgeSbc, BridgeSbcContext.XMLRPC_ADDRESS);
        BridgeSbcXmlRpcApi api = m_bridgeSbcApiProvider.getApi(address.toString());
        return api;
    }

    /**
     * Gets an array of Registration records - one record for each account that requires
     * registration.
     *
     * @return an array of registration records.
     * @throws Exception
     */
    public BridgeSbcRegistrationRecord[] getRegistrationRecords(BridgeSbc bridgeSbc) throws Exception {
        BridgeSbcXmlRpcApi api = getApi(bridgeSbc);
        if (api == null) {
            return null;
        }

        Map<String, String> registrationRecordMap = null;
        registrationRecordMap = api.getRegistrationStatus();
        if (registrationRecordMap == null) {
            return null;
        }

        BridgeSbcRegistrationRecord[] registrationRecords = new BridgeSbcRegistrationRecord[registrationRecordMap
                .size()];
        int i = 0;
        Set<String> keys = registrationRecordMap.keySet();
        for (String key : keys) {
            registrationRecords[i++] = new BridgeSbcRegistrationRecord(key, registrationRecordMap.get(key));
        }

        return registrationRecords;
    }

    @Required
    public void setBridgeSbcApiProvider(ApiProvider bridgeSbcApiProvider) {
        m_bridgeSbcApiProvider = bridgeSbcApiProvider;
    }

    @Required
    public void setSnmpManager(SnmpManager snmpManager) {
        m_snmpManager = snmpManager;
    }

}
