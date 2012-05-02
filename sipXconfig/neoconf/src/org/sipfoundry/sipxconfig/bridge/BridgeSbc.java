/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.bridge;


import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.FileSystemProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.SipTrunk;
import org.sipfoundry.sipxconfig.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.springframework.beans.factory.annotation.Required;

public class BridgeSbc extends SbcDevice implements DeployConfigOnEdit {
    public static final String LOG_SETTING = "bridge-configuration/log-level";
    public static final String LOCATION_ID_SETTING = "bridge-configuration/location-id";
    public static final String ITSP_PROXY_DOMAIN_SETTING = "itsp-account/itsp-proxy-domain";
    public static final String USER_NAME_SETTING = "itsp-account/user-name";
    public static final String SIXECS_LINEIDS_SETTING = "itsp-account/sipxecs-lineids";
    public static final String SIXECS_LINEID_START = "<sipxecs-lineid>";
    public static final String SIXECS_LINEID_END = "</sipxecs-lineid>";
    public static final String CONFIG_FORMAT_PREFIX = "    ";
    public static final String NEW_LINE_FEED = "\n";

    private GatewayContext m_gatewayContext;
    private LocationsManager m_locationsManager;
    private Location m_location;
    private ConfigManager m_configManager;

    @Required
    public void setGatewayContext(GatewayContext gatewayContext) {
        m_gatewayContext = gatewayContext;
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setLocation(Location location) {
        m_location = location;
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("bridge-sbc.xml", "sipxbridge");
    }

    @Override
    protected ProfileContext createContext() {
        return new Context(this, "sipxbridge/bridge.xml.vm");
    }

    @Override
    public String getProfileFilename() {
        return "sipxbridge.xml";
    }

    @Override
    public void initialize() {
        addDefaultBeanSettingHandler(new Defaults(getDefaults(), this, getLocation()));
    }

    @Override
    public ProfileLocation getProfileLocation() {
        String path = m_configManager.getLocationDataDirectory(getLocation()).getAbsolutePath();
        FileSystemProfileLocation profileLocation = new FileSystemProfileLocation();
        profileLocation.setParentDir(path);
        return profileLocation;
    }

    boolean isRedundant(SipTrunk sipTrunk, List<SipTrunk> list) {
        for (SipTrunk t : list) {
            String domain = t.getAddress();
            String username = t.getSettingValue(USER_NAME_SETTING);
            if (domain == null) {
                continue;
            }

            if (sipTrunk.getAddress() != null && domain.compareToIgnoreCase(sipTrunk.getAddress()) != 0) {
                continue;
            }

            if (username == null && sipTrunk.getSettingValue(USER_NAME_SETTING) == null) {
                return true;
            }

            if (username == null && sipTrunk.getSettingValue(USER_NAME_SETTING) != null) {
                continue;
            }

            if (username.equals(sipTrunk.getSettingValue(USER_NAME_SETTING))) {
                return true;
            }
        }
        return false;
    }

    void addGWReference(SipTrunk sipTrunk, List< ? extends SipTrunk> list) {
        String lineID;
        String lineIDs = "";
        for (Object o : list) {
            if (o instanceof SipTrunk) {
                SipTrunk t = (SipTrunk) o;

                String domain = t.getSettingValue(ITSP_PROXY_DOMAIN_SETTING);
                String username = t.getSettingValue(USER_NAME_SETTING);
                if ((domain != null && domain.equals(sipTrunk.getSettingValue(ITSP_PROXY_DOMAIN_SETTING)))) {
                    if ((username != null && username.equals(sipTrunk.getSettingValue(USER_NAME_SETTING)))
                            || (username == null && sipTrunk.getSettingValue(USER_NAME_SETTING) == null)) {

                        lineID = Integer.toString(t.getId());
                        lineIDs = lineIDs.concat(NEW_LINE_FEED + CONFIG_FORMAT_PREFIX + SIXECS_LINEID_START + lineID
                                + SIXECS_LINEID_END);
                    }
                }
            }
        }
        lineIDs = lineIDs.concat(NEW_LINE_FEED + CONFIG_FORMAT_PREFIX);
        sipTrunk.setSettingValue(SIXECS_LINEIDS_SETTING, lineIDs);
    }

    public List<SipTrunk> getMySipItsps() {
        List<SipTrunk> itsps = new ArrayList<SipTrunk>();
        List< ? extends SipTrunk> list = m_gatewayContext.getGatewayByType(SipTrunk.class);
        for (SipTrunk t : list) {
            if (equals(t.getSbcDevice()) && t.isEnabled() && !isRedundant(t, itsps)) {
                addGWReference(t, list);
                itsps.add(t);
            }
        }
        return itsps;
    }

    public List<SipTrunk> getMySipTrunks() {
        List<SipTrunk> trunks = new ArrayList<SipTrunk>();
        for (SipTrunk t : m_gatewayContext.getGatewayByType(SipTrunk.class)) {
            if (equals(t.getSbcDevice()) && t.isEnabled()) {
                trunks.add(t);
            }
        }
        return trunks;
    }

    public Location getLocation() {
        if (m_location != null) {
            return m_location;
        }
        Integer id = (Integer) getSettings().getSetting(LOCATION_ID_SETTING).getTypedValue();
        if (id == null) {
            return m_locationsManager.getLocationByAddress(getAddress());
        }
        return m_locationsManager.getLocation(id);
    }

    public void updateBridgeLocationId() {
        Location location = m_locationsManager.getLocationByAddress(getAddress());
        setSettingTypedValue(LOCATION_ID_SETTING, location.getId());
    }

    public static class Context extends ProfileContext<BridgeSbc> {
        public Context(BridgeSbc device, String profileTemplate) {
            super(device, profileTemplate);
        }

        @Override
        public Map<String, Object> getContext() {
            Map<String, Object> context = super.getContext();
            BridgeSbc device = getDevice();
            // context.put("trunks", device.getMySipTrunks());
            context.put("itsps", device.getMySipItsps());
            return context;
        }
    }

    public class Defaults {
        private final DeviceDefaults m_defaults;
        private final SbcDevice m_device;
        private final Location m_location;

        Defaults(DeviceDefaults defaults, SbcDevice device, Location location) {
            m_defaults = defaults;
            m_device = device;
            m_location = location;
        }

        @SettingEntry(paths = {"bridge-configuration/local-address", "bridge-configuration/external-address"
                })
        public String getExternalAddress() {
            return m_location.getAddress();
        }

        @SettingEntry(path = "bridge-configuration/global-address")
        public String getGlobalAddress() {
            return m_location.getPublicAddress();
        }

        @SettingEntry(path = "bridge-configuration/local-port")
        public int getLocalPort() {
            return m_device.getPort();
        }

        @SettingEntry(path = "bridge-configuration/sipx-proxy-domain")
        public String getDomainName() {
            return m_defaults.getDomainName();
        }

        @SettingEntry(path = "bridge-configuration/log-directory")
        public String getLogDirectory() {
            return m_defaults.getLogDirectory() + "/";
        }

        @SettingEntry(path = "bridge-configuration/stun-server-address")
        public String getStunServerAddress() {
            return m_location.getStunAddress();
        }

        @SettingEntry(path = "bridge-configuration/sipx-supervisor-host")
        public String getSipxSupervisorHost() {
            return m_location.getFqdn();
        }

        @SettingEntry(path = "bridge-configuration/sipx-supervisor-xml-rpc-port")
        public int getSipxSupervisorXmlRpcPort() {
            return Location.PROCESS_MONITOR_PORT;
        }
    }

    public int getXmlRpcPort() {
        return (Integer) getSettingTypedValue("bridge-configuration/xml-rpc-port");
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Arrays.asList((Feature) DialPlanContext.FEATURE, (Feature) BridgeSbcContext.FEATURE);
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }
}
