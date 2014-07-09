/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.api.model;

import java.util.ArrayList;
import java.util.List;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.sipfoundry.sipxconfig.gateway.Gateway;

@XmlRootElement(name = "Gateways")
public class GatewayList {

    private List<GatewayBean> m_gateways;

    public void setGateways(List<GatewayBean> gateways) {
        m_gateways = gateways;
    }

    @XmlElement(name = "Gateway")
    public List<GatewayBean> getGateways() {
        if (m_gateways == null) {
            m_gateways = new ArrayList<GatewayBean>();
        }
        return m_gateways;
    }

    public static GatewayList convertGatewayList(List<Gateway> gateways) {
        List<GatewayBean> gatewayList = new ArrayList<GatewayBean>();
        for (Gateway gateway : gateways) {
            gatewayList.add(GatewayBean.convertGateway(gateway));
        }
        GatewayList list = new GatewayList();
        list.setGateways(gatewayList);
        return list;
    }
}
