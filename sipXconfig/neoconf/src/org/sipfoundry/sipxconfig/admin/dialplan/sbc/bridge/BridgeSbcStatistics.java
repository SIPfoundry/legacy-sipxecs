/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge;


import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.springframework.beans.factory.annotation.Required;

public class BridgeSbcStatistics {

    private ApiProvider<BridgeSbcXmlRpcApi> m_bridgeSbcApiProvider;

    @Required
    public void setBridgeSbcApiProvider(ApiProvider bridgeSbcApiProvider) {
        m_bridgeSbcApiProvider = bridgeSbcApiProvider;
    }

    /**
     * Get a count of the number of ongoing calls.
     *
     * @return the number of ongoing calls.
     * @throws Exception
     */
    public int getCallCount(BridgeSbc bridgeSbc) throws Exception {
        if (!bridgeSbc.isBridgeSbcRunning()) {
            return 0;
        }

        BridgeSbcXmlRpcApi api = m_bridgeSbcApiProvider.getApi(bridgeSbc.getBridgeSbcXmlRpcUrl());
        return api.getCallCount();
    }

    /**
     * Gets an array of Registration records - one record for each account
     * that requires registration.
     *
     * @return an array of registration records.
     * @throws Exception
     */
    public BridgeSbcRegistrationRecord[] getRegistrationRecords(BridgeSbc bridgeSbc) throws Exception {
        if (!bridgeSbc.isBridgeSbcRunning()) {
            return null;
        }

        Map<String, String> registrationRecordMap = null;

        BridgeSbcXmlRpcApi api = m_bridgeSbcApiProvider.getApi(bridgeSbc.getBridgeSbcXmlRpcUrl());
        registrationRecordMap = api.getRegistrationStatus();

        if (registrationRecordMap == null) {
            return null;
        }

        BridgeSbcRegistrationRecord[] registrationRecords = new
            BridgeSbcRegistrationRecord[registrationRecordMap.size()];
        int i = 0;
        Set<String> keys = registrationRecordMap.keySet();
        for (String key : keys) {
            registrationRecords[i++] = new BridgeSbcRegistrationRecord(key, registrationRecordMap.get(key));
        }

        return registrationRecords;
    }
}
