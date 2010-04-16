/*
 *  Copyright (C) 2010 Avaya Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.clientauthutils.UserCredentials;

public class UserCredentialsImpl implements UserCredentials {

	private String userName;
	private String sipDomain;
	private String password;

	public UserCredentialsImpl(String userName, String sipDomain,
			String password) {
		this.userName = userName;
		this.sipDomain = sipDomain;
		this.password = password;
	}

	public String getPassword() {
		return this.password;
	}

	public String getSipDomain() {
			return this.sipDomain;
	}

	public String getUserName() {
		return this.userName;
	}

}
