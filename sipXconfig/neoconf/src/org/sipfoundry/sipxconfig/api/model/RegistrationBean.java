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

import org.apache.commons.beanutils.BeanUtils;
import org.sipfoundry.sipxconfig.commserver.imdb.RegistrationItem;

@XmlRootElement(name = "Registration")
public class RegistrationBean extends RegistrationItem {
    private String m_status;
    private long m_timeToExpire;

    public String getStatus() {
        return m_status;
    }

    public void setStatus(String status) {
        m_status = status;
    }

    public long getSecondsToExpire() {
        return m_timeToExpire;
    }

    public void setSecondsToExpire(long expire) {
        m_timeToExpire = expire;
    }

    public static RegistrationBean convertRegistration(RegistrationItem item, long now) throws Exception {
        RegistrationBean bean = new RegistrationBean();
        BeanUtils.copyProperties(bean, item);
        long timeToExpire = item.timeToExpireAsSeconds(now);
        if (timeToExpire > 0) {
            bean.setStatus("active");
            bean.setSecondsToExpire(timeToExpire);
        } else {
            bean.setStatus("expired");
        }
        return bean;
    }

    public static List<RegistrationBean> buildRegistrationList(Collection<RegistrationBean> items, long now) {
        try {
            List<RegistrationBean> registrations = new LinkedList<RegistrationBean>();
            for (RegistrationItem item : items) {
                registrations.add(convertRegistration(item, now));
            }
            if (registrations.size() > 0) {
                return registrations;
            }
        } catch (Exception ex) {
            return null;
        }
        return null;
    }

    @XmlRootElement(name = "Registrations")
    public static class RegistrationList {

        private List<RegistrationBean> m_registrations;

        public void setRegistrations(List<RegistrationBean> regs) {
            m_registrations = regs;
        }

        @XmlElement(name = "Registration")
        public List<RegistrationBean> getRegistrations() {
            if (m_registrations == null) {
                m_registrations = new ArrayList<RegistrationBean>();
            }
            return m_registrations;
        }

        public static RegistrationList convertRegistrationList(Collection<RegistrationBean> regs, long now) {
            RegistrationList list = new RegistrationList();
            list.setRegistrations(RegistrationBean.buildRegistrationList(regs, now));
            return list;
        }
    }
}
