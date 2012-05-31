/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.imbot;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.springframework.web.client.RestClientException;
import org.springframework.web.client.RestTemplate;

public class ImBotManager {
    private static final String ADD_TO_ROSTER_URL = "{address}/{userName}/addToRoster";
    private AddressManager m_addressManager;
    private RestTemplate m_restTemplate;

    public boolean requestToAddMyAssistantToRoster(String userName) {
        Address imbotRestAddress = m_addressManager.getSingleAddress(ImBot.REST_API);
        try {
            m_restTemplate.put(ADD_TO_ROSTER_URL, null, imbotRestAddress.toString(), userName);
        } catch (RestClientException ex) {
            return false;
        }
        return true;

    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    public void setRestTemplate(RestTemplate restTemplate) {
        m_restTemplate = restTemplate;
    }
}
