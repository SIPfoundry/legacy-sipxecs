/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.openfire.ws;

import java.io.File;
import java.io.IOException;
import java.security.GeneralSecurityException;

import org.apache.commons.httpclient.contrib.ssl.EasySSLProtocolSocketFactory;
import org.apache.commons.httpclient.protocol.Protocol;
import org.apache.commons.httpclient.protocol.ProtocolSocketFactory;
import org.jivesoftware.openfire.container.Plugin;
import org.jivesoftware.openfire.container.PluginManager;
import org.jivesoftware.openfire.user.PresenceEventDispatcher;

public class WebSocketPlugin implements Plugin {

	@Override
	public void initializePlugin(PluginManager pluginManager, File file) {
	    trustAllCerts();
	    PresenceEventDispatcher.addListener(new PresenceEventListenerImpl());
	}

	@Override
	public void destroyPlugin() {
	}

    public void trustAllCerts(){
        ProtocolSocketFactory sf;
        try {
            sf = new EasySSLProtocolSocketFactory();
            Protocol p = new Protocol("https", sf, 443);
            Protocol.registerProtocol("https", p);
        } catch (GeneralSecurityException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
