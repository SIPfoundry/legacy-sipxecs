package org.sipfoundry.sipxconfig.mongo;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.springframework.data.mongodb.core.MongoTemplate;


public class MongoTestIntegration extends IntegrationTestCase {
    private MongoTemplate m_imdb;

    public void testTemplateFactory() {
        assertNotNull(m_imdb);
    }

    public MongoTemplate getImdb() {
        return m_imdb;
    }

    public void setImdb(MongoTemplate imdb) {
        m_imdb = imdb;
    }
}
