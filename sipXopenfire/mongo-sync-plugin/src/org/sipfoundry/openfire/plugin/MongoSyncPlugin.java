/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.plugin;

import java.io.File;

import org.apache.log4j.Logger;
import org.jivesoftware.openfire.container.Plugin;
import org.jivesoftware.openfire.container.PluginManager;
import org.sipfoundry.openfire.plugin.job.JobFactory;
import org.sipfoundry.openfire.plugin.listener.ImdbOplogListener;
import org.sipfoundry.openfire.plugin.listener.ProfilesOplogListener;

public class MongoSyncPlugin implements Plugin {
    private static Logger logger = Logger.getLogger(MongoSyncPlugin.class);

    @Override
    public void initializePlugin(PluginManager manager, File pluginDirectory) {
        String configNode = System.getProperty("confignode", "true");
        boolean isConfigNode = Boolean.parseBoolean(configNode);
        if (!isConfigNode) {
            logger.warn("MongoSyncPlugin: not a config node, sync threads won't be started");
        } else {
            logger.warn("Mongo sync plugin initializing");
            ThreadGroup group = new ThreadGroup("mongoSync");
            Thread imdbOpLogThread = new Thread(group, new ImdbOplogListener(new JobFactory()), "imdbOpLogListener");
            Thread profilesdbOpLogThread = new Thread(group, new ProfilesOplogListener(new JobFactory()), "profilesOpLogListener");
            imdbOpLogThread.start();
            profilesdbOpLogThread.start();
        }
    }

    @Override
    public void destroyPlugin() {
        logger.info("Mongo sync plugin stopping");
    }
}
