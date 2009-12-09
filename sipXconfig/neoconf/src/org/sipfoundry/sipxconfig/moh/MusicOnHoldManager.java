/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.moh;

import java.io.File;

import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleProvider;
import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.common.User;

public interface MusicOnHoldManager extends DialingRuleProvider, AliasOwner, AliasProvider {

    String getAudioDirectoryPath();

    boolean isAudioDirectoryEmpty();

    void replicateMohConfiguration();

    String getDefaultMohUri();

    String getLocalFilesMohUri();

    String getPortAudioMohUri();

    String getPersonalMohFilesUri(String userName);

    File getUserAudioDirectory(User user);
}
