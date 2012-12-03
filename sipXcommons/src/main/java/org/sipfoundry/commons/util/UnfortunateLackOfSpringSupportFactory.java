package org.sipfoundry.commons.util;

import java.net.UnknownHostException;

import org.sipfoundry.commons.mongo.MongoFactory;
import org.sipfoundry.commons.userdb.ValidUsers;

import com.mongodb.DB;
import com.mongodb.Mongo;

/**
 * Connection factory to mongo.  Need to call this before using
 *   UnfortunateLackOfSpringSupportFactory.initialize("/file/to/client/config");
 * File is probably
 *   @SIPX_CONFDIR@/mongo-client.ini
 */
public class UnfortunateLackOfSpringSupportFactory {
    private static ValidUsers s_validUsers;
    private static DB s_imdb;
    private static DB s_openfiredb;

    public synchronized static void initialize() throws UnknownHostException {
        if (s_validUsers == null) {

            // useful in unit tests to direct operations to imdb_TEST
            String imdbNs = System.getProperty("mongo_ns", "imdb");
            String openfireNs = System.getProperty("openfire_ns", "openfiredb");

            Mongo mongo = MongoFactory.fromConnectionFile();
            try {
                s_imdb = mongo.getDB(imdbNs);
                s_openfiredb = mongo.getDB(openfireNs);
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
            ValidUsers validUsers = new ValidUsers();
            validUsers.setImdb(s_imdb);
            // be sure this is the last successful thing you do to ensure
            // singleton is properly initialized
            s_validUsers = validUsers;
        }
    }

    public static ValidUsers getValidUsers() {
        checkinit();
        return s_validUsers;
    }

    public static DB getImdb() {
        checkinit();
        return s_imdb;
    }

    public static DB getOpenfiredb() {
        checkinit();
        return s_openfiredb;
    }

    public static String getConnectionURL() {
        checkinit();
        return MongoFactory.getConnectionURL();
    }

    private static void checkinit() {
        if (s_validUsers == null) {
            try {
				initialize();
			} catch (UnknownHostException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
        }
    }
}
