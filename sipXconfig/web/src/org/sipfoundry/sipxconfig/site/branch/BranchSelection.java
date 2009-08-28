/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.site.branch;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.branch.BranchManager;
import org.sipfoundry.sipxconfig.components.TapestryContext;

public abstract class BranchSelection extends BaseComponent {

    @InjectObject("spring:branchManager")
    public abstract BranchManager getBranchManager();

    @InjectObject("spring:tapestry")
    public abstract TapestryContext getTapestry();

    @Parameter(required = true)
    public abstract Branch getBranch();

    public IPropertySelectionModel getBranchesModel() {
        List<Branch> branches = getBranchManager().getBranches();
        IPropertySelectionModel branchSelectionModel = new BranchSelectionModel(branches);
        return getTapestry().instructUserToSelect(branchSelectionModel, getMessages());
    }
}
