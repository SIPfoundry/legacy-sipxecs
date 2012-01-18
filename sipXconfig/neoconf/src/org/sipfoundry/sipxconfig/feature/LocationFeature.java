/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.feature;

/**
 * Unique ID representing a feature that applies to a location. FeatureManager will return
 * collection of LocationFeature objects that are enabled in a location.
 *
 * Spring beans "invent" a new Location Feature simply by returning a instance of a
 * LocationFeature class ensure the id universally unique to all existing and future location
 * feature objects. Bean IDs make fairly good IDs.
 *
 * Classes are final to ensure features are not overloaded to misused to communicate feature
 * specifics. For that consider alternate objects. If you can think of good reason to extend
 * this object, open up discussion.
 */
public final class LocationFeature extends Feature {
    public LocationFeature(String id) {
        super(id);
    }
}
