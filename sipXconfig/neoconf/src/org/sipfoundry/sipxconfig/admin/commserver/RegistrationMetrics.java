/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.collections.Closure;
import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.apache.commons.collections.functors.ConstantFactory;
import org.apache.commons.collections.map.LazyMap;
import org.apache.commons.collections.map.LinkedMap;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.RegistrationItem;

/**
 * Metrics about registration distributions
 */
public class RegistrationMetrics {
    private Collection m_uniqueRegistrations;
    private long m_startTime;

    /**
     * All registrations from registration.xml which may include redunant
     * registrations from clients that reregister w/slighlty different uri
     * w/o unregistering last uri.
     */
    public void setRegistrations(List registrations) {
        UniqueRegistrations unique = new UniqueRegistrations();
        CollectionUtils.forAllDo(registrations, unique);
        setUniqueRegistrations(unique.getRegistrations());
    }

    public void setStartTime(long startTime) {
        m_startTime = startTime;
    }

    public double getLoadBalance() {
        LoadDistribution metric = new LoadDistribution();
        // decided to count expired registrations, shouldn't matter and more history
        // gives a more accurate value.
        CollectionUtils.forAllDo(m_uniqueRegistrations, metric);
        double loadBalance = metric.getLoadBalance();
        return loadBalance;
    }

    public int getActiveRegistrationCount() {
        int count = CollectionUtils.countMatches(m_uniqueRegistrations, new ActiveRegistrations(
                m_startTime));
        return count;
    }

    public Collection getUniqueRegistrations() {
        return m_uniqueRegistrations;
    }

    void setUniqueRegistrations(Collection registrations) {
        m_uniqueRegistrations = registrations;
    }

    /**
     * Filter out multiple registrations for a single contact
     */
    static class UniqueRegistrations implements Closure {
        private Map<String, RegistrationItem> m_contact2registration = new LinkedMap();

        public Collection getRegistrations() {
            return m_contact2registration.values();
        }

        public void execute(Object input) {
            RegistrationItem ri = (RegistrationItem) input;
            String contact = ri.getContact();
            RegistrationItem riOld = m_contact2registration.get(contact);
            // replace older registrations
            if (riOld == null || ri.compareTo(riOld) > 0) {
                m_contact2registration.put(contact, ri);
            }
        }
    }

    /**
     * Filter out expired registrations
     */
    static class ActiveRegistrations implements Predicate {
        private long m_startTime;

        ActiveRegistrations(long startTime) {
            m_startTime = startTime;
        }

        public boolean evaluate(Object input) {
            RegistrationItem reg = (RegistrationItem) input;
            if (reg.timeToExpireAsSeconds(m_startTime) > 0) {
                return true;
            }
            return false;
        }
    }

    /**
     * Calculate how many registrations originated on each regististrar
     */
    static class LoadDistribution implements Closure {

        private long m_total;

        private Map<String, Integer> m_distribution = LazyMap.decorate(
                new HashMap<String, Integer>(), ConstantFactory.getInstance(0));

        public int getRegistrationCount(String server) {
            return m_distribution.get(server);
        }

        public void execute(Object input) {
            RegistrationItem reg = (RegistrationItem) input;
            String primary = reg.getPrimary();
            int count = m_distribution.get(primary);
            m_distribution.put(primary, count + 1);
            m_total++;
        }

        public double getLoadBalance() {
            double loadBalanceInverse = 0;
            for (int m : m_distribution.values()) {
                double ratioSquared = Math.pow((double) m / m_total, 2);
                loadBalanceInverse += ratioSquared;
            }
            if (loadBalanceInverse == 0) {
                return 1;
            }
            return 1 / loadBalanceInverse;
        }
    }
}
