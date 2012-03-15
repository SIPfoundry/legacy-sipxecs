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

import java.io.IOException;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.eclipse.jetty.websocket.WebSocket;
import org.eclipse.jetty.websocket.WebSocketFactory;
import org.jivesoftware.openfire.user.PresenceEventDispatcher;
import org.springframework.beans.BeansException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.web.HttpRequestHandler;


public class PresenceServlet implements HttpRequestHandler, BeanFactoryAware {
	private WebSocketFactory wsFactory;
	private PresenceEventListenerImpl m_presenceEventListener;
	private BeanFactory m_beanFactory;

	public void init() {
		// Create and configure WS factory

		wsFactory = new WebSocketFactory(new WebSocketFactory.Acceptor() {
			public boolean checkOrigin(HttpServletRequest request, String origin) {
				// Allow all origins
				return true;
			}

			public WebSocket doWebSocketConnect(HttpServletRequest request, String protocol) {
				return createPresenceWebSocket();
			}
		});
		wsFactory.setBufferSize(4096);
		wsFactory.setMaxIdleTime(60000);
		PresenceEventDispatcher.addListener(m_presenceEventListener);
	}

	@Override
	public void handleRequest(HttpServletRequest request, HttpServletResponse response)
			throws ServletException, IOException {
		if (wsFactory.acceptWebSocket(request, response)) {
			return;
		}
		response.sendError(HttpServletResponse.SC_SERVICE_UNAVAILABLE,
				"Websocket only");
	}

	@Required
	public void setPresenceEventListener(PresenceEventListenerImpl presenceEventListener) {
		m_presenceEventListener = presenceEventListener;
	}

	@Override
	public void setBeanFactory(BeanFactory beanFactory) throws BeansException {
		m_beanFactory = beanFactory;
	}

   protected PresenceWebSocket createPresenceWebSocket() {
	   return (PresenceWebSocket) this.m_beanFactory.getBean(PresenceWebSocket.CONTEXT_BEAN_NAME); // notice the Spring API dependency
   }
}
