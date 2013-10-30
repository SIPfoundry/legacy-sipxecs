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

import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.dialplan.DialingRuleProvider;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface MusicOnHoldManager extends DialingRuleProvider, AliasOwner {
    static final LocationFeature FEATURE = new LocationFeature("moh");

    String getAudioDirectoryPath();

    boolean isAudioDirectoryEmpty();

    MohAddressFactory getAddressFactory();

    File getAudioDirectory(String name);

    MohSettings getSettings();

    void saveSettings(MohSettings settings);
}
