/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.gateway;

import org.apache.commons.collections.Predicate;
import org.sipfoundry.sipxconfig.device.FilteredModelSource;

public class GatewayModelSource extends FilteredModelSource<GatewayModel> {
    public GatewayModelSource() {
        setFilter(getPredicate());
    }

    static Predicate getPredicate() {
        return new Predicate() {
            public boolean evaluate(Object arg0) {
                GatewayModel model = (GatewayModel) arg0;
                return (!model.getModelId().startsWith("itsp"));
            }
        };
    }
}
