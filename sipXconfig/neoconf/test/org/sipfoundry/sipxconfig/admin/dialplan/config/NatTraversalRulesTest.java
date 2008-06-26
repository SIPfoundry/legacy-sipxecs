package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.Arrays;
import java.util.List;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.DefaultSbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.Sbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcRoutes;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversal;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversalManager;

import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.expectLastCall;

public class NatTraversalRulesTest extends XMLTestCase {
    private SbcManager m_sbcManager;
    private NatTraversalManager m_natTraversalManager;
    private SbcDeviceManager m_sbcDeviceManager;
    private NatTraversal m_natTraversal;
    private BridgeSbc m_bridgeSbc;

    protected void setUp() throws Exception {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);

        Sbc sbc = configureSbc(new DefaultSbc(), configureSbcDevice("10.1.2.3"), Arrays.asList("*.example.org"), Arrays
                .asList("10.0.0.0/22", "172.16.0.0/12", "192.168.0.0/16"));

        m_sbcManager = createNiceMock(SbcManager.class);
        m_sbcManager.loadDefaultSbc();
        expectLastCall().andReturn(sbc);

        m_natTraversal = new NatTraversal();
        m_natTraversal.setModelFilesContext(TestHelper.getModelFilesContext());
        m_natTraversal.setEnabled(true);
        m_natTraversal.getSettings().getSetting(NatTraversal.INFO_AGGRESSIVENESS).setTypedValue("Aggressive");
        m_natTraversal.getSettings().getSetting(NatTraversal.INFO_MAXCONCRELAYS).setTypedValue(Integer.parseInt("2"));
        m_natTraversal.getSettings().getSetting(NatTraversal.INFO_REFRESHINTERVAL).setTypedValue(Integer.parseInt("26"));

