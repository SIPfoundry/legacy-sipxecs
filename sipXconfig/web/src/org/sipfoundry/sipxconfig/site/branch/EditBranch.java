/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.site.branch;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.branch.BranchManager;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class EditBranch extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "branch/EditBranch";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:branchManager")
    public abstract BranchManager getBranchManager();

    public abstract Branch getBranch();

    public abstract void setBranch(Branch branch);

    @Persist
    public abstract Integer getBranchId();

    public abstract void setBranchId(Integer branchId);

    public void pageBeginRender(PageEvent event_) {
        Integer branchId = getBranchId();
        Branch branch;
        if (branchId != null) {
            branch = getBranchManager().getBranch(branchId);
        } else {
            branch = new Branch();
        }
        setBranch(branch);
    }

    /*
     * If the input is valid, then save changes to the branch.
     */
    public void apply() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        Branch branch = getBranch();
        getBranchManager().saveBranch(branch);
        setBranchId(branch.getId());
    }
}
