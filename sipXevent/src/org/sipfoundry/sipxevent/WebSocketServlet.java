/**
 *
 *
 * Copyright (c) 2010 / 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxevent;

import java.io.IOException;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.springframework.beans.BeansException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.web.HttpRequestHandler;


public class WebSocketServlet implements HttpRequestHandler, BeanFactoryAware {
	//private WebSocketFactory wsFactory;
	private BeanFactory m_beanFactory;

	public void init() {
		// Create and configure WS factory

		//wsFactory = new WebSocketFactory(new WebSocketFactory.Acceptor() {
			//public boolean checkOrigin(HttpServletRequest request, String origin) {
				// Allow all origins
				//return true;
			//}

			//public WebSocket doWebSocketConnect(HttpServletRequest request, String protocol) {
				//return createSipXWebSocket();
			//}
		//});
		//wsFactory.setBufferSize(4096);
		//wsFactory.setMaxIdleTime(60000);
	}

	@Override
	public void handleRequest(HttpServletRequest request, HttpServletResponse response)
			throws ServletException, IOException {
		//if (wsFactory.acceptWebSocket(request, response)) {
			//return;
		//}
		response.sendError(HttpServletResponse.SC_SERVICE_UNAVAILABLE,
				"Websocket only");
	}

	@Override
	public void setBeanFactory(BeanFactory beanFactory) throws BeansException {
		m_beanFactory = beanFactory;
	}

   protected SipXWebSocket createSipXWebSocket() {
	   return (SipXWebSocket) this.m_beanFactory.getBean(SipXWebSocket.CONTEXT_BEAN_NAME); // notice the Spring API dependency
   }
}
