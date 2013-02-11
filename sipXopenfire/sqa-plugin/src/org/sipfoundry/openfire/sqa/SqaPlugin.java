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
package org.sipfoundry.openfire.sqa;

import static org.apache.commons.lang.StringUtils.isBlank;
import ietf.params.xml.ns.dialog_info.DialogInfo;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;

import org.jivesoftware.openfire.container.Plugin;
import org.jivesoftware.openfire.container.PluginManager;
import org.jivesoftware.openfire.user.PresenceEventDispatcher;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;
import org.sipfoundry.sqaclient.SQAEvent;
import org.sipfoundry.sqaclient.SQAWatcher;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class SqaPlugin implements Plugin {
    Map<String, SipPresenceBean> m_presenceCache = new HashMap<String, SipPresenceBean>();

    private static final String INITALIZATION_EXCEPTION = "SqaPlugin initialization exception";
    private static final Logger logger = LoggerFactory.getLogger(SqaPlugin.class);
    @Override
    public void initializePlugin(PluginManager manager, File pluginDirectory) {
        try {
            String configurationPath = System.getProperty("conf.dir");
            String libPath = System.getProperty("lib.dir");
            String presence = System.getProperty("openfire.presence");
            if (isBlank(configurationPath) || isBlank(libPath) || isBlank(presence)) {
                System.getProperties().load(new FileInputStream(new File("/tmp/sipx.properties")));
                configurationPath = System.getProperty("conf.dir", "/etc/sipxpbx");
                libPath = System.getProperty("lib.dir", "/lib");
                presence = System.getProperty("openfire.presence", "true");
            }

            UnfortunateLackOfSpringSupportFactory.initialize();

            if (Boolean.valueOf(presence)) {
                System.load(libPath + "/libsqaclient.so");
                SQAWatcher watcher = new SQAWatcher("openfire", "sswdata", 1, 100, 100);
                logger.info("Connected: " + watcher.isConnected());

                JAXBContext context = JAXBContext.newInstance(DialogInfo.class);

                new SqaSubscriberThread(watcher, context, m_presenceCache).start();

                PresenceEventDispatcher.addListener(new PresenceEventListenerImpl(m_presenceCache));

                logger.info("SQA subscriber started...");
            } else {
                logger.info("XMPP presence not enabled");
            }
        } catch (SecurityException e) {
            logger.error(INITALIZATION_EXCEPTION, e);
        } catch (IllegalArgumentException e) {
            logger.error(INITALIZATION_EXCEPTION, e);
        } catch (FileNotFoundException e) {
            logger.error(INITALIZATION_EXCEPTION, e);
        } catch (IOException e) {
            logger.error(INITALIZATION_EXCEPTION, e);
        } catch (JAXBException e) {
            logger.error(INITALIZATION_EXCEPTION, e);
        }
    }

    @Override
    public void destroyPlugin() {
        // TODO Auto-generated method stub

    }
    private class SqaSubscriberThread extends Thread {
        SQAWatcher m_watcher;
        JAXBContext m_context;
        Map<String, SipPresenceBean> m_presenceCache = null;

        public SqaSubscriberThread(SQAWatcher watcher, JAXBContext context, Map<String, SipPresenceBean> presenceCache) {
            m_watcher = watcher;
            m_context = context;
            m_presenceCache = presenceCache;
        }
        @Override
        public void run() {
            ExecutorService executor = Executors.newFixedThreadPool(10);
            logger.info("Start watching...");
            while (true) {
                SQAEvent event = m_watcher.watch();
                logger.debug("Execute event handler "+event.getId());
                executor.execute(new SqaEventHandler(event, m_context, m_presenceCache));
            }
        }
    }
}
