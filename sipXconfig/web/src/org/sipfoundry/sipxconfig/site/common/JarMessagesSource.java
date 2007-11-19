/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * 
 */
package org.sipfoundry.sipxconfig.site.common;

import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hivemind.Messages;
import org.apache.hivemind.Resource;
import org.apache.hivemind.util.LocalizedNameGenerator;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.services.ComponentMessagesSource;
import org.apache.tapestry.services.impl.ComponentMessages;

public class JarMessagesSource implements ComponentMessagesSource {
    private static final String WEB_INF_STRING = "WEB-INF";

    private static final Log LOG = LogFactory.getLog(JarMessagesSource.class);

    private JarMessagesSourceContext m_context;
    private ComponentMessagesSource m_systemMessagesSource;
    private URLClassLoader m_jarClassLoader;
    
    public void setSystemMessagesSource(ComponentMessagesSource systemMessagesSource) {
        m_systemMessagesSource = systemMessagesSource;
    }
    
    public void setContext(JarMessagesSourceContext context) {
        m_context = context;
    }
    
    public Messages getMessages(IComponent component) {
        Properties messageProperties = loadMessagesFromJar(component);
        
        if (messageProperties == null) {
            return m_systemMessagesSource.getMessages(component);
        }
        
        return new ComponentMessages(component.getPage().getLocale(),
                messageProperties);
        
    }
    
    private Properties loadMessagesFromJar(IComponent component) {
        if (m_jarClassLoader == null) {
            try {
                initializeClassLoader();
            } catch (IOException ioe) {
                LOG.error("Error initializing messages class loader.", ioe);
                return null;
            }
        }
        
        Resource specificationLocation = component.getSpecification().getSpecificationLocation();
        String specificationName = specificationLocation.getName();
        String basename = specificationName.substring(0, specificationName.lastIndexOf('.'));
        LocalizedNameGenerator localizedNameGenerator = new LocalizedNameGenerator(basename, 
                component.getPage().getLocale(), ".properties");
        
        List<Resource> localizedNameList = new ArrayList<Resource>();
        while (localizedNameGenerator.more()) {
            String localizedName = localizedNameGenerator.next();
            localizedNameList.add(specificationLocation.getRelativeResource(localizedName));
        }
        
        InputStream stream = null;
        for (Resource resource : localizedNameList) {
            String resourcePath = resource.getPath();
            
            // only handle resource paths that contain "WEB-INF"/ (i.e. part of 
            // the sipXconfig web app
            if (!resourcePath.contains(WEB_INF_STRING)) {
                continue;
            }
            
            // strip off any portion of the resource path preceding "WEB-INF" 
            // in order to load the correct messages
            String contextPath = resourcePath.substring(resourcePath.indexOf(WEB_INF_STRING));
            stream = m_jarClassLoader.getResourceAsStream(contextPath);
            
            // once we are able to load a stream for a resource, exit loop
            if (stream != null) {
                break;
            }
        }
        
        if (stream == null) {
            return null;
        }
        
        Properties messages = new Properties();
        try {
            messages.load(stream);
        } catch (IOException ioe) {
            LOG.error("Error loading localized messages.", ioe);
            return null;
        }
        return messages;
    }
    
    private void initializeClassLoader() throws IOException {
        if (m_context == null) {
            throw new IllegalStateException("Context must be set.");
        }
        File localizationPackageRootDir = new File(m_context.getLocalizationPackageRoot());
        File[] jarFiles = localizationPackageRootDir.listFiles(new FilenameFilter() {
            public boolean accept(File dir, String name) {
                return name.matches("sipxconfig_.{2}\\.jar");
            }
        });
        URL[] jarUrls = new URL[jarFiles.length];
        for (int i = 0; i < jarFiles.length; i++) {
            jarUrls[i] = jarFiles[i].toURL();
        }
        
        m_jarClassLoader = new URLClassLoader(jarUrls);
    }
    
}
