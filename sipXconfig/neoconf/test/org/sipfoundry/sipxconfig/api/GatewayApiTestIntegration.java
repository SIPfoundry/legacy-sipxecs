package org.sipfoundry.sipxconfig.api;

import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dialplan.DialPlanSetup;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.registrar.Registrar;
import org.sipfoundry.sipxconfig.test.RestApiIntegrationTestCase;

public class GatewayApiTestIntegration extends RestApiIntegrationTestCase {
    private GatewayContext m_gatewayContext;
    private DialPlanContext m_dialPlanContext;
    private FeatureManager m_featureManager;
    private LocationsManager m_locationsManager;
    private DialPlanSetup m_dialPlanSetup;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        sql("commserver/SeedLocations.sql");
        Location primary = m_locationsManager.getPrimaryLocation();
        m_featureManager.enableLocationFeature(Registrar.FEATURE, primary, true);
        m_featureManager.enableLocationFeature(ProxyManager.FEATURE, primary, true);
        m_dialPlanSetup.setupDefaultRegion();
    }

    @Test
    public void testGatewayJsonApi() throws Exception {
        // query empty schedules
        String emptyGateways = getAsJson("gateways");
        assertEquals("{\"gateways\":[]}", emptyGateways);

        // create gateway
        String createGateway = "{\"name\":\"myGateway\",\"serialNo\":null,\"deviceVersion\":null,\"description\":\"myDesc\","
            + "\"model\":{\"modelId\":\"acmeGatewayStandard\",\"label\":\"Acme 1000\",\"vendor\":\"acme\",\"versions\":null},"
            + "\"enabled\":true,\"address\":\"1.1.1.1\",\"addressPort\":0,\"outboundAddress\":null,\"outboundPort\":5060,"
            + "\"addressTransport\":\"none\",\"prefix\":null,\"shared\":true,\"useInternalBridge\":true,\"branch\":null,"
            + "\"callerAliasInfo\":{\"defaultCallerAlias\":null,\"anonymous\":false,\"ignoreUserInfo\":false,"
            + "\"transformUserExtension\":false,\"addPrefix\":null,\"keepDigits\":0,\"displayName\":null,\"urlParameters\":null}}";
        int code = postJsonString(createGateway, "gateways");
        assertEquals(200, code);
        List<Gateway> gateways = m_gatewayContext.getGateways();
        assertEquals(1, gateways.size());

        Gateway gateway = m_gatewayContext.getGateway(gateways.get(0).getId());

        // retrieve gateways
        String strGateways = getAsJson("gateways");
        assertEquals(
                String.format(
                        "{\"gateways\":[{\"id\":%s,\"name\":\"myGateway\",\"serialNo\":null,\"deviceVersion\":null,\"description\":\"myDesc\","
                        + "\"model\":{\"modelId\":\"acmeGatewayStandard\",\"label\":\"Acme 1000\",\"vendor\":\"acme\","
                        + "\"versions\":null},\"enabled\":true,\"address\":\"1.1.1.1\",\"addressPort\":0,\"outboundAddress\":null,\"outboundPort\":5060,"
                        + "\"addressTransport\":\"none\",\"prefix\":null,\"shared\":true,\"useInternalBridge\":true,\"branch\":null,"
                        + "\"callerAliasInfo\":{\"defaultCallerAlias\":null,\"anonymous\":false,\"ignoreUserInfo\":false,\"transformUserExtension\":false,"
                        + "\"addPrefix\":null,\"keepDigits\":0,\"displayName\":null,\"urlParameters\":null}}]}", gateway.getId()), strGateways);

        // retrieve gateway
        String gatewayJson = getAsJson(String.format("gateways/%s", gateway.getId()));
        assertEquals(
                String.format(
                    "{\"id\":%s,\"name\":\"myGateway\",\"serialNo\":null,\"deviceVersion\":null,\"description\":\"myDesc\","
                        + "\"model\":{\"modelId\":\"acmeGatewayStandard\",\"label\":\"Acme 1000\",\"vendor\":\"acme\",\"versions\":null},"
                        + "\"enabled\":true,\"address\":\"1.1.1.1\",\"addressPort\":0,\"outboundAddress\":null,\"outboundPort\":5060,"
                        + "\"addressTransport\":\"none\",\"prefix\":null,\"shared\":true,\"useInternalBridge\":true,\"branch\":null,"
                        + "\"callerAliasInfo\":{\"defaultCallerAlias\":null,\"anonymous\":false,\"ignoreUserInfo\":false,"
                        + "\"transformUserExtension\":false,\"addPrefix\":null,\"keepDigits\":0,\"displayName\":null,\"urlParameters\":null}}",
                        gateway.getId()), gatewayJson);

        // modify gateway
        String modifyGateway = "{\"name\":\"myGateway\",\"serialNo\":null,\"deviceVersion\":null,\"description\":\"myDesc\","
            + "\"model\":{\"modelId\":\"acmeGatewayStandard\",\"label\":\"Acme 1000\",\"vendor\":\"acme\",\"versions\":null},"
            + "\"enabled\":true,\"address\":\"1.1.1.1\",\"addressPort\":0,\"outboundAddress\":null,\"outboundPort\":5060,"
            + "\"addressTransport\":\"none\",\"prefix\":500,\"shared\":true,\"useInternalBridge\":true,\"branch\":null,"
            + "\"callerAliasInfo\":{\"defaultCallerAlias\":null,\"anonymous\":false,\"ignoreUserInfo\":false,"
            + "\"transformUserExtension\":false,\"addPrefix\":null,\"keepDigits\":0,\"displayName\":null,\"urlParameters\":null}}";
        int putCode = putJsonString(modifyGateway, String.format("gateways/%s", gateway.getId()));
        assertEquals(200, putCode);

        //retrieve modified gateway
        gatewayJson = getAsJson(String.format("gateways/%s", gateway.getId()));
        assertEquals(
                String.format(
                    "{\"id\":%s,\"name\":\"myGateway\",\"serialNo\":null,\"deviceVersion\":null,\"description\":\"myDesc\","
                        + "\"model\":{\"modelId\":\"acmeGatewayStandard\",\"label\":\"Acme 1000\",\"vendor\":\"acme\",\"versions\":null},"
                        + "\"enabled\":true,\"address\":\"1.1.1.1\",\"addressPort\":0,\"outboundAddress\":null,\"outboundPort\":5060,"
                        + "\"addressTransport\":\"none\",\"prefix\":\"500\",\"shared\":true,\"useInternalBridge\":true,\"branch\":null,"
                        + "\"callerAliasInfo\":{\"defaultCallerAlias\":null,\"anonymous\":false,\"ignoreUserInfo\":false,"
                        + "\"transformUserExtension\":false,\"addPrefix\":null,\"keepDigits\":0,\"displayName\":null,\"urlParameters\":null}}", gateway.getId()), gatewayJson);

        //retrieve gateway models
        gatewayJson = getAsJson(String.format("gateways/models"));
        assertEquals(String.format("{\"models\":[{\"modelId\":\"acmeGatewayStandard\",\"label\":\"Acme 1000\",\"vendor\":\"acme\",\"versions\":null},"
            + "{\"modelId\":\"genericGatewayStandard\",\"label\":\"Unmanaged gateway\",\"vendor\":null,\"versions\":null},"
            + "{\"modelId\":\"sipTrunkStandard\",\"label\":\"SIP trunk\",\"vendor\":null,\"versions\":null}]}"), gatewayJson);

        //retrieve gateway setting
        gatewayJson = getAsJson(String.format("gateways/%s/settings/basic/proxyAddress", gateway.getId()));
        assertEquals("{\"path\":\"basic/proxyAddress\",\"type\":\"string\",\"options\":null,"
            + "\"value\":\"192.168.0.26\",\"defaultValue\":\"192.168.0.26\",\"label\":null,\"description\":null}", gatewayJson);

        // modify gateway setting
        int settingCode = putPlainText("2.2.2.2", String.format("gateways/%s/settings/basic/proxyAddress", gateway.getId()));
        assertEquals(200, settingCode);
        String modifiedSetting = getAsJson(String.format("gateways/%s/settings/basic/proxyAddress", gateway.getId()));
        assertEquals("{\"path\":\"basic/proxyAddress\",\"type\":\"string\",\"options\":null,"
            + "\"value\":\"2.2.2.2\",\"defaultValue\":\"192.168.0.26\",\"label\":null,\"description\":null}", modifiedSetting);

        // reset setting
        int resetCode = delete(String.format("gateways/%s/settings/basic/proxyAddress", gateway.getId()));
        assertEquals(200, resetCode);

        // get available rules
        List<DialingRule> rules = m_dialPlanContext.getAvailableRules(gateway.getId());
        String rulesJson = getAsJson(String.format("gateways/%s/availablerules", gateway.getId()));
        assertEquals(String.format("{\"rules\":[{\"id\":%s,\"name\":\"Emergency\",\"enabled\":false,\"type\":"
            + "\"Emergency\",\"description\":\"Emergency dialing plan\",\"schedule\":null,\"permissions\":{\"names\":[]},\"gatewayAware\":true,"
            + "\"authorizationChecked\":true,\"internal\":false,\"mediaServerHostname\":null,\"mediaServerType\":null,\"dialPatterns\":null,"
            + "\"callPattern\":null,\"pstnPrefix\":null,\"pstnPrefixOptional\":false,\"longDistancePrefix\":null,\"longDistancePrefixOptional\":false,"
            + "\"areaCodes\":null,\"externalLen\":0,\"optionalPrefix\":\"9\",\"emergencyNumber\":\"911\",\"afterHoursAttendant\":null,\"holidayAttendant\":null,"
            + "\"workingTimeAttendant\":null,\"holidayAttendantPeriods\":null,\"workingTimeAttendantPeriods\":null,\"extension\":null,\"attendantAliases\":null,"
            + "\"did\":null,\"enableLiveAttendant\":false},{\"id\":%s,\"name\":\"International\",\"enabled\":false,\"type\":\"Long Distance\","
            + "\"description\":\"International dialing\",\"schedule\":null,\"permissions\":{\"names\":[]},\"gatewayAware\":true,"
            + "\"authorizationChecked\":true,\"internal\":false,\"mediaServerHostname\":null,\"mediaServerType\":null,\"dialPatterns\":null,"
            + "\"callPattern\":null,\"pstnPrefix\":\"\",\"pstnPrefixOptional\":false,\"longDistancePrefix\":\"011\",\"longDistancePrefixOptional\":false,"
            + "\"areaCodes\":\"\",\"externalLen\":-1,\"optionalPrefix\":null,\"emergencyNumber\":null,\"afterHoursAttendant\":null,\"holidayAttendant\":null,"
            + "\"workingTimeAttendant\":null,\"holidayAttendantPeriods\":null,\"workingTimeAttendantPeriods\":null,\"extension\":null,\"attendantAliases\":null,"
            + "\"did\":null,\"enableLiveAttendant\":false},{\"id\":%s,\"name\":\"Local\",\"enabled\":false,\"type\":\"Long Distance\",\"description\":\"Local dialing\","
            + "\"schedule\":null,\"permissions\":{\"names\":[]},\"gatewayAware\":true,\"authorizationChecked\":true,\"internal\":false,\"mediaServerHostname\":null,"
            + "\"mediaServerType\":null,\"dialPatterns\":null,\"callPattern\":null,\"pstnPrefix\":\"9\",\"pstnPrefixOptional\":false,\"longDistancePrefix\":\"\",\"longDistancePrefixOptional\":true,"
            + "\"areaCodes\":\"\",\"externalLen\":7,\"optionalPrefix\":null,\"emergencyNumber\":null,\"afterHoursAttendant\":null,\"holidayAttendant\":null,\"workingTimeAttendant\":null,"
            + "\"holidayAttendantPeriods\":null,\"workingTimeAttendantPeriods\":null,\"extension\":null,\"attendantAliases\":null,\"did\":null,\"enableLiveAttendant\":false},"
            + "{\"id\":%s,\"name\":\"Long Distance\",\"enabled\":false,\"type\":\"Long Distance\",\"description\":\"Long distance dialing plan\",\"schedule\":null,"
            + "\"permissions\":{\"names\":[]},\"gatewayAware\":true,\"authorizationChecked\":true,\"internal\":false,\"mediaServerHostname\":null,\"mediaServerType\":null,"
            + "\"dialPatterns\":null,\"callPattern\":null,\"pstnPrefix\":\"9\",\"pstnPrefixOptional\":true,\"longDistancePrefix\":\"1\",\"longDistancePrefixOptional\":false,"
            + "\"areaCodes\":\"\",\"externalLen\":10,\"optionalPrefix\":null,\"emergencyNumber\":null,\"afterHoursAttendant\":null,\"holidayAttendant\":null,\"workingTimeAttendant\":null,"
            + "\"holidayAttendantPeriods\":null,\"workingTimeAttendantPeriods\":null,\"extension\":null,\"attendantAliases\":null,\"did\":null,\"enableLiveAttendant\":false},{\"id\":%s,"
            + "\"name\":\"Restricted\",\"enabled\":false,\"type\":\"Long Distance\",\"description\":\"Restricted dialing\",\"schedule\":null,\"permissions\":{\"names\":[]},"
            + "\"gatewayAware\":true,\"authorizationChecked\":true,\"internal\":false,\"mediaServerHostname\":null,\"mediaServerType\":null,\"dialPatterns\":null,"
            + "\"callPattern\":null,\"pstnPrefix\":\"\",\"pstnPrefixOptional\":false,\"longDistancePrefix\":\"\",\"longDistancePrefixOptional\":false,\"areaCodes\":\"900\","
            + "\"externalLen\":0,\"optionalPrefix\":null,\"emergencyNumber\":null,\"afterHoursAttendant\":null,\"holidayAttendant\":null,\"workingTimeAttendant\":null,\"holidayAttendantPeriods\":null,"
            + "\"workingTimeAttendantPeriods\":null,\"extension\":null,\"attendantAliases\":null,\"did\":null,\"enableLiveAttendant\":false},{\"id\":%s,\"name\":\"Toll free\",\"enabled\":false,"
            + "\"type\":\"Long Distance\",\"description\":\"Toll free dialing\",\"schedule\":null,\"permissions\":{\"names\":[]},\"gatewayAware\":true,\"authorizationChecked\":true,\"internal\":false,"
            + "\"mediaServerHostname\":null,\"mediaServerType\":null,\"dialPatterns\":null,\"callPattern\":null,\"pstnPrefix\":\"\",\"pstnPrefixOptional\":false,\"longDistancePrefix\":\"\","
            + "\"longDistancePrefixOptional\":false,\"areaCodes\":\"800, 866, 877, 888\",\"externalLen\":0,\"optionalPrefix\":null,\"emergencyNumber\":null,"
            + "\"afterHoursAttendant\":null,\"holidayAttendant\":null,\"workingTimeAttendant\":null,\"holidayAttendantPeriods\":null,\"workingTimeAttendantPeriods\":null,\"extension\":null,"
            + "\"attendantAliases\":null,\"did\":null,\"enableLiveAttendant\":false}]}",
            rules.get(0).getId(), rules.get(1).getId(), rules.get(2).getId(), rules.get(3).getId(), rules.get(4).getId(), rules.get(5).getId()),
            rulesJson);

        // add gateway to long distance rule
        putCode = putJsonString(StringUtils.EMPTY, String.format("gateways/%s/%s", gateway.getId(), rules.get(3).getId()));
        assertEquals(200, putCode);

        //remove gateway from rule
        int deleteCode = delete(String.format("gateways/%s/%s", gateway.getId(), rules.get(3).getId()));
        assertEquals(200, deleteCode);

        // delete gateway
        int deleteGateway = delete(String.format("gateways/%s", gateway.getId()));
        assertEquals(200, deleteGateway);
        assertEquals(0, m_gatewayContext.getGateways().size());
    }

    @Test
    public void testGatewayXmlApi() throws Exception {
        // query empty schedules
        String emptyGateways = getAsXml("gateways");
        assertEquals("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Gateways/>", emptyGateways);

        // create gateway
        String createGateway =
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Gateway><name>myGateway</name><description>myDesc</description>"
            + "<Model><modelId>acmeGatewayStandard</modelId><label>Acme 1000</label><vendor>acme</vendor></Model>"
            + "<enabled>true</enabled><address>1.1.1.1</address><addressPort>0</addressPort><outboundPort>5060</outboundPort><addressTransport>none</addressTransport>"
            + "<shared>true</shared><useInternalBridge>true</useInternalBridge><CallerAliasInfo><anonymous>false</anonymous><ignoreUserInfo>false</ignoreUserInfo>"
            + "<transformUserExtension>false</transformUserExtension><keepDigits>0</keepDigits></CallerAliasInfo></Gateway>";
        int code = postXmlString(createGateway, "gateways");
        assertEquals(200, code);
        List<Gateway> gateways = m_gatewayContext.getGateways();
        assertEquals(1, gateways.size());

        Gateway gateway = m_gatewayContext.getGateway(gateways.get(0).getId());

        // retrieve gateways
        String strGateways = getAsXml("gateways");
        assertEquals(
                String.format(
                    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Gateways><Gateway><id>%s</id><name>myGateway</name>"
                    + "<description>myDesc</description><Model><modelId>acmeGatewayStandard</modelId>"
                    + "<label>Acme 1000</label><vendor>acme</vendor></Model><enabled>true</enabled><address>1.1.1.1</address><addressPort>0</addressPort>"
                    + "<outboundPort>5060</outboundPort><addressTransport>none</addressTransport>"
                    + "<shared>true</shared><useInternalBridge>true</useInternalBridge>"
                    + "<CallerAliasInfo><anonymous>false</anonymous><ignoreUserInfo>false</ignoreUserInfo><transformUserExtension>false</transformUserExtension><keepDigits>0</keepDigits></CallerAliasInfo>"
                    + "</Gateway></Gateways>", gateway.getId()), strGateways);

        // retrieve gateway
        String gatewayXml = getAsXml(String.format("gateways/%s", gateway.getId()));
        assertEquals(
                String.format(
                    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Gateway><id>%s</id><name>myGateway</name><description>myDesc</description>"
                    + "<Model><modelId>acmeGatewayStandard</modelId><label>Acme 1000</label><vendor>acme</vendor></Model>"
                    + "<enabled>true</enabled><address>1.1.1.1</address><addressPort>0</addressPort><outboundPort>5060</outboundPort>"
                    + "<addressTransport>none</addressTransport><shared>true</shared><useInternalBridge>true</useInternalBridge>"
                    + "<CallerAliasInfo><anonymous>false</anonymous><ignoreUserInfo>false</ignoreUserInfo>"
                    + "<transformUserExtension>false</transformUserExtension><keepDigits>0</keepDigits></CallerAliasInfo></Gateway>",
                        gateway.getId()), gatewayXml);

        // modify gateway
        String modifyGateway = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            + "<Gateway><id>%s</id><name>myGateway</name><description>myDesc</description><Model><modelId>acmeGatewayStandard</modelId><label>Acme 1000</label><vendor>acme</vendor>"
            + "</Model><enabled>true</enabled><address>1.1.1.1</address><addressPort>0</addressPort><outboundPort>5060</outboundPort>"
            + "<addressTransport>none</addressTransport><prefix>500</prefix>"
            + "<shared>true</shared><useInternalBridge>true</useInternalBridge>"
            + "<CallerAliasInfo><anonymous>false</anonymous><ignoreUserInfo>false</ignoreUserInfo><transformUserExtension>false</transformUserExtension>"
            + "<keepDigits>0</keepDigits></CallerAliasInfo></Gateway>";

        int putCode = putXmlString(modifyGateway, String.format("gateways/%s", gateway.getId()));
        assertEquals(200, putCode);

        //retrieve modified gateway
        gatewayXml = getAsXml(String.format("gateways/%s", gateway.getId()));
        assertEquals(String.format(
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                + "<Gateway><id>%s</id><name>myGateway</name><description>myDesc</description><Model><modelId>acmeGatewayStandard</modelId><label>Acme 1000</label><vendor>acme</vendor>"
                + "</Model><enabled>true</enabled><address>1.1.1.1</address><addressPort>0</addressPort><outboundPort>5060</outboundPort>"
                + "<addressTransport>none</addressTransport><prefix>500</prefix>"
                + "<shared>true</shared><useInternalBridge>true</useInternalBridge>"
                + "<CallerAliasInfo><anonymous>false</anonymous><ignoreUserInfo>false</ignoreUserInfo><transformUserExtension>false</transformUserExtension>"
                + "<keepDigits>0</keepDigits></CallerAliasInfo></Gateway>", gateway.getId()), gatewayXml);

        //retrieve gateway models
        gatewayXml = getAsXml(String.format("gateways/models"));
        assertEquals(String.format("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            + "<Models><Model><modelId>acmeGatewayStandard</modelId><label>Acme 1000</label><vendor>acme</vendor></Model>"
            + "<Model><modelId>genericGatewayStandard</modelId><label>Unmanaged gateway</label></Model>"
            + "<Model><modelId>sipTrunkStandard</modelId><label>SIP trunk</label></Model></Models>"), gatewayXml);

        //retrieve gateway setting
        gatewayXml = getAsXml(String.format("gateways/%s/settings/basic/proxyAddress", gateway.getId()));
        assertEquals("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            + "<Setting><path>basic/proxyAddress</path><type>string</type>"
            + "<value>192.168.0.26</value><defaultValue>192.168.0.26</defaultValue></Setting>", gatewayXml);

        // modify gateway setting
        int settingCode = putPlainText("2.2.2.2", String.format("gateways/%s/settings/basic/proxyAddress", gateway.getId()));
        assertEquals(200, settingCode);
        String modifiedSetting = getAsXml(String.format("gateways/%s/settings/basic/proxyAddress", gateway.getId()));
        assertEquals("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            + "<Setting><path>basic/proxyAddress</path><type>string</type>"
            + "<value>2.2.2.2</value><defaultValue>192.168.0.26</defaultValue></Setting>", modifiedSetting);

        // reset setting
        int resetCode = delete(String.format("gateways/%s/settings/basic/proxyAddress", gateway.getId()));
        assertEquals(200, resetCode);

        // get available rules
        List<DialingRule> rules = m_dialPlanContext.getAvailableRules(gateway.getId());
        String rulesXml = getAsXml(String.format("gateways/%s/availablerules", gateway.getId()));
        assertEquals(String.format("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            + "<Rules><Rule><id>%s</id><name>Emergency</name><enabled>false</enabled><type>Emergency</type><description>Emergency dialing plan</description><permissions/>"
            + "<gatewayAware>true</gatewayAware><authorizationChecked>true</authorizationChecked><internal>false</internal><pstnPrefixOptional>false</pstnPrefixOptional>"
            + "<longDistancePrefixOptional>false</longDistancePrefixOptional><externalLen>0</externalLen><optionalPrefix>9</optionalPrefix><emergencyNumber>911</emergencyNumber>"
            + "<enableLiveAttendant>false</enableLiveAttendant></Rule><Rule><id>%s</id><name>International</name><enabled>false</enabled><type>Long Distance</type>"
            + "<description>International dialing</description><permissions/><gatewayAware>true</gatewayAware>"
            + "<authorizationChecked>true</authorizationChecked><internal>false</internal><pstnPrefix></pstnPrefix><pstnPrefixOptional>false</pstnPrefixOptional>"
            + "<longDistancePrefix>011</longDistancePrefix><longDistancePrefixOptional>false</longDistancePrefixOptional><areaCodes></areaCodes>"
            + "<externalLen>-1</externalLen><enableLiveAttendant>false</enableLiveAttendant></Rule><Rule><id>%s</id><name>Local</name><enabled>false</enabled>"
            + "<type>Long Distance</type><description>Local dialing</description><permissions/><gatewayAware>true</gatewayAware><authorizationChecked>true</authorizationChecked>"
            + "<internal>false</internal><pstnPrefix>9</pstnPrefix><pstnPrefixOptional>false</pstnPrefixOptional><longDistancePrefix></longDistancePrefix>"
            + "<longDistancePrefixOptional>true</longDistancePrefixOptional><areaCodes></areaCodes><externalLen>7</externalLen><enableLiveAttendant>false</enableLiveAttendant></Rule>"
            + "<Rule><id>%s</id><name>Long Distance</name><enabled>false</enabled><type>Long Distance</type><description>Long distance dialing plan</description><permissions/>"
            + "<gatewayAware>true</gatewayAware><authorizationChecked>true</authorizationChecked><internal>false</internal><pstnPrefix>9</pstnPrefix>"
            + "<pstnPrefixOptional>true</pstnPrefixOptional><longDistancePrefix>1</longDistancePrefix><longDistancePrefixOptional>false</longDistancePrefixOptional><areaCodes></areaCodes>"
            + "<externalLen>10</externalLen><enableLiveAttendant>false</enableLiveAttendant></Rule>"
            + "<Rule><id>%s</id><name>Restricted</name><enabled>false</enabled><type>Long Distance</type><description>Restricted dialing</description><permissions/>"
            + "<gatewayAware>true</gatewayAware><authorizationChecked>true</authorizationChecked><internal>false</internal><pstnPrefix></pstnPrefix>"
            + "<pstnPrefixOptional>false</pstnPrefixOptional><longDistancePrefix></longDistancePrefix><longDistancePrefixOptional>false</longDistancePrefixOptional>"
            + "<areaCodes>900</areaCodes><externalLen>0</externalLen><enableLiveAttendant>false</enableLiveAttendant></Rule>"
            + "<Rule><id>%s</id><name>Toll free</name><enabled>false</enabled><type>Long Distance</type><description>Toll free dialing</description><permissions/>"
            + "<gatewayAware>true</gatewayAware><authorizationChecked>true</authorizationChecked><internal>false</internal><pstnPrefix></pstnPrefix>"
            + "<pstnPrefixOptional>false</pstnPrefixOptional><longDistancePrefix></longDistancePrefix><longDistancePrefixOptional>false</longDistancePrefixOptional>"
            + "<areaCodes>800, 866, 877, 888</areaCodes><externalLen>0</externalLen><enableLiveAttendant>false</enableLiveAttendant></Rule></Rules>",
            rules.get(0).getId(), rules.get(1).getId(), rules.get(2).getId(), rules.get(3).getId(), rules.get(4).getId(), rules.get(5).getId()),
            rulesXml);

        // add gateway to long distance rule
        putCode = putXmlString(StringUtils.EMPTY, String.format("gateways/%s/%s", gateway.getId(), rules.get(3).getId()));
        assertEquals(200, putCode);

        //remove gateway from rule
        int deleteCode = delete(String.format("gateways/%s/%s", gateway.getId(), rules.get(3).getId()));
        assertEquals(200, deleteCode);

        // delete gateway
        int deleteGateway = delete(String.format("gateways/%s", gateway.getId()));
        assertEquals(200, deleteGateway);
        assertEquals(0, m_gatewayContext.getGateways().size());

    }

    public void setGatewayContext(GatewayContext gatewayContext) {
        m_gatewayContext = gatewayContext;
    }

    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setDialPlanSetup(DialPlanSetup dialPlanSetup) {
        m_dialPlanSetup = dialPlanSetup;
    }
}
