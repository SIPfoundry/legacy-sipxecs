/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.commons.redis;

import java.io.FileInputStream;
import java.io.IOException;
import java.util.Properties;

import org.apache.commons.io.IOUtils;
import org.springframework.data.redis.connection.jedis.JedisConnectionFactory;

/**
 * Creates Redis connection from a properties file for spring based projects
 * 
 */
public class RedisSpringFactory extends JedisConnectionFactory {

    public RedisSpringFactory(String configFile) {
        super();
        readConfig(configFile);
    }

    private void readConfig(String configFile) {        
        Properties p = new Properties();
        FileInputStream in = null;
        try {
            in = new FileInputStream(configFile);
            p.load(in);
        } catch (IOException e) {
            destroy();
            return;
        } finally {
            IOUtils.closeQuietly(in);            
        }
        if (Boolean.valueOf(p.getProperty("enabled"))) {
            setHostName(p.getProperty("tcp-address"));
            setPort(Integer.valueOf(p.getProperty("tcp-port")));
        } else {
            destroy();
        }
    }
}