        m_natTraversalManager = createNiceMock(NatTraversalManager.class);
        m_natTraversalManager.getNatTraversal();
        expectLastCall().andReturn(m_natTraversal);

    }

    public void testGenerateNoBehindNat() throws Exception {
        m_natTraversal.setBehindnat(false);

        m_sbcDeviceManager = createNiceMock(SbcDeviceManager.class);
        m_bridgeSbc = new BridgeSbc(); 
        m_bridgeSbc.setModelFilesContext(TestHelper.getModelFilesContext());
        m_bridgeSbc.getSettings().getSetting("bridge-configuration/external-address").setTypedValue("1.2.3.4");
        m_bridgeSbc.getSettings().getSetting("bridge-configuration/local-address").setTypedValue("11.12.21.1");
        m_bridgeSbc.getSettings().getSetting("bridge-configuration/external-port").setTypedValue("4321");
        m_sbcDeviceManager.getBridgeSbc();
        expectLastCall().andReturn(m_bridgeSbc);

        EasyMock.replay(m_sbcManager, m_sbcDeviceManager, m_natTraversalManager);

        NatTraversalRules rules = generate(m_sbcManager, m_natTraversalManager, m_sbcDeviceManager);

        String generatedXml = rules.getFileContent();

        InputStream referenceXmlStream = NatTraversalRulesTest.class.getResourceAsStream("nattraversalrules.test.no-behind-nat.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));

    }

    public void testGenerateNoPublicAddress() throws Exception {
        m_natTraversal.setBehindnat(true);
        m_natTraversal.getSettings().getSetting(NatTraversal.INFO_PUBLICADDRESS).setTypedValue("");
        m_sbcDeviceManager = createNiceMock(SbcDeviceManager.class);
        m_bridgeSbc = new BridgeSbc(); 
        m_bridgeSbc.setModelFilesContext(TestHelper.getModelFilesContext());
        m_bridgeSbc.getSettings().getSetting("bridge-configuration/external-address").setTypedValue("1.2.3.4");
        m_bridgeSbc.getSettings().getSetting("bridge-configuration/local-address").setTypedValue("11.12.21.1");
        m_bridgeSbc.getSettings().getSetting("bridge-configuration/external-port").setTypedValue("4321");
        m_sbcDeviceManager.getBridgeSbc();
        expectLastCall().andReturn(m_bridgeSbc);

        EasyMock.replay(m_sbcManager, m_sbcDeviceManager, m_natTraversalManager);

        NatTraversalRules rules = generate(m_sbcManager, m_natTraversalManager, m_sbcDeviceManager);

        String generatedXml = rules.getFileContent();

        InputStream referenceXmlStream = NatTraversalRulesTest.class
                .getResourceAsStream("nattraversalrules.test.no-public-address.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
    }

    public void testGenerateNoSbcBridge() throws Exception {
        m_natTraversal.setBehindnat(true);
        m_natTraversal.getSettings().getSetting(NatTraversal.INFO_PUBLICADDRESS).setTypedValue("1.2.3.4");
        m_sbcDeviceManager = createNiceMock(SbcDeviceManager.class);
        m_bridgeSbc = null;
        m_sbcDeviceManager.getBridgeSbc();
        expectLastCall().andReturn(m_bridgeSbc);

        EasyMock.replay(m_sbcManager, m_sbcDeviceManager, m_natTraversalManager);

        NatTraversalRules rules = generate(m_sbcManager, m_natTraversalManager, m_sbcDeviceManager);

        String generatedXml = rules.getFileContent();

        InputStream referenceXmlStream = NatTraversalRulesTest.class
                .getResourceAsStream("nattraversalrules.test.no-sbc-bridge.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
    }

    public void testGenerateNoStunServer() throws Exception {
        m_natTraversal.setBehindnat(true);
        m_natTraversal.getSettings().getSetting(NatTraversal.INFO_PUBLICADDRESS).setTypedValue("1.2.3.4");

        m_sbcDeviceManager = createNiceMock(SbcDeviceManager.class);
        m_bridgeSbc = new BridgeSbc();
        m_bridgeSbc.setModelFilesContext(TestHelper.getModelFilesContext());
        m_bridgeSbc.getSettings().getSetting("bridge-configuration/external-address").setTypedValue("1.2.3.4");
        m_bridgeSbc.getSettings().getSetting("bridge-configuration/local-address").setTypedValue("11.12.21.1");
        m_bridgeSbc.getSettings().getSetting("bridge-configuration/external-port").setTypedValue("4321");
        m_sbcDeviceManager.getBridgeSbc();
        expectLastCall().andReturn(m_bridgeSbc);

        EasyMock.replay(m_sbcManager, m_sbcDeviceManager, m_natTraversalManager);

        NatTraversalRules rules = generate(m_sbcManager, m_natTraversalManager, m_sbcDeviceManager);

        String generatedXml = rules.getFileContent();

        InputStream referenceXmlStream = NatTraversalRulesTest.class
                .getResourceAsStream("nattraversalrules.test.no-stun.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
    }

    private static NatTraversalRules generate(SbcManager sbcManager, NatTraversalManager natTraversalManager,
            SbcDeviceManager sbcDeviceManager) {
        NatTraversalRules rules = new NatTraversalRules();
        rules.setVelocityEngine(TestHelper.getVelocityEngine());
        rules.setTemplate("nattraversal/nattraversalrules.vm");
        rules.setSbcManager(sbcManager);
        rules.setNatTraversalManager(natTraversalManager);
        rules.setSbcDeviceManager(sbcDeviceManager);
        return rules;
    }

    private static Sbc configureSbc(Sbc sbc, SbcDevice sbcDevice, List<String> domains, List<String> subnets) {
        SbcRoutes routes = new SbcRoutes();
        routes.setDomains(domains);
        routes.setSubnets(subnets);

        sbc.setRoutes(routes);
        sbc.setSbcDevice(sbcDevice);
        sbc.setEnabled(true);
        return sbc;
    }

    private static SbcDevice configureSbcDevice(String address) {
        SbcDevice sbcDevice = new SbcDevice();
        sbcDevice.setAddress(address);
        return sbcDevice;
    }

}
