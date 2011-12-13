package org.sipfoundry.commons.mongo;

import java.io.FileInputStream;
import java.io.IOException;
import java.net.UnknownHostException;
import java.util.Properties;

import org.apache.commons.io.IOUtils;

import com.mongodb.Mongo;
import com.mongodb.MongoURI;

public class MongoFactory {

    public static final Mongo fromConnectionFile(String configFile) throws UnknownHostException {
        String url = MongoFactory.readConfig(configFile);
        return fromConnectionString(url);
    }
    
    public static final Mongo fromConnectionString(String connectionUrl) throws UnknownHostException {
        MongoURI uri = new MongoURI(connectionUrl);
        return uri.connect();
    }
 
    static String readConfig(String configFile) {        
        Properties p = new Properties();
        FileInputStream in = null;
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
}
