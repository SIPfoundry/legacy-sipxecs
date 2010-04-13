/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.skin;

import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import org.springframework.beans.factory.config.ConfigurableListableBeanFactory;
import org.springframework.beans.factory.config.PropertyResourceConfigurer;
import org.springframework.core.io.ClassPathResource;

public class SkinProperties extends PropertyResourceConfigurer {

    @Override
    protected void processProperties(ConfigurableListableBeanFactory beanFactory, Properties props) {
        SkinControl skin = (SkinControl) beanFactory.getBean("skin");
        Map<String, String> assets = new HashMap(props.size());
        for (Object key : props.keySet()) {
            assets.put((String) key, (String) props.get(key));
        }
        skin.setAssets(assets);
    }

    /**
     * convience method to set location w/o ClassPathResource in bean file.
     * @param path
     */
    public void setLocationPath(String path) {
        ClassPathResource location = new ClassPathResource(path);
        setLocation(location);
    }
}
