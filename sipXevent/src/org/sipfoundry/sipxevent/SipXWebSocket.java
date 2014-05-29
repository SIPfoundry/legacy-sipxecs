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

import org.apache.commons.lang.StringUtils;
import org.eclipse.jetty.io.Connection;
import org.sipfoundry.commons.security.MongoUserDetailsService;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.security.authentication.encoding.PasswordEncoder;
import org.springframework.security.core.userdetails.UserDetails;

public class SipXWebSocket {
	public static final String CONTEXT_BEAN_NAME = "sipxWebSocket";
	private Connection m_connection;
	private String m_userId;
	private RegisteredClients m_registeredClients;
	private MongoUserDetailsService m_userDetailsService;
	private PasswordEncoder m_passwordEncoder;

	public SipXWebSocket() {
	}

	public void onOpen(Connection connection) {
		m_connection = connection;
	}

	public void onClose(int closeCode, String message) {
	    m_registeredClients.unregisterClient(this);
	}

	public void onMessage(String data) {
		String [] auth = StringUtils.split(data, ":");
		m_userId = auth[0];
		String password = auth[1];
		if (authenticateUser(m_userId, password)) {
		    m_registeredClients.registerClient(this);
			//sendPresenceMessage();
		} else {
			//try {
				//m_connection.sendMessage("not_valid");
			//} catch (IOException e) {
				// TODO Auto-generated catch block
				//e.printStackTrace();
			//}
		}
	}

	private boolean authenticateUser(String userId, String password) {
		UserDetails userDetails = m_userDetailsService.loadUserByUsername(userId);
		if (userDetails == null) {
			return false;
		}
		return userDetails.getPassword().equals(m_passwordEncoder.encodePassword(password, userId));
	}

	public String getUserId() {
		return m_userId;
	}

	public Connection getConnection() {
		return m_connection;
	}

    @Required
	public void setRegisteredClients(RegisteredClients registeredClients) {
        m_registeredClients = registeredClients;
    }

    @Required
	public void setUserDetailsService(MongoUserDetailsService userDetailsService) {
		m_userDetailsService = userDetailsService;
	}

	@Required
	public void setPasswordEncoder(PasswordEncoder passwordEncoder) {
		m_passwordEncoder = passwordEncoder;
	}
}
