package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.InputStream;
import java.io.Reader;
import java.util.Set;
import java.util.TreeSet;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class InitialConfigTestIntegration extends IntegrationTestCase {
    private InitialConfig m_initialConfig;
    private DomainManager m_domainManager;

    public void testGetArchiveStream() throws Exception {
        if (TestUtil.isWindows()) {
            // tries to run a shell script. ignore test
            return;
        }
        Domain d = new Domain();
        Set<String> aliasesSet = new TreeSet<String>();
        aliasesSet.add("alias1");
        d.setName("testDomain");
        d.setSharedSecret("secret");

        d.setAliases(aliasesSet);
        m_domainManager.saveDomain(d);
        m_initialConfig.setBinDirectory(TestUtil.getTestSourceDirectory(this.getClass()));
        // create initial-config directory
        File file = new File(m_initialConfig.getTmpDirectory() + "/initial-config");
        file.mkdirs();
        m_initialConfig.getArchiveStream("test_location");

        InputStream referenceStream = InitialConfigTestIntegration.class
                .getResourceAsStream("expected-domain-config");
        Reader reader = new BufferedReader(new FileReader(m_initialConfig.getTmpDirectory() + "/domain-config"));

        String referenceText = IOUtils.toString(referenceStream);
        String readerText = IOUtils.toString(reader);

        // domain-config is generated
        System.out.println(referenceText);
        System.out.println(readerText);
        assertTrue(referenceText.equals(readerText));

        // archive is created
        assertTrue(new File(m_initialConfig.getTmpDirectory() + "/initial-config/test_location.tar.gz").exists());

    }

    public void setInitialConfig(InitialConfig initialConfig) {
        m_initialConfig = initialConfig;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

}
