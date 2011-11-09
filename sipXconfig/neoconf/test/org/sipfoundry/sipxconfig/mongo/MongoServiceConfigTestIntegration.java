package org.sipfoundry.sipxconfig.mongo;

import java.io.IOException;
import java.io.StringWriter;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.Location;


public class MongoServiceConfigTestIntegration extends IntegrationTestCase {
    private MongoServiceConfig m_mongoServiceConfig;
    private static final String[] EXPECTED = new String[] {
        "logappend = true", 
        "fork = true", 
        "port = 27107", 
        "replSet = sipxecs", 
        "nohttpinterface = false",
        "logpath=@SIPX_LOGDIR@/mongod.log", 
        "dbpath=@SIPX_VARDIR@/mongo",
        ""
    };

    public void testConfig() throws IOException {
        Location location = null;
        StringWriter actual = new StringWriter();
        m_mongoServiceConfig.write(actual, location);
        assertEquals(StringUtils.join(EXPECTED, "\n"), actual.toString());
    }

    public MongoServiceConfig getMongoServiceConfig() {
        return m_mongoServiceConfig;
    }

    public void setMongoServiceConfig(MongoServiceConfig mongoServiceConfig) {
        m_mongoServiceConfig = mongoServiceConfig;
    }
}
