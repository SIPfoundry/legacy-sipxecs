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
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.sipfoundry.sipxconfig.gateway.FxoPort;


@XmlRootElement(name = "Port")

public class FxoPortBean {
    private int m_id;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public static FxoPortBean convertFxoPortBean(FxoPort fxoPort) {
        FxoPortBean fxoPortBean = new FxoPortBean();
        fxoPortBean.setId(fxoPort.getId());
        return fxoPortBean;
    }

    public static List<FxoPortBean> buildFxoPortList(Collection<FxoPort> fxoPorts) {
        List<FxoPortBean> ports = new LinkedList<FxoPortBean>();
        for (FxoPort fxoPort : fxoPorts) {
            ports.add(convertFxoPortBean(fxoPort));
        }
        if (ports.size() > 0) {
            return ports;
        }
        return null;
    }

    @XmlRootElement(name = "Ports")
    public static class FxoPortList {

        private List<FxoPortBean> m_ports;

        public void setPorts(List<FxoPortBean> ports) {
            m_ports = ports;
        }

        @XmlElement(name = "Port")
        public List<FxoPortBean> getModels() {
            if (m_ports == null) {
                m_ports = new ArrayList<FxoPortBean>();
            }
            return m_ports;
        }

        public static FxoPortList convertPortsList(Collection<FxoPort> ports) {
            FxoPortList list = new FxoPortList();
            list.setPorts(FxoPortBean.buildFxoPortList(ports));
            return list;
        }
    }
}
