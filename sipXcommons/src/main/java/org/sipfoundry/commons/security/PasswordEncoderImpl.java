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
package org.sipfoundry.commons.security;


import org.springframework.beans.factory.annotation.Required;
import org.springframework.security.authentication.encoding.PasswordEncoder;

public class PasswordEncoderImpl implements PasswordEncoder{
	private String m_realm;

	@Override
	public String encodePassword(String rawPass, Object salt) {
        if (salt instanceof String) {
            return Md5Encoder.getEncodedPassword(rawPass);
        }
        return rawPass;
	}

	@Override
	public boolean isPasswordValid(String encPass, String rawPass, Object salt) {
        if (rawPass == null) {
            return false;
        }
        String pass = encodePassword(rawPass, salt);
        return pass.equals(encPass);
	}

	@Required
	public void setRealm(String realm) {
		m_realm = realm;
	}

}
