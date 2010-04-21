/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.FileSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;

public abstract class AudioCodesGateway extends Gateway {
    public static final String BEAN_ID = "audiocodes";
    protected static final List PARAMETER_TABLE_SETTINGS = Arrays.asList("CoderName");
    private static final String CALL_PROGRESS_TONES_FILE = "Media_RTP_RTPC/Telephony/CallProgressTonesFilename";
    private static final String FXS_LOOP_CHARACTERISTICS_FILE =
        "Media_RTP_RTPC/Telephony/FXSLoopCharacteristicsFilename";
    private static final String REL_5_4_OR_LATER = "5.4orLater";
    private static final String REL_5_6_OR_LATER = "5.6orLater";
    private static final String REL_6_0_OR_LATER = "6.0orLater";
    private static final String REL_USE_PROXYSET0 = "useProxySet0";
    private static final String[] COPY_FILES = {CALL_PROGRESS_TONES_FILE, FXS_LOOP_CHARACTERISTICS_FILE};

    private List<Ip2TelRoute> m_ip2TelRoutes = new ArrayList<Ip2TelRoute>();
    private List<Tel2IpRoute> m_tel2IpRoutes = new ArrayList<Tel2IpRoute>();

    public AudioCodesGateway() {
        setDeviceVersion(AudioCodesModel.REL_6_0);
    }

    public void setDefaultVersionId(String defaultVersionId) {
        setDeviceVersion(DeviceVersion.getDeviceVersion(BEAN_ID + defaultVersionId));
    }

    public List<Ip2TelRoute> getIp2telroutes() {
        return m_ip2TelRoutes;
    }

    public List<Tel2IpRoute> getTel2iproutes() {
        return m_tel2IpRoutes;
    }

    @Override
    public void setDeviceVersion(DeviceVersion version) {
        super.setDeviceVersion(version);
        DeviceVersion myVersion = getDeviceVersion();

        if (myVersion == AudioCodesModel.REL_5_0) {
            myVersion.addSupportedFeature(REL_USE_PROXYSET0);
        } else if (myVersion == AudioCodesModel.REL_5_2) {
            myVersion.addSupportedFeature(REL_USE_PROXYSET0);
        } else if (myVersion == AudioCodesModel.REL_5_4) {
            myVersion.addSupportedFeature(REL_USE_PROXYSET0);
            myVersion.addSupportedFeature(REL_5_4_OR_LATER);
        } else if (myVersion == AudioCodesModel.REL_5_6) {
            myVersion.addSupportedFeature(REL_USE_PROXYSET0);
            myVersion.addSupportedFeature(REL_5_4_OR_LATER);
            myVersion.addSupportedFeature(REL_5_6_OR_LATER);
        } else if (myVersion == AudioCodesModel.REL_6_0) {
            // Trunk gateways version 6.0 and later use Proxy Set 1 (Normal mode) and
            // Proxy Set 2 (Failover mode) rather than the default Proxy Set 0.
            myVersion.addSupportedFeature(REL_5_4_OR_LATER);
            myVersion.addSupportedFeature(REL_5_6_OR_LATER);
            myVersion.addSupportedFeature(REL_6_0_OR_LATER);
        }
    }

