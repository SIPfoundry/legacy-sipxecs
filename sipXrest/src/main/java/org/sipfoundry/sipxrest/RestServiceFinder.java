/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrest;

import java.io.File;
import java.io.FileInputStream;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.List;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.JarInputStream;

import org.restlet.Restlet;
import org.apache.log4j.Logger;

public class RestServiceFinder {

    private static Logger logger = Logger.getLogger(RestServiceFinder.class);

    private HashSet<Plugin> pluginCollection = new HashSet<Plugin>();
    private HashSet<MetaInf> plugins = new HashSet<MetaInf>();

    class JarFilter implements FilenameFilter {
        public boolean accept(File dir, String name) {
            return (name.endsWith(".jar"));
        }
    }

    public RestServiceFinder() {

    }

    public void search(String directory) throws Exception {
        logger.debug("Location to search " + directory);
        File dir = new File(directory);
        logger.debug("Searching directory for plugins");
        if (dir.isFile()) {
            return;
        }

        /*
         * For each Jar file in the directory, look for meta data In the meta data we have a
         * pointer to where the main class is residing. This class must implement the Plugin
         * interface.
         */
        File[] files = dir.listFiles(new JarFilter());
        for (File f : files) {
            logger.debug("Checking " + f.getName());
            if (f.isFile()) {
                JarFile jarFile = new JarFile(f);
                JarEntry jarEntry = null;
                Enumeration<JarEntry> entries = jarFile.entries();
                while (entries.hasMoreElements()) {
                    jarEntry = entries.nextElement();
                    if (jarEntry.getName().equals("plugin.xml")) {

                        InputStream metaInfFile = jarFile.getInputStream(jarEntry);

                        if (metaInfFile != null) {
                            try {
                                boolean conflict = false;
                                MetaInfParser metaInfParser = new MetaInfParser();

                                MetaInf metaInf = metaInfParser.parse(metaInfFile);
                                for (MetaInf mi : this.plugins) {
                                    if (mi.getUriPrefix().equals(metaInf.getUriPrefix())) {
                                        logger
                                                .error("Cannot load plugin - url prefix not unqiue "
                                                        + mi.getUriPrefix());
                                        conflict = true;
                                        break;
                                    }

                                }
                                if (!conflict) {
                                    this.plugins.add(metaInf);
                                }
                            } catch (Exception ex) {
                                logger.error("Exception in parsing meta inf file for "
                                        + f.getName(), ex);
                            }
                        }
                        break;
                    }
                }

            }
        }

        for (MetaInf mi : this.plugins) {
            try {
                String className = mi.getPluginClass();

                Class< ? > clazz = Class.forName(className);
                if (Plugin.class.isAssignableFrom(clazz)) {
                    logger.debug("Plugin support found in  = " + className);
                    Plugin plugin = (Plugin) clazz.newInstance();
                    plugin.setMetaInf(mi);
                    pluginCollection.add(plugin);
                    if ( plugin.getSpecialUserName() != null && plugin.getSpecialUsersClearTextSipPassword() != null ) {
                        RestServer.getAccountManager().addAccount(plugin.getSpecialUserName(), 
                                plugin.getSpecialUsersClearTextSipPassword());
                    }
                }
                /*
                 * A plugin should not initialize its own logging. This will be done by the container.
                 */
                Logger.getLogger(clazz.getPackage().getName()).addAppender(RestServer.getAppender());
            } catch (Exception ex) {
                logger.error("Error loading class", ex);
            }
            String sipListenerClassName = mi.getSipListenerClassName();
            if (sipListenerClassName != null) {
                try {
                    Class< ? > clazz = Class.forName(sipListenerClassName);
                    if (AbstractSipListener.class.isAssignableFrom(clazz)) {
                        logger.debug("SipListener support found in  = " + sipListenerClassName);
                        AbstractSipListener sipListener = (AbstractSipListener) clazz
                                .newInstance();
                        sipListener.setMetaInf(mi);
                        SipStackBean stackBean = RestServer.getSipStack();
                        stackBean.sipListener.addServiceListener(mi, sipListener);
                    }
                } catch (Exception ex) {
                    logger.error("error loading class", ex);
                }
            }
        }

    }

    public String getDescriptions() {
        StringBuffer descriptionPage = new StringBuffer();
        descriptionPage.append("<html><body>\n");
        descriptionPage.append("<h1>Available plugins</h1>\n");
        descriptionPage.append("<ul>\n");
        for (MetaInf mi : this.plugins) {
            descriptionPage.append("<li>\n");
            descriptionPage.append("<b> URI Prefix : " + mi.getUriPrefix() + "</b>\n");
            descriptionPage.append("</li>\n");
            descriptionPage.append("<li><b>Security :</b>" + mi.getSecurity() + "</li>");
            if (mi.getSipConvergenceName() != null) {
                descriptionPage.append("<li> SIP Service Name :</b>" + mi.getSipConvergenceName() + "</li>");
            }
            descriptionPage.append("<li><b>Description : </b> \n");
            descriptionPage.append(mi.getServiceDescription());
            descriptionPage.append("</li>\n");
        }
        descriptionPage.append("</ul>\n");
        descriptionPage.append("</body></html>");
        return descriptionPage.toString();
    }

    public Collection<Plugin> getPluginCollection() {
        return pluginCollection;
    }

}
