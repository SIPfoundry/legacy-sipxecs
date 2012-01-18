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

import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.dialplan.DialingRuleProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;

public interface MusicOnHoldManager extends DialingRuleProvider, AliasOwner {
    static final GlobalFeature FEATURE = new GlobalFeature("moh");

    String getAudioDirectoryPath();

    boolean isAudioDirectoryEmpty();

    MohAddressFactory getAddressFactory();
}
