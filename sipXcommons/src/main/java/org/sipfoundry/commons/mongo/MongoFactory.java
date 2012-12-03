package org.sipfoundry.commons.mongo;

import static org.apache.commons.lang.StringUtils.isBlank;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.UnknownHostException;
import java.util.Properties;

import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;

import com.mongodb.Mongo;
import com.mongodb.MongoURI;
import com.mongodb.WriteConcern;

public class MongoFactory {
    private static final Logger log = Logger.getLogger(MongoFactory.class);

    private static final byte[] FILE_LOCK = new byte[0];

    private static String connectionURL;

    public static final Mongo fromConnectionFile() throws UnknownHostException {
        if (connectionURL == null) {
            synchronized (FILE_LOCK) {
                String configurationPath = System.getProperty("conf.dir", "/etc/sipxpbx");
                @SuppressWarnings("resource")
                InputStream is = null;
                String config = null;

                try {
                    if (isBlank(configurationPath)) {
                        File openfireTmp = new File("/tmp/sipx.properties");
                        if (openfireTmp.exists()) {
                            is = new FileInputStream(openfireTmp);
                            System.getProperties().load(is);
                            configurationPath = System.getProperty("conf.dir", "/etc/sipxpbx");
                        }
                    }
                    config = configurationPath + "/mongo-client.ini";
                    connectionURL = MongoFactory.readConfig(config);
                } catch (IOException e) {
                    log.error("Error getting connection URL from [" + config + "]: " + e.getMessage());
                } finally {
                    IOUtils.closeQuietly(is);
                }
            }
        }

        return fromConnectionString(connectionURL);
    }

    public static final Mongo fromConnectionString(String connectionUrl) throws UnknownHostException {
        MongoURI uri = new MongoURI(connectionUrl);
        Mongo m = uri.connect();

        // set explicitly, in case the driver changes the default value
        m.setWriteConcern(WriteConcern.ACKNOWLEDGED);

        return m;
    }

    @SuppressWarnings("resource")
    public static String readConfig(String configFile) {
        Properties p = new Properties();
        InputStream in = null;
        try {
            in = new FileInputStream(configFile);
            p.load(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(in);
        }
        return p.getProperty("connectionUrl");
    }

    public static String getConnectionURL() {
        return connectionURL;
    }
}
