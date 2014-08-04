/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.dns;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;
import java.security.InvalidParameterException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.SimpleCommandRunner;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.region.Region;
import org.sipfoundry.sipxconfig.region.RegionManager;

public class DnsTestContextImpl implements DnsTestContext {
    private static final String EMPTY = "";
    private static final String RESOURCE_FORMAT = "%s%s";
    private static final String DOT = ".";
    private static final String SRV = "SRV";
    private static final String NEW_LINE = "\n";
    private static final Log LOG = LogFactory.getLog(DnsTestContextImpl.class);
    private String m_validatorScript;
    private String m_validatorScriptRegions;
    private RegionManager m_regionManager;
    private LocationsManager m_locationsManager;
    private DnsManager m_dnsManager;
    private ConfigManager m_configManager;
    private DomainManager m_domainManager;

    @Override
    public String missingRecords(String server) {
        File missingRecords = null;
        Reader rdr = null;
        String commandLine = StringUtils.EMPTY;
        try {
            missingRecords = File.createTempFile("dns-test", ".tmp");
            String[] cmd = new String[] {
                m_validatorScript, "--server", server, "--out", missingRecords.getAbsolutePath()
            };
            LOG.info(StringUtils.join(cmd, ' '));
            ProcessBuilder pb = new ProcessBuilder(cmd);
            Process process = pb.start();
            int code = process.waitFor();
            if (code != 0) {
                String errorMsg = String
                        .format("DNS validation command %s failed. Exit code: %d", commandLine, code);
                throw new RuntimeException(errorMsg);
            }
            rdr = new FileReader(missingRecords);
            return IOUtils.toString(rdr);
        } catch (IOException e) {
            String errorMsg = String.format("Error running archive command %s.", commandLine);
            throw new RuntimeException(errorMsg);
        } catch (InterruptedException e) {
            String errorMsg = String.format("Timed out running archive command %s.", commandLine);
            throw new RuntimeException(errorMsg);
        } finally {
            IOUtils.closeQuietly(rdr);
            if (missingRecords != null) {
                missingRecords.delete();
            }
        }
    }

    @Override
    public String missingRecords(Region region, String dnsHost) {
        // get a host from the region, ideally primary
        String remoteHost = null;
        for (Integer locationId : m_regionManager.getServersByRegion(region.getId())) {
            Location location = m_locationsManager.getLocation(locationId);
            remoteHost = location.getAddress();
            if (location.isPrimary()) {
                break;
            }
        }
        Map<String, List<PrivateResourceRecord>> resultRrs = new HashMap<String, List<PrivateResourceRecord>>();
        Map<String, List<PrivateResourceRecord>> rrs = getComputedRecords(region.getId());
        for (String rr : rrs.keySet()) {
            SimpleCommandRunner runner = createRemoteCommand(remoteHost, dnsHost, rr, m_domainManager.getDomain()
                    .getName(), SRV, EMPTY);
            LOG.debug("Running: " + runner.toString());
            boolean done = runner.run();
            if (done) {
                if (runner.getExitCode() != 0) {
                    throw new UserException("Command did not complete properly. " + runner.getStderr());
                }
            }
            resultRrs.putAll(parseOutput(rr, StringUtils.remove(runner.getStdout(), m_validatorScriptRegions),
                    m_domainManager.getDomainName()));
        }

        LOG.debug("Computed: " + rrs.toString());
        LOG.debug("DIGged:   " + resultRrs.toString());
        String missing = extractMissingRecords(rrs, resultRrs);
        LOG.debug("Missing: " + missing);
        return extractMissingRecords(rrs, resultRrs);
    }

    private Map<String, List<PrivateResourceRecord>> getComputedRecords(int regionId) {
        Map<String, List<PrivateResourceRecord>> rrs = new HashMap<String, List<PrivateResourceRecord>>();
        for (ResourceRecords resourceRecords : m_dnsManager.getResourceRecords()) {
            List<PrivateResourceRecord> srvRecords = new ArrayList<PrivateResourceRecord>();
            for (ResourceRecord rr : resourceRecords.getRecords()) {
                if (rr.getRegionId() == regionId) {
                    PrivateResourceRecord prvSrvRecord;
                    prvSrvRecord = new PrivateResourceRecord(resourceRecords.getProto(),
                            resourceRecords.getResource(), EMPTY, rr.getPort(), rr.getAddress());
                    prvSrvRecord.setKey(String.format(RESOURCE_FORMAT, resourceRecords.getProto(), StringUtils
                            .isNotBlank(resourceRecords.getResource()) ? (DOT + resourceRecords.getResource())
                            : EMPTY));
                    srvRecords.add(prvSrvRecord);
                }
                rrs.put(String.format(RESOURCE_FORMAT, resourceRecords.getProto(), StringUtils
                        .isNotBlank(resourceRecords.getResource()) ? (DOT + resourceRecords.getResource()) : EMPTY),
                        srvRecords);
            }
        }
        return rrs;
    }

