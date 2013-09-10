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
package org.sipfoundry.sipxconfig.alarm;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class WarningManagerImpl implements WarningManager, BeanFactoryAware {
    private Set<WarningProvider> m_providers;
    private ListableBeanFactory m_beanFactory;

    @Override
    public List<Warning> getWarnings() {
        List<Warning> warnings = new ArrayList<Warning>();
        for (WarningProvider provider : getWarningProviders()) {
            Collection<Warning> warns = provider.getWarnings();
            if (warns != null) {
                warnings.addAll(warns);
            }
        }
        return warnings;
    }

    Set<WarningProvider> getWarningProviders() {
        if (m_providers == null) {
            Map<String, WarningProvider> beanMap = m_beanFactory.getBeansOfType(
                    WarningProvider.class, false, false);
            m_providers = new HashSet<WarningProvider>(beanMap.values());
        }

        return m_providers;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

}
