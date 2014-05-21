package org.sipfoundry.openfire.provider;

import static org.junit.Assert.assertEquals;

import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.provider.PubSubProvider;
import org.jivesoftware.openfire.pubsub.DefaultNodeConfiguration;
import org.jivesoftware.openfire.pubsub.PubSubService;
import org.junit.After;
import org.junit.Test;

import com.mongodb.BasicDBObject;

@SuppressWarnings("static-method")
public class PubsubProviderTest extends BaseMongoTest {
    private static final String TEST_LANGUAGE = "latin";

    @After
    public void teardown() {
        getOpenfireDb().getCollection("ofPubsubDefaultConf").remove(new BasicDBObject());
    }

    @Test
    public void testConfigWrite() {
        PubSubProvider provider = new MongoPubSubProvider();
        PubSubService srv = XMPPServer.getInstance().getPubSubModule();
        DefaultNodeConfiguration config = new DefaultNodeConfiguration(true);

        config.setLanguage(TEST_LANGUAGE);
        // some configurations are created on server initialization
        long before = getOpenfireDb().getCollection("ofPubsubDefaultConf").count();
        provider.createDefaultConfiguration(srv, config);

        assertEquals(before + 1, getOpenfireDb().getCollection("ofPubsubDefaultConf").count());
    }

    @Test
    public void testConfigRead() {
        PubSubProvider provider = new MongoPubSubProvider();
        PubSubService srv = XMPPServer.getInstance().getPubSubModule();
        DefaultNodeConfiguration config = new DefaultNodeConfiguration(true);

        config.setLanguage(TEST_LANGUAGE);
        provider.createDefaultConfiguration(srv, config);

        config.setLanguage(TEST_LANGUAGE);

        DefaultNodeConfiguration configFromDB = provider.loadDefaultConfiguration(srv, true);

        assertEquals(TEST_LANGUAGE, configFromDB.getLanguage());
    }
}
