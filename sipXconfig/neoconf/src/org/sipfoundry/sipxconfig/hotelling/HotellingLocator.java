/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.hotelling;

import java.util.Map;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class HotellingLocator implements BeanFactoryAware {
    private HotellingManager m_hotellingManager;

    /*
     * return one instance for now (for polycom). In the future, if there are multiple beans
     * defined, we may provide a list of beans (i.e. for more models), and cycle through them and
     * apply .generate() on each.
     */
    public HotellingManager getHotellingBean() {
        return m_hotellingManager;
    }

    public boolean isHotellingEnabled() {
        if (m_hotellingManager == null) {
            return false;
        } else {
            return m_hotellingManager.isActive();
        }
    }

    @Override
    public void setBeanFactory(BeanFactory bf) {
        Map<String, HotellingManager> managers = ((ListableBeanFactory) bf).getBeansOfType(HotellingManager.class);
        if (!managers.isEmpty()) {
            for (String key : managers.keySet()) {
                m_hotellingManager = managers.get(key);
                return;
            }
        }
    }

}
