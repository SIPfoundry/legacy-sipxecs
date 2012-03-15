/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
