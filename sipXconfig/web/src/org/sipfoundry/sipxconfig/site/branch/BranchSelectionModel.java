/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.site.branch;

import java.util.Collection;

import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;

public class BranchSelectionModel extends ObjectSelectionModel {
    public BranchSelectionModel(Collection <Branch> branches) {
        setCollection(branches);
        setLabelExpression("name");
    }
}
