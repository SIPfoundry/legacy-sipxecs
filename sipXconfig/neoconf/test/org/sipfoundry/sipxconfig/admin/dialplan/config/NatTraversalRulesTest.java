package org.sipfoundry.sipxconfig.admin.dialplan.config;

import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.expectLastCall;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.DefaultSbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.Sbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcRoutes;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversal;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversalManager;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class NatTraversalRulesTest extends XMLTestCase {
    private SbcManager m_sbcManager;
    private NatTraversalManager m_natTraversalManager;
    private SbcDeviceManager m_sbcDeviceManager;
    private NatTraversal m_natTraversal;
    private BridgeSbc m_bridgeSbc;
    private LocationsManager m_locationsManager;
    private SipxServiceManager m_sipxServiceManager;

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
        
        m_locationsManager = EasyMock.createMock(LocationsManager.class);
        m_locationsManager.getPrimaryLocation();
        EasyMock.expectLastCall().andReturn(TestUtil.createDefaultLocation()).anyTimes();
        EasyMock.replay(m_locationsManager);
        m_natTraversal.setLocationsManager(m_locationsManager);
        
        m_natTraversal.setModelFilesContext(TestHelper.getModelFilesContext());
        m_natTraversal.setEnabled(true);
        m_natTraversal.getSettings().getSetting("nattraversal-info/relayaggressiveness").setValue("Aggressive");
        m_natTraversal.getSettings().getSetting("nattraversal-info/concurrentrelays").setValue("2");
        m_natTraversal.getSettings().getSetting("nattraversal-info/rediscovery-time").setValue("26");
        m_natTraversal.setLogDirectory("/usr/local/sipx/var/log/sipxpbx");
        m_natTraversal.getSettings().getSetting("nattraversal-info/publicport").setValue("1234");

        m_natTraversalManager = createNiceMock(NatTraversalManager.class);
        m_natTraversalManager.getNatTraversal();
        expectLastCall().andReturn(m_natTraversal);
        
        SipxProxyService sipxProxyService = new SipxProxyService();
        sipxProxyService.setBeanId(SipxProxyService.BEAN_ID);
        sipxProxyService.setSipPort("5060");
        m_sipxServiceManager = TestUtil.getMockSipxServiceManager(true, sipxProxyService);
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

        NatTraversalRules rules = generate(m_natTraversalManager);
        assertCorrectFileGeneration(rules, "nattraversalrules.test.no-behind-nat.xml");
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

        NatTraversalRules rules = generate(m_natTraversalManager);
        assertCorrectFileGeneration(rules, "nattraversalrules.test.no-public-address.xml");
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
        natTraversal.setLogDirectory("/usr/local/sipx/var/log/sipxpbx");
        natTraversal.getSettings().getSetting("nattraversal-info/publicport").setValue("1234");
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

        NatTraversalRules rules = generate(natTraversalManager);
        assertCorrectFileGeneration(rules, "nattraversalrules.test.sbc-bridge.xml");
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

        NatTraversalRules rules = generate(m_natTraversalManager);
        assertCorrectFileGeneration(rules, "nattraversalrules.test.no-stun.xml");
    }

    private NatTraversalRules generate(NatTraversalManager natTraversalManager) {
        NatTraversalRules rules = new NatTraversalRules();
        rules.setVelocityEngine(TestHelper.getVelocityEngine());
        rules.setTemplate("nattraversal/nattraversalrules.vm");
        rules.setSbcManager(m_sbcManager);
        rules.setSipxServiceManager(m_sipxServiceManager);
        rules.setNatTraversalManager(natTraversalManager);
        return rules;
    }
    
    private void assertCorrectFileGeneration(NatTraversalRules natTraveralRules,
            String expectedFileName) throws Exception {
        natTraveralRules.setVelocityEngine(TestHelper.getVelocityEngine());

        StringWriter actualConfigWriter = new StringWriter();
        natTraveralRules.write(actualConfigWriter, m_locationsManager.getPrimaryLocation());

        InputStream resourceAsStream = natTraveralRules.getClass().getResourceAsStream(expectedFileName);
        assertNotNull(resourceAsStream);

        Reader referenceConfigReader = new InputStreamReader(resourceAsStream);
        String referenceConfig = IOUtils.toString(referenceConfigReader);

        Reader actualConfigReader = new StringReader(actualConfigWriter.toString());
        String actualConfig = IOUtils.toString(actualConfigReader);

        assertEquals(referenceConfig, actualConfig);
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
