/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.admin;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

/**
 * Backup provides Java interface to backup scripts
 */
public class AdminContextImpl extends HibernateDaoSupport implements AdminContext {
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(HTTP_ADDRESS, HTTPS_ADDRESS,
            TFTP_ADDRESS, FTP_ADDRESS);

    @Override
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        return ADDRESSES;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Object requester) {
        if (!ADDRESSES.contains(type)) {
            return null;
        }
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        List<Address> addresses = new ArrayList<Address>(locations.size());
        for (Location location : locations) {
            Address address = new Address();
            address.setAddress(location.getAddress());
            if (type.equals(HTTP_ADDRESS)) {
                address.setPort(12000);
            } else if (type.equals(HTTPS_ADDRESS)) {
                address.setPort(8443);
            }
            // else ftp and tftp won't have ports defines, 0 means it's assumed to be default
            // also, this assumed admin ui is also tftp and ftp server, which is a correct assumption
            // for now.
        }
        return addresses;
    }

    public String[] getInitializationTasks() {
        List l = getHibernateTemplate().findByNamedQuery("taskNames");
        return (String[]) l.toArray(new String[l.size()]);
    }

    public void deleteInitializationTask(String task) {
        List l = getHibernateTemplate().findByNamedQueryAndNamedParam("taskByName", "task", task);
        getHibernateTemplate().deleteAll(l);
    }

    public boolean inInitializationPhase() {
        String initializationPhase = System.getProperty("sipxconfig.initializationPhase");
        if (initializationPhase == null) {
            return false;
        }

        return Boolean.parseBoolean(initializationPhase);
    }
}
