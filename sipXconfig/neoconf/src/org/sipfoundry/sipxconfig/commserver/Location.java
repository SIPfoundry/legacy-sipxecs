/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver;


import static org.apache.commons.lang.StringUtils.substringBefore;

import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.collections.Transformer;
import org.apache.commons.lang.RandomStringUtils;
import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.EnumUserType;
import org.sipfoundry.sipxconfig.common.event.KeepsOriginalCopy;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeType;
import org.sipfoundry.sipxconfig.systemaudit.SystemAuditable;

public class Location extends BeanWithId implements KeepsOriginalCopy<Location>, DeployConfigOnEdit,
        Comparable<Location>, SystemAuditable {
    // security role
    public static final String ROLE_LOCATION = "ROLE_LOCATION";
    public static final int PROCESS_MONITOR_PORT = 8092;
    public static final Transformer GET_ADDRESS = new Transformer() {
        @Override
        public Object transform(Object o) {
            return (o == null ? null : ((Location) o).getAddress());
        }
    };
    public static final Transformer GET_FQDN = new Transformer() {
        @Override
        public Object transform(Object o) {
            return (o == null ? null : ((Location) o).getFqdn());
        }
    };
    public static final Transformer GET_HOSTNAME = new Transformer() {
        @Override
        public Object transform(Object o) {
            return (o == null ? null : ((Location) o).getHostname());
        }
    };
    private static final int SOFTWARE_ADMIN_PORT = 8092;
    private static final int HTTPS_SERVER_PORT = 8092;
    private static final int OPENFIRE_CONTACT_INFO_UPDATE_PORT = 9099;
    private static final int LOCATION_PASSWORD_LEN = 8;
    private static final String XML_RPC_URL = "https://%s:%d/RPC2";
    private static final String DOT = ".";

    private String m_name;
    private String m_address;
    private String m_fqdn;
    private String m_password = RandomStringUtils.randomAlphanumeric(LOCATION_PASSWORD_LEN);
    private boolean m_primary;
    private boolean m_registered;
    private boolean m_callTraffic;
    private State m_state = State.UNCONFIGURED;
    private Timestamp m_lastAttempt;
    private Set<String> m_failedReplications;
    private Branch m_branch;
    private boolean m_useStun = true;
    private String m_stunAddress = "stun.ezuce.com";
    private int m_stunInterval = 60; // seconds
    private String m_publicAddress;
    private int m_publicPort = 5060;
    private int m_publicTlsPort = 5061;
    private int m_startRtpPort = 30000;
    private int m_stopRtpPort = 31000;
    private String m_hostName;
    private Integer m_regionId;
    private Location m_originalCopy;

    public Location() {
    }

    public Location(String fqdn) {
        m_fqdn = fqdn;
    }

    public Location(String fqdn, String address) {
        this(fqdn);
        m_address = address;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getAddress() {
        return m_address;
    }

    public void setAddress(String address) {
        m_address = address;
    }

    public String getFqdn() {
        return m_fqdn;
    }

    public void setFqdn(String fqdn) {
        m_fqdn = fqdn;
    }


    public Set<String> getFailedReplications() {
        return m_failedReplications;
    }

    public void setFailedReplications(Set<String> failedReplications) {
        m_failedReplications = failedReplications;
    }

    public void setUseStun(boolean useStun) {
        m_useStun = useStun;
    }

    public boolean isUseStun() {
        return m_useStun;
    }

    public String getPublicAddress() {
        return m_publicAddress;
    }

    public void setPublicAddress(String publicAddress) {
        m_publicAddress = publicAddress;
    }

    public int getPublicPort() {
        return m_publicPort;
    }

    public void setPublicPort(int publicPort) {
        m_publicPort = publicPort;
    }

    public int getPublicTlsPort() {
        return m_publicTlsPort;
    }

    public void setPublicTlsPort(int publicTlsPort) {
        m_publicTlsPort = publicTlsPort;
    }

    public int getStartRtpPort() {
        return m_startRtpPort;
    }

    public void setStartRtpPort(int startRtpPort) {
        m_startRtpPort = startRtpPort;
    }

    public int getStopRtpPort() {
        return m_stopRtpPort;
    }

    public void setStopRtpPort(int stopRtpPort) {
        m_stopRtpPort = stopRtpPort;
    }

    public String getStunAddress() {
        return m_stunAddress;
    }

    public void setStunAddress(String stunAddress) {
        m_stunAddress = stunAddress;
    }

    public int getStunInterval() {
        return m_stunInterval;
    }

    public void setStunInterval(int stunInterval) {
        m_stunInterval = stunInterval;
    }

    /**
     * Sets this instances address field based on the value parsed from the given URL. For
     * example, the URL of "https://localhost:8091/cgi-bin/replication/replication.cgi" will
     * result in an address value of "localhost"
     *
     * This method is used during migrating from old topology.xml format only.
     *
     * @param url The URL to parse, either the process monitor url or the replication url
     */
    @Deprecated
    public void setUrl(String url) {
        Pattern addressPattern = Pattern.compile("^http[s]?://([a-zA-Z0-9\\.]+):.*$");
        Matcher matcher = addressPattern.matcher(url);
        matcher.matches();
        String address = matcher.group(1);
        setFqdn(address);
    }

    public String getProcessMonitorUrl() {
        return String.format(XML_RPC_URL, m_fqdn, PROCESS_MONITOR_PORT);
    }

    public String getSoftwareAdminUrl() {
        return String.format(XML_RPC_URL, m_fqdn, SOFTWARE_ADMIN_PORT);
    }

    public String getHttpsServerUrl() {
        return String.format("https://%s:%d", m_fqdn, HTTPS_SERVER_PORT);
    }

    public String getXmppContactInfoUpdateUrl() {
        return String.format("http://%s:%d/xmlrpc", m_fqdn, OPENFIRE_CONTACT_INFO_UPDATE_PORT);
    }

    public Branch getBranch() {
        return m_branch;
    }

    public void setBranch(Branch branch) {
        m_branch = branch;
    }

    /**
     * DAOEventListeners might only care is an IP address or FQDN changed.  If so, allow them to check the status
     * of that flag.
     */
    public boolean hasFqdnOrIpChangedOnSave() {
        if (m_originalCopy == null) {
            return true;
        }
        return !isSame(m_hostName, m_originalCopy.m_hostName) || !isSame(m_address, m_originalCopy.m_address);
    }

    boolean isSame(String a, String b) {
        return a == null ? b == null : a.equals(b);
    }

    public String getPassword() {
        return m_password;
    }

    public void setPassword(String password) {
        m_password = password;
    }

    /**
     * Get the hostname portion of the fully qualified domain name
     */
    public String getHostname() {
        if (m_hostName == null) {
            m_hostName = substringBefore(m_fqdn, DOT + Domain.getDomain().getNetworkName());
        }
        return m_hostName;
    }

    public String getHostnameInSipDomain() {
        String sipDomainName = Domain.getDomain().getName();
        if (m_fqdn.equals(sipDomainName)) {
            return m_fqdn;
        }
        return getHostname() + DOT + sipDomainName;
    }

    public boolean isPrimary() {
        return m_primary;
    }

    public void setPrimary(boolean primary) {
        m_primary = primary;
    }

    public boolean isRegistered() {
        return m_registered;
    }

    public void setRegistered(boolean registered) {
        m_registered = registered;
    }

    public State getState() {
        if (!isRegistered()) {
            return State.UNINITIALIZED;
        } else {
            return m_state;
        }
    }

    public void setState(State state) {
        m_state = state;
    }

    public Timestamp getLastAttempt() {
        return m_lastAttempt;
    }

    public void setLastAttempt(Timestamp lastAttempt) {
        m_lastAttempt = lastAttempt;
    }

    public Integer getRegionId() {
        return m_regionId;
    }

    public void setRegionId(Integer regionId) {
        m_regionId = regionId;
    }

    public boolean isInProgressState() {
        return getState().equals(State.PROGRESS);
    }

    public boolean isInConfigurationErrorState() {
        return getState().equals(State.CONFIGURATION_ERROR);
    }

    public boolean isConfigured() {
        return getState().equals(State.CONFIGURED);
    }

    public boolean isInNotFinishedState() {
        return getState().equals(State.NOT_FINISHED);
    }

    public boolean isUninitialized() {
        return getState().equals(State.UNINITIALIZED);
    }

    public static final class State extends Enum {
        public static final State CONFIGURATION_ERROR = new State("CONFIGURATION_ERROR");
        public static final State UNINITIALIZED = new State("UNINITIALIZED");
        public static final State UNCONFIGURED = new State("UNCONFIGURED");
        public static final State CONFIGURED = new State("CONFIGURED");
        public static final State PROGRESS = new State("PROGRESS");
        public static final State NOT_FINISHED = new State("NOT_FINISHED");
        public static final State DISABLED = new State("DISABLED");

        public State(String name) {
            super(name);
        }

        public static State getEnum(String type) {
            return (State) getEnum(State.class, type);
        }
    }

    /**
     * Used for Hibernate type translation
     */
    public static class LocationState extends EnumUserType {
        public LocationState() {
            super(State.class);
        }
    }

    /**
     * The default number of mongo replications for a location.
     * Used only to retrieve replication percentage when profiles are sent.
     * @return 2 if the location is primary (db regeneration and location registration)
     * and 1 if the location is slave (only location regeneration is performed)
     */
    public String[] getDefaultMongoReplications() {
        if (isPrimary()) {
            return new String[] {SipxReplicationContext.IMDB_REGENERATION,
                SipxReplicationContext.MONGO_LOCATION_REGISTRATION};
        }
        return new String[] {SipxReplicationContext.MONGO_LOCATION_REGISTRATION};
    }

    public boolean isCallTraffic() {
        return m_callTraffic;
    }

    public void setCallTraffic(boolean callTraffic) {
        m_callTraffic = callTraffic;
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) LocationsManager.FEATURE);
    }

    /**
     * Convenience method to turn collection of locations into addresses
     */
    public static List<Address> toAddresses(AddressType t, Collection<Location> locations) {
        return toAddresses(t, locations, -1);
    }

    public static List<Address> toAddresses(AddressType t, Collection<Location> locations, int port) {
        if (locations == null || locations.size() == 0) {
            return Collections.emptyList();
        }
        List<Address> addresses = new ArrayList<Address>(locations.size());
        for (Location l : locations) {
            Address a = (port > 0 ? new Address(t, l.getAddress(), port) : new Address(t, l.getAddress()));
            addresses.add(a);
        }
        return addresses;
    }

    @Override
    public int compareTo(Location o) {
        if (o.isPrimary() && isPrimary()) {
            return 0;
        } else if (isPrimary()) {
            return -1;
        } else if (o.isPrimary()) {
            return 1;
        }
        return getId() - o.getId();
    }

    @Override
    public String toString() {
        return "Location [m_name=" + m_name + ", m_address=" + m_address + ", m_fqdn=" + m_fqdn + ", m_password="
                + m_password + ", m_primary=" + m_primary + ", m_registered=" + m_registered + ", m_callTraffic="
                + m_callTraffic + ", m_state=" + m_state + ", m_lastAttempt=" + m_lastAttempt
                + ", m_failedReplications=" + m_failedReplications + ", m_branch=" + m_branch
                + ", m_useStun=" + m_useStun
                + ", m_stunAddress=" + m_stunAddress + ", m_stunInterval=" + m_stunInterval + ", m_publicAddress="
                + m_publicAddress + ", m_publicPort=" + m_publicPort + ", m_publicTlsPort=" + m_publicTlsPort
                + ", m_startRtpPort=" + m_startRtpPort + ", m_stopRtpPort=" + m_stopRtpPort + ", m_hostName="
                + m_hostName + "]";
    }

    @Override
    public Location getOriginalCopy() {
        return m_originalCopy;
    }

    @Override
    public void makeBackupAsOriginalCopy() {
        try {
            m_originalCopy = (Location) this.clone();
        } catch (CloneNotSupportedException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public String getEntityIdentifier() {
        return getFqdn();
    }

    @Override
    public String getConfigChangeType() {
        return ConfigChangeType.SERVER.getName();
    }
}
