package org.sipfoundry.openfire.provider;

import java.net.UnknownHostException;

import org.jivesoftware.openfire.XMPPServer;
import org.junit.AfterClass;
import org.junit.BeforeClass;

import com.mongodb.DB;
import com.mongodb.MongoClient;

/**
 * Setup and teardown for all tests going to OpenfireDB is the same.
 */
public abstract class BaseMongoTest {
    private static final String IM_DB_NAME = "imdb_TEST";
    private static final String OF_DB_NAME = "openfiredb_TEST";

    private static MongoClient mongoClient;
    private static DB openfiredb;
    private static DB imdb;
    private static XMPPServer server;

    @BeforeClass
    public static void classSetup() throws UnknownHostException {
        System.setProperty("mongo_ns", IM_DB_NAME);
        System.setProperty("openfire_ns", OF_DB_NAME);
        System.setProperty("openfireHome", "./mongo-lib/src/test/resources");
        System.setProperty("conf.dir", "./mongo-lib/src/test/resources");
        System.setProperty("provider.properties.className", "org.jivesoftware.util.FilePropertiesProvider");
        System.setProperty("configFile", "mongo-lib/src/test/resources/openfire.properties");

        mongoClient = new MongoClient();
        imdb = mongoClient.getDB(IM_DB_NAME);
        openfiredb = mongoClient.getDB(OF_DB_NAME);
        // if (server == null) {
        // synchronized (new byte[0]) {
        // if (server == null) {
        // server = new XMPPServer();
        // }
        // }
        // }
        try {
            server = new XMPPServer();
        } catch (IllegalStateException ex) {
            server = XMPPServer.getInstance();
        }
    }

    @AfterClass
    public static void classTeardown() {
        server.stop();
        mongoClient.dropDatabase(IM_DB_NAME);
        mongoClient.dropDatabase(OF_DB_NAME);
    }

    protected static DB getOpenfireDb() {
        return openfiredb;
    }

    protected static DB getImdb() {
        return imdb;
    }
}
