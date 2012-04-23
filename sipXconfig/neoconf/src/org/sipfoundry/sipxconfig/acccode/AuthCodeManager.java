/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.acccode;

import java.io.Serializable;
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.common.DataObjectSource;

public interface AuthCodeManager extends DataObjectSource<AuthCode>, AliasOwner {
    AuthCode getAuthCode(Integer authCodeId);

    void saveAuthCode(AuthCode authCode);

    void deleteAuthCode(AuthCode authCode);

    void deleteAuthCodes(Collection<Integer> allSelected);

    List<AuthCode> getAuthCodes();

    List<AuthCode> getAuthCodes(Collection<Integer> ids);

    AuthCode getAuthCodeByCode(String code);

    AuthCode newAuthCode();

    AuthCode load(Class<AuthCode> c, Serializable id);
}
