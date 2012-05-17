/**
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
package org.sipfoundry.sipxconfig.common;

import java.util.Collection;
import java.util.HashSet;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.context.MessageSource;
import org.springframework.context.MessageSourceResolvable;
import org.springframework.context.NoSuchMessageException;

/**
 * Read resource bundles from all beans that implement MessageSource, including plugins.
 *
 * To register your resource bundle into the global pool, just add bean
 *
 *   <bean id="alarmMessages" class="org.springframework.context.support.ResourceBundleMessageSource">
 *     <property name="basename" value="my.package.xyx"/>
 *   < /bean>
 *
 *   And create file with english translations
 *     src/my/package/xyz.properties
 *   and optionally other files with other language translations following the java resource rules
 *
 *   Code that wished to load strings from global pool can just inject this bean and call one
 *   of the getMessage methods.
 */
public class GlobalMessageSource implements MessageSource, BeanFactoryAware {
    private ListableBeanFactory m_beanFactory;
    private Collection<MessageSource> m_delegates;
    private int m_avoidCheckstyleError;

    @Override
    public String getMessage(MessageSourceResolvable r, Locale l) {
        for (MessageSource d : getDelegates()) {
            try {
                return d.getMessage(r, l);
            } catch (NoSuchMessageException ignore) {
                m_avoidCheckstyleError++;
            }
        }
        throw new NoSuchMessageException(r.toString(), l);
    }

    @Override
    public String getMessage(String s, Object[] p, Locale l) {
        for (MessageSource d : getDelegates()) {
            try {
                return d.getMessage(s, p, l);
            } catch (NoSuchMessageException ignore) {
                m_avoidCheckstyleError++;
            }
        }
        throw new NoSuchMessageException(s, l);
    }

    @Override
    public String getMessage(String s, Object[] p, String s2, Locale l) {
        for (MessageSource d : getDelegates()) {
            try {
                return d.getMessage(s, p, s2, l);
            } catch (NoSuchMessageException ignore) {
                m_avoidCheckstyleError++;
            }
        }
        throw new NoSuchMessageException(s, l);
    }

    @Override
    public void setBeanFactory(BeanFactory bf) {
        m_beanFactory = (ListableBeanFactory) bf;
    }

    Collection<MessageSource> getDelegates() {
        if (m_delegates == null) {
            Map<String, MessageSource> beans = m_beanFactory.getBeansOfType(MessageSource.class);
            Set<MessageSource> copy = new HashSet<MessageSource>(beans.values());
            // otherwise recursive!
            copy.remove(this);
            m_delegates = copy;
        }
        return m_delegates;
    }
}
