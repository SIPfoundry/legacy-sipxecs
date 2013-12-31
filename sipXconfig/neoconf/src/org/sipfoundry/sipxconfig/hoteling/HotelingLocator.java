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
package org.sipfoundry.sipxconfig.hoteling;

import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class HotelingLocator implements BeanFactoryAware {
    private static final Log LOG = LogFactory.getLog(HotelingLocator.class);
    private HotelingManager m_hotelingManager;

    /*
     * return one instance for now (for polycom). In the future, if there are multiple beans
     * defined, we may provide a list of beans (i.e. for more models), and cycle through them and
     * apply .generate() on each.
     */
    public HotelingManager getHotellingBean() {
        return m_hotelingManager;
    }

    public boolean isHotellingEnabled() {
        if (m_hotelingManager == null) {
            return false;
        }
        return true;
    }

    @Override
    public void setBeanFactory(BeanFactory bf) {
        Map<String, HotelingManager> managers = ((ListableBeanFactory) bf).getBeansOfType(HotelingManager.class);
        if (!managers.isEmpty()) {
            /*
             * see above comment. Only one manager is supported for now.
             */
            if (managers.size() > 1) {
                LOG.error("Multiple hoteling managers declared, but only one is supported!");
            }
            for (String key : managers.keySet()) {
                m_hotelingManager = managers.get(key);
                return;
            }
        }
    }

}
