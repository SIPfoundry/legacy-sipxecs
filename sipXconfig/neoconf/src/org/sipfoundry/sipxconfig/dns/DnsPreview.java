/**
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
package org.sipfoundry.sipxconfig.dns;

import static java.lang.String.format;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Collection;
import java.util.List;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.common.SimpleCommandRunner;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.domain.Domain;

public class DnsPreview {
    private static final Log LOG = LogFactory.getLog(DnsPreview.class);
    private DnsManager m_dnsManager;
    private DnsConfig m_dnsConfig;
    private AddressManager m_addressManager;
    private LocationsManager m_locationsManager;
    private String m_command;
    private int m_commandTimeout = 1000;
    public enum Show {
        PUBLIC, ALL
    }

    @SuppressWarnings("unchecked")
    public String getZone(DnsView view, final Show show) {
        File customRecordsFile = null;
        File zoneFile = null;
        Writer zoneStream = null;
        List<Address> dns = m_addressManager.getAddresses(DnsManager.DNS_ADDRESS, null);
        List<Location> all = m_locationsManager.getLocationsList();
        long serNo = m_dnsConfig.getSerNo();
        try {
            Collection<DnsSrvRecord> srvs = m_dnsManager.getResourceRecords(view);
            srvs = (Collection<DnsSrvRecord>) CollectionUtils.select(srvs, new Predicate() {
                @Override
                public boolean evaluate(Object arg0) {
                    DnsSrvRecord srv = (DnsSrvRecord) arg0;
                    switch (show) {
                    case PUBLIC:
                        return !srv.isInternal();
                    default:
                        return true;
                    }
                }
            });
            Domain domain = Domain.getDomain();
            zoneFile = File.createTempFile("dns_preview", ".yml");
            zoneStream = new FileWriter(zoneFile);
            m_dnsConfig.writeZoneConfig(zoneStream, domain, all, dns, srvs, serNo);
            customRecordsFile = File.createTempFile("dns_preview_custom", ".dat");
            m_dnsConfig.writeZoneCustomRecords(customRecordsFile, view);
            SimpleCommandRunner commandRunner = new SimpleCommandRunner();
            String command = format(m_command, zoneFile.getAbsolutePath(), customRecordsFile.getAbsolutePath());
            commandRunner.setRunParameters(command, m_commandTimeout);
            IOUtils.closeQuietly(zoneStream);
            zoneStream = null;
            commandRunner.run();
            Integer exitCode = commandRunner.getExitCode();
            if (exitCode != 0) {
                StringBuilder msg = new StringBuilder();
                msg.append("Exit code ").append(exitCode).append(".\n");
                msg.append(commandRunner.getStderr());
                return msg.toString();
            }
            return commandRunner.getStdout();
        } catch (IOException e) {
            LOG.error(e);
            return e.toString();
        } finally {
            IOUtils.closeQuietly(zoneStream);
            if (zoneFile != null && zoneFile.exists()) {
                zoneFile.delete();
            }
            if (customRecordsFile != null && customRecordsFile.exists()) {
                customRecordsFile.delete();
            }
        }
    }

    public void setDnsManager(DnsManager dnsManager) {
        m_dnsManager = dnsManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    public void setCommand(String command) {
        m_command = command;
    }

    public void setDnsConfig(DnsConfig dnsConfig) {
        m_dnsConfig = dnsConfig;
    }

    public void setCommandTimeout(int commandTimeout) {
        m_commandTimeout = commandTimeout;
    }
}
