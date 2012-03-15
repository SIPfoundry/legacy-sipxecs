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

import org.jivesoftware.openfire.container.Plugin;
import org.jivesoftware.openfire.container.PluginManager;

public class WebSocketPlugin implements Plugin {

	@Override
	public void initializePlugin(PluginManager pluginManager, File file) {
		System.out.println("Initialize web socket plugin "+file.getAbsolutePath());

		WebSocketServer server = new WebSocketServer(file.getAbsolutePath());
		new ServerThread(server).start();
		System.out.println("Web socket plugin initialized");
	}

	@Override
	public void destroyPlugin() {
		// TODO Auto-generated method stub

	}
	private class ServerThread extends Thread {
		WebSocketServer m_server;

		public ServerThread(WebSocketServer server) {
			m_server = server;
		}
		@Override
		public void run() {
			m_server.start();
		}
	}
}
