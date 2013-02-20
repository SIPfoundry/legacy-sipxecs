/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.site.branch;

import java.util.Collections;
import java.util.List;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.branch.BranchManager;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.time.NtpManager;

import com.davekoelle.AlphanumComparator;

public abstract class EditBranch extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "branch/EditBranch";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:branchManager")
    public abstract BranchManager getBranchManager();

    @InjectObject("spring:ntpManager")
    public abstract NtpManager getTimeManager();

    public abstract Branch getBranch();

    public abstract void setBranch(Branch branch);

    @Persist
    public abstract Integer getBranchId();

    public abstract void setBranchId(Integer branchId);

    public abstract String getTimezoneType();

    public abstract void setTimezoneType(String type);

    public abstract IPropertySelectionModel getTimezoneTypeModel();

    public abstract void setTimezoneTypeModel(IPropertySelectionModel model);

    public void pageBeginRender(PageEvent event_) {
        Integer branchId = getBranchId();
        Branch branch;
        if (branchId != null) {
            branch = getBranchManager().getBranch(branchId);
        } else {
            branch = new Branch();
        }
        // Init. the timezone dropdown menu.
        List<String> timezoneList = getTimeManager().getAvailableTimezones();

        // Sort list alphanumerically.
        Collections.sort(timezoneList, new AlphanumComparator());
        StringPropertySelectionModel model = new StringPropertySelectionModel(
                timezoneList.toArray(new String[timezoneList.size()]));
        setTimezoneTypeModel(model);

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