    @Override
    public void initialize() {
        AudioCodesGatewayDefaults defaults = new AudioCodesGatewayDefaults(this, getDefaults());
        addDefaultBeanSettingHandler(defaults);
        // Add the default Ip2Tel route used in both Normal and Failover mode
        for (int ipRouteId = 1; ipRouteId <= 4; ipRouteId++) {
            String ipRoute = Integer.toString(ipRouteId);
            Ip2TelRoute outgoingRoute = new Ip2TelRoute();
            outgoingRoute.setGateway(this);
            outgoingRoute.setRouteId(ipRoute);
            outgoingRoute.setSettingDestManipulation(
                "ip2tel-call-routing/ip-to-tel-manipulation/DestManipulation" + ipRoute);
            m_ip2TelRoutes.add(outgoingRoute);
        }
        // Add the default Tel2Ip Nonmal routes
        Tel2IpRoute normalRoute = new Tel2IpRoute();
        normalRoute.setGateway(this);
        normalRoute.setRouteId(Integer.toString(1));
        normalRoute.setDescription("Normal");
        normalRoute.setSettingProxyAddress("tel2ip-call-routing/tel-to-ip-normal/ProxyAddress");
        normalRoute.setSettingProxyKeepalive("tel2ip-call-routing/tel-to-ip-normal/ProxyKeepalive");
        normalRoute.setSettingProxyKeeptime("tel2ip-call-routing/tel-to-ip-normal/ProxyKeeptime");
        normalRoute.setSettingProxyHotSwap("tel2ip-call-routing/tel-to-ip-normal/ProxyHotSwap");
        normalRoute.setSettingDestManipulation("tel2ip-call-routing/tel-to-ip-normal/DestManipulation");
        m_tel2IpRoutes.add(normalRoute);
        // Add the default Tel2Ip Failover routes
        Tel2IpRoute failoverRoute = new Tel2IpRoute();
        failoverRoute.setGateway(this);
        failoverRoute.setRouteId(Integer.toString(2));
        failoverRoute.setDescription("Failover");
        failoverRoute.setSettingProxyAddress("tel2ip-call-routing/tel-to-ip-failover/ProxyAddress");
        failoverRoute.setSettingProxyKeepalive("tel2ip-call-routing/tel-to-ip-failover/ProxyKeepalive");
        failoverRoute.setSettingProxyKeeptime("tel2ip-call-routing/tel-to-ip-failover/ProxyKeeptime");
        failoverRoute.setSettingProxyHotSwap("tel2ip-call-routing/tel-to-ip-failover/ProxyHotSwap");
        failoverRoute.setSettingDestManipulation("tel2ip-call-routing/tel-to-ip-failover/DestManipulation");
        m_tel2IpRoutes.add(failoverRoute);
    }

    public void initializeIp2TelRoute(@SuppressWarnings("unused") Ip2TelRoute route) {
    }

    public void initializeTel2IpRoute(@SuppressWarnings("unused") Tel2IpRoute route) {
    }

    @Override
    public String getProfileFilename() {
        String serialNumber = getSerialNumber();
        if (serialNumber == null) {
            return null;
        }
        return serialNumber.toUpperCase() + ".ini";
    }

    @Override
    protected ProfileContext createContext() {
        return new AudioCodesContext(this, getModel().getProfileTemplate());
    }

    @Override
    protected Setting loadSettings() {
        Setting setting = getModelFilesContext().loadDynamicModelFile("gateway.xml",
                getModel().getModelDir(), getSettingsEvaluator());
        String configDir = new File(((AudioCodesModel) getModel()).getConfigDirectory(),
                getModel().getModelDir()).getAbsolutePath();
        GatewayDirectorySetter gatewayDirectorySetter = new GatewayDirectorySetter(configDir);
        setting.acceptVisitor(gatewayDirectorySetter);
        return setting;
    }

    private void copyGatewayFiles(ProfileLocation location) {
        String name;
        Setting settings = getSettings();
        String sourceDir = new File(((AudioCodesModel) getModel()).getConfigDirectory(),
                getModel().getModelDir()).getAbsolutePath();
        for (String file : COPY_FILES) {
            name = settings.getSetting(file).getValue();
            getProfileGenerator().copy(location, sourceDir, name, name);
        }
    }

    @Override
    protected void copyFiles(ProfileLocation location) {
        super.copyFiles(location);
        copyGatewayFiles(location);
    }

    private static class GatewayDirectorySetter extends AbstractSettingVisitor {
        private final String m_gatewayDirectory;

        public GatewayDirectorySetter(String directory) {
            m_gatewayDirectory = directory;
        }

        @Override
        public void visitSetting(Setting setting) {
            SettingType type = setting.getType();
            if (type instanceof FileSetting) {
                FileSetting fileType = (FileSetting) type;
                fileType.setDirectory(m_gatewayDirectory);
            }
        }
    }

    abstract int getMaxCalls();
}
