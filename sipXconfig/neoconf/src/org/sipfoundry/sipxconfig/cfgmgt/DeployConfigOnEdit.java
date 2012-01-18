/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import java.util.Collection;

import org.sipfoundry.sipxconfig.feature.Feature;

/**
 * Only to be implemented by beans that are persisted thru hibernate. For everything else, you can
 * explicitly flag required replication through replication manager.
 */
public interface DeployConfigOnEdit {

    public Collection<Feature> getAffectedFeaturesOnChange();

}
