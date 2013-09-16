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
package org.sipfoundry.sipxconfig.security;

import java.util.Collection;
import java.util.Map;

import org.springframework.beans.BeansException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.security.authentication.AuthenticationServiceException;

public class SystemAuthPolicyCollectorImpl implements BeanFactoryAware {
    private ListableBeanFactory m_beanFactory;
    private Collection<SystemAuthPolicyVerifier> m_verifiers;

    public void verifyPolicy(String username) throws AuthenticationServiceException {
        for (SystemAuthPolicyVerifier verifier : getVerifiers()) {
            verifier.verifyPolicy(username);
        }
    }

    public boolean isExternalXmppAuthOnly() {
        for (SystemAuthPolicyVerifier verifier : getVerifiers()) {
            if (verifier.isExternalXmppAuthOnly()) {
                return true;
            }
        }
        return false;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) throws BeansException {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    private Collection<SystemAuthPolicyVerifier> getVerifiers() {
        if (m_verifiers == null) {
            Map<String, SystemAuthPolicyVerifier> verifiers = m_beanFactory.
                    getBeansOfType(SystemAuthPolicyVerifier.class, false, false);
            m_verifiers = verifiers.values();
        }
        return m_verifiers;
    }
}
