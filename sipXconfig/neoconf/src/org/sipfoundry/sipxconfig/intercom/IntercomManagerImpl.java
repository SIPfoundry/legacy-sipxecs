/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.intercom;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import org.apache.commons.collections.CollectionUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.dialplan.IntercomRule;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class IntercomManagerImpl extends SipxHibernateDaoSupport implements IntercomManager, BeanFactoryAware,
        FeatureProvider, FeatureListener {

    public static final String CONTEXT_BEAN_NAME = "intercomManagerImpl";
    private ConfigManager m_configManager;

    private BeanFactory m_beanFactory;

    public Intercom newIntercom() {
        return (Intercom) m_beanFactory.getBean(Intercom.class.getName());
    }

    /**
     * Return an Intercom instance. Create one if none exist. Throw UserException if more than one
     * Intercom instance exists, since the caller is assuming that there can be at most one
     * Intercom instance.
     */
    public Intercom getIntercom() {
        List intercoms = loadIntercoms();
        int numIntercoms = intercoms.size();
        if (numIntercoms == 0) {
            return newIntercom();
        } else if (numIntercoms == 1) {
            return (Intercom) intercoms.get(0);
        } else {
            throw new UserException("Expecting at most 1 Intercom, but there are " + numIntercoms);
        }
    }

    public void saveIntercom(Intercom intercom) {
        getHibernateTemplate().saveOrUpdate(intercom);
    }

    public List<Intercom> loadIntercoms() {
        return getHibernateTemplate().loadAll(Intercom.class);
    }

    /**
     * Remove all intercoms - mostly used for testing
     */
    public void clear() {
        HibernateTemplate template = getHibernateTemplate();
        Collection<Intercom> intercoms = template.loadAll(Intercom.class);
        getDaoEventPublisher().publishDeleteCollection(intercoms);
        template.deleteAll(intercoms);
    }

    /**
     * Return the intercom associated with a phone, through the groups the phone belongs to, or
     * null if there is no intercom for the phone. There should be at most one intercom for any
     * phone. If there is more than one, then return the first intercom found.
     */
    public Intercom getIntercomForPhone(Phone phone) {
        Set phoneGroups = phone.getGroups();
        for (Intercom intercom : loadIntercoms()) {
            Set intercomGroups = intercom.getGroups();
            if (CollectionUtils.containsAny(phoneGroups, intercomGroups)) {
                return intercom;
            }
        }
        return null;
    }

    /** Return a list of dialing rules, one for each intercom configuration */
    @Override
    public List<DialingRule> getDialingRules(Location location) {
        List<Intercom> intercoms = loadIntercoms();
        List<DialingRule> rules = new ArrayList<DialingRule>(intercoms.size());
        for (Intercom intercom : intercoms) {
            DialingRule rule = new IntercomRule(intercom);
            rules.add(rule);
        }
        return rules;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return Collections.singleton(FEATURE);
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return null;
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.ADVANCED_TELEPHONY) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public void enableLocationFeature(FeatureManager manager, FeatureEvent event, LocationFeature feature,
            Location location) {
    }

    @Override
    public void enableGlobalFeature(FeatureManager manager, FeatureEvent event, GlobalFeature feature) {
        if (!feature.equals(IntercomManager.FEATURE)) {
            return;
        }

        Intercom intercom = getIntercom();
        if (!intercom.isNew()) {
            switch (event) {
            case POST_ENABLE:
                intercom.setEnabled(true);
                saveIntercom(intercom);
                m_configManager.configureEverywhere(DialPlanContext.FEATURE);
                break;
            case PRE_DISABLE:
                intercom.setEnabled(false);
                saveIntercom(intercom);
                m_configManager.configureEverywhere(DialPlanContext.FEATURE);
            default:
                break;
            }
        }
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }
}