    private static String extractMissingRecords(Map<String, List<PrivateResourceRecord>> computed,
            Map<String, List<PrivateResourceRecord>> digged) {
        StringBuilder missing = new StringBuilder();
        for (String rr : computed.keySet()) {
            for (PrivateResourceRecord prr : computed.get(rr)) {
                if (!digged.get(rr).contains(prr)) {
                    missing.append(prr);
                }
            }
        }
        return missing.toString();
    }

    private SimpleCommandRunner createRemoteCommand(String remoteHost, String dnsHost, String record, String domain,
            String type, String out) {
        if (StringUtils.isBlank(record)) {
            throw new InvalidParameterException(
                    "Bad command calling sipx dns validator script: record cannot be empty.");
        }
        SimpleCommandRunner runner = new SimpleCommandRunner();
        StringBuilder cmd = new StringBuilder(m_configManager.getRemoteCommand(remoteHost));
        cmd.append(' ').append(m_validatorScriptRegions);
        if (StringUtils.isNotBlank(dnsHost)) {
            cmd.append(" --host ").append(dnsHost);
        }
        if (StringUtils.isNotBlank(type)) {
            cmd.append(" --type ").append(type);
        }
        if (StringUtils.isNotBlank(out)) {
            cmd.append(" --out ").append(out);
        }
        cmd.append(" --record ").append(record);
        cmd.append(" --domain ").append(domain);
        String command = cmd.toString();
        runner.setRunParameters(command, 1000, 0);
        return runner;
    }

    private Map<String, List<PrivateResourceRecord>> parseOutput(String rr, String out, String domain) {
        Map<String, List<PrivateResourceRecord>> rrs = new HashMap<String, List<PrivateResourceRecord>>();
        List<PrivateResourceRecord> hosts = new ArrayList<PrivateResourceRecord>();
        String[] lines = StringUtils.split(out, NEW_LINE);
        for (int i = 0; i < lines.length; i++) {
            String host = StringUtils.removeEnd(lines[i], DOT + domain + DOT);
            PrivateResourceRecord prvRr = new PrivateResourceRecord(EMPTY, EMPTY, EMPTY, 0, host);
            prvRr.setKey(rr);
            hosts.add(prvRr);
            LOG.debug(prvRr);
        }
        rrs.put(rr, hosts);
        return rrs;
    }

    private class PrivateResourceRecord extends DnsSrvRecord {
        private static final String SPACE = " ";
        private String m_key;

        public PrivateResourceRecord(String protocol, String resource, String host, int port, String destination) {
            super(protocol, resource, host, port, destination);
        }

        @Override
        public String toString() {
            return m_key + "\tIN\tSRV\t" + getPriority() + SPACE + getWeight() + SPACE + getPort() + SPACE
                    + getDestination() + NEW_LINE;
        }

        @Override
        public int hashCode() {
            final int prime = 31;
            int result = 1;
            result = prime * result + ((getDestination() == null) ? 0 : getDestination().hashCode());
            result = prime * result + ((m_key == null) ? 0 : m_key.hashCode());
            return result;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) {
                return true;
            }
            if (obj == null) {
                return false;
            }
            if (getClass() != obj.getClass()) {
                return false;
            }
            PrivateResourceRecord other = (PrivateResourceRecord) obj;
            if (getDestination() == null) {
                if (other.getDestination() != null) {
                    return false;
                }
            } else if (!getDestination().equals(other.getDestination())) {
                return false;
            }
            if (m_key == null) {
                if (other.m_key != null) {
                    return false;
                }
            } else if (!m_key.equals(other.m_key)) {
                return false;
            }
            return true;
        }

        public void setKey(String key) {
            m_key = key;
        }
    }

    public void setValidatorScript(String validatorScript) {
        m_validatorScript = validatorScript;
    }

    public void setValidatorScriptRegions(String validatorScriptRegions) {
        m_validatorScriptRegions = validatorScriptRegions;
    }

    public void setRegionManager(RegionManager regionManager) {
        m_regionManager = regionManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setDnsManager(DnsManager dnsManager) {
        m_dnsManager = dnsManager;
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
}
