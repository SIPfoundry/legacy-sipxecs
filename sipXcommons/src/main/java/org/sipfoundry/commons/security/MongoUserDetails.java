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

import static org.sipfoundry.commons.security.UserRole.User;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.springframework.security.core.GrantedAuthority;
import org.springframework.security.core.userdetails.UserDetails;

public class MongoUserDetails implements UserDetails {
	String m_userName;
	String m_password;

	public MongoUserDetails(String userName, String password) {
		m_userName = userName;
		m_password = password;
	}
	@Override
	public Collection<? extends GrantedAuthority> getAuthorities() {
        List<GrantedAuthority> gas = new ArrayList<GrantedAuthority>(1);
        gas.add(User.toAuth());
        return gas;
	}

	@Override
	public String getPassword() {
		return m_password;
	}

	@Override
	public String getUsername() {
		return m_userName;
	}

	@Override
	public boolean isAccountNonExpired() {
		return true;
	}

	@Override
	public boolean isAccountNonLocked() {
		return true;
	}

	@Override
	public boolean isCredentialsNonExpired() {
		return true;
	}

	@Override
	public boolean isEnabled() {
		return true;
	}
}

