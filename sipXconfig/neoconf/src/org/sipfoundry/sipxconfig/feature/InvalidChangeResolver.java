/**
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

import org.sipfoundry.sipxconfig.commserver.Location;

/**
 * Resolve as many invalid changes as you can by automatically changing the request
 * when there's no options the user would have anyway.
 */
public class InvalidChangeResolver {

    /**
     * @return true is request was modified. this means you have to resend change request to be validated
     */
    boolean resolve(FeatureChangeValidator validator, Location primary) {
        boolean changedRequest = false;
        FeatureChangeRequest request = validator.getRequest();
        boolean singleLocation = request.getEnableByLocation().size() == 1;
        for (InvalidChange change : validator.getInvalidChanges()) {
            if (change.getFeature() instanceof GlobalFeature) {
                request.enableFeature((GlobalFeature) change.getFeature(), true);
            } else if (singleLocation) {
                request.enableLocationFeature((LocationFeature) change.getFeature(), primary, true);
            } else if (change.getLocation() != null) {
                request.enableLocationFeature((LocationFeature) change.getFeature(), change.getLocation(), true);
            } else {
                continue;
            }
            changedRequest = true;
        }
        if (changedRequest) {
            // best to clear all changes, validator is in an undetermined state unti
            // it can be sent to all FeatureListeners again
            validator.getInvalidChanges().clear();
        }
        return changedRequest;
    }
}
