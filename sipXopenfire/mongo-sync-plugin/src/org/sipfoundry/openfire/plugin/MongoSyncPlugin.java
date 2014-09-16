/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
