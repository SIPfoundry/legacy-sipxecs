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

    @Override
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
        m_natTraversal.getSettings().getSetting("nattraversal-info/relayaggressiveness").setValue("Aggressive");
        m_natTraversal.getSettings().getSetting("nattraversal-info/concurrentrelays").setValue("2");
        m_natTraversal.getSettings().getSetting("nattraversal-info/rediscovery-time").setValue("26");
        m_natTraversal.setProxyAddress("11.126.12.15");
        m_natTraversal.setProxyServerSipHostport("11.126.12.15:5060");
        m_natTraversal.setLogDirectory("/usr/local/sipx/var/log/sipxpbx");

        m_natTraversalManager = createNiceMock(NatTraversalManager.class);
        m_natTraversalManager.getNatTraversal();
        expectLastCall().andReturn(m_natTraversal);

    }

    public void testGenerateNoBehindNat() throws Exception {
        m_natTraversal.setBehindnat(false);
        m_natTraversal.getSettings().getSetting("nattraversal-info/publicaddress").setValue("1.2.3.4");

        m_bridgeSbc = null;
        m_sbcDeviceManager = createNiceMock(SbcDeviceManager.class);
        m_sbcDeviceManager.getBridgeSbc();
        expectLastCall().andReturn(m_bridgeSbc);

        m_natTraversal.setSbcDeviceManager(m_sbcDeviceManager);
        EasyMock.replay(m_sbcManager, m_sbcDeviceManager, m_natTraversalManager);

        NatTraversalRules rules = generate(m_sbcManager, m_natTraversalManager);

        String generatedXml = rules.getFileContent();

        InputStream referenceXmlStream = NatTraversalRulesTest.class.getResourceAsStream("nattraversalrules.test.no-behind-nat.xml");
        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
    }

    public void testGenerateNoPublicAddress() throws Exception {
        m_natTraversal.setBehindnat(true);
        m_natTraversal.getSettings().getSetting("nattraversal-info/publicaddress").setValue(null);

        m_bridgeSbc = null;
        m_sbcDeviceManager = createNiceMock(SbcDeviceManager.class);
        m_sbcDeviceManager.getBridgeSbc();
        expectLastCall().andReturn(m_bridgeSbc);

        m_natTraversal.setSbcDeviceManager(m_sbcDeviceManager);
        EasyMock.replay(m_sbcManager, m_sbcDeviceManager, m_natTraversalManager);

        NatTraversalRules rules = generate(m_sbcManager, m_natTraversalManager);

        String generatedXml = rules.getFileContent();

        InputStream referenceXmlStream = NatTraversalRulesTest.class
                .getResourceAsStream("nattraversalrules.test.no-public-address.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
    }
    /**
     * Tests default values generation when there is a SBC bridge created
     * @throws Exception
     */
    public void testGenerateDefaultsSbcBridge() throws Exception {
        NatTraversal natTraversal = new NatTraversal();
        natTraversal.setModelFilesContext(TestHelper.getModelFilesContext());
        natTraversal.setEnabled(true);
        natTraversal.getSettings().getSetting("nattraversal-info/relayaggressiveness").setValue("Aggressive");
        natTraversal.setBehindnat(true);
        natTraversal.getSettings().getSetting("nattraversal-info/publicaddress").setValue("1.2.3.4");
        natTraversal.setProxyAddress("11.126.12.15");
        natTraversal.setProxyServerSipHostport("11.126.12.15:5060");
        natTraversal.setLogDirectory("/usr/local/sipx/var/log/sipxpbx");
        m_sbcDeviceManager = createNiceMock(SbcDeviceManager.class);
        m_bridgeSbc = new BridgeSbc();
        m_bridgeSbc.setModelFilesContext(TestHelper.getModelFilesContext());
        m_bridgeSbc.getSettings().getSetting("bridge-configuration/external-address").setValue("11.12.13.14");
        m_bridgeSbc.getSettings().getSetting("bridge-configuration/local-address").setValue("21.22.23.24");
        m_sbcDeviceManager.getBridgeSbc();
        expectLastCall().andReturn(m_bridgeSbc);
        expectLastCall().times(4);

        natTraversal.setSbcDeviceManager(m_sbcDeviceManager);
        NatTraversalManager natTraversalManager = createNiceMock(NatTraversalManager.class);
        natTraversalManager.getNatTraversal();
        expectLastCall().andReturn(natTraversal);

        EasyMock.replay(m_sbcManager, m_sbcDeviceManager, natTraversalManager);

        NatTraversalRules rules = generate(m_sbcManager, natTraversalManager);

        String generatedXml = rules.getFileContent();

        InputStream referenceXmlStream = NatTraversalRulesTest.class
                .getResourceAsStream("nattraversalrules.test.sbc-bridge.xml");
        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
    }

    public void testGenerateNoStunServer() throws Exception {
        m_natTraversal.setBehindnat(true);
        m_natTraversal.getSettings().getSetting("nattraversal-info/publicaddress").setValue("1.2.3.4");
        m_sbcDeviceManager = createNiceMock(SbcDeviceManager.class);
        m_bridgeSbc = null;
        m_sbcDeviceManager.getBridgeSbc();
        expectLastCall().andReturn(m_bridgeSbc);

        m_natTraversal.setSbcDeviceManager(m_sbcDeviceManager);
        EasyMock.replay(m_sbcManager, m_sbcDeviceManager, m_natTraversalManager);

        NatTraversalRules rules = generate(m_sbcManager, m_natTraversalManager);

        String generatedXml = rules.getFileContent();

        InputStream referenceXmlStream = NatTraversalRulesTest.class
                .getResourceAsStream("nattraversalrules.test.no-stun.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
    }

    private static NatTraversalRules generate(SbcManager sbcManager, NatTraversalManager natTraversalManager) {
        NatTraversalRules rules = new NatTraversalRules();
        rules.setVelocityEngine(TestHelper.getVelocityEngine());
        rules.setTemplate("nattraversal/nattraversalrules.vm");
        rules.setSbcManager(sbcManager);
        rules.setNatTraversalManager(natTraversalManager);
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
