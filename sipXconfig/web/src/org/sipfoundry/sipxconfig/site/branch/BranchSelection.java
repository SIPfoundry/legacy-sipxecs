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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.ObjectUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.branch.BranchManager;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;

public abstract class BranchSelection extends BaseComponent {

    @InjectObject("spring:branchManager")
    public abstract BranchManager getBranchManager();

    @InjectObject("spring:tapestry")
    public abstract TapestryContext getTapestry();

    @Parameter(required = true)
    public abstract Branch getBranch();

    public abstract void setBranch(Branch branch);

    public abstract void setSelectedAction(IActionListener selectedAction);

    public abstract IActionListener getSelectedAction();

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        Branch branch = getBranch();
        if (branch != null) {
            setSelectedAction(new BranchAction(branch));
        } else {
            setSelectedAction(null);
        }

        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this) && TapestryUtils.isValid(this)) {
            triggerAction(cycle);
        }
    }

    private void triggerAction(IRequestCycle cycle) {
        IActionListener a = getSelectedAction();
        if (!(a instanceof BranchAdapter)) {
            return;
        }

        BranchAdapter action = (BranchAdapter) a;
        Branch branch = action.getSelectedBranch();
        if (branch != null) {
            action.setId(branch.getId());
        }
        action.actionTriggered(this, cycle);
    }

    public IPropertySelectionModel getBranchesModel() {
        List<Branch> branches = getBranchManager().getBranches();
        Collection<OptionAdapter<Branch>> options = new ArrayList<OptionAdapter<Branch>>();
        for (Branch branch : branches) {
            BranchAction adapter = new BranchAction(branch);
            options.add(adapter);
        }
        AdaptedSelectionModel branchSelectionModel = new AdaptedSelectionModel();
        branchSelectionModel.setCollection(options);
        return getTapestry().instructUserToSelect(branchSelectionModel, getMessages());
    }

    private class BranchAction extends BranchAdapter {
        public BranchAction(Branch branch) {
            super(branch);
        }

        public void actionTriggered(IComponent component, final IRequestCycle cycle) {
            setBranch(getSelectedBranch());
        }

        @Override
        public Object getValue(Object option, int index) {
            return this;
        }

        @Override
        public String squeezeOption(Object option, int index) {
            return getSelectedBranch().getId().toString();
        }

        @Override
        public boolean equals(Object obj) {
            return ObjectUtils.equals(this.getSelectedBranch(), ((BranchAction) obj).getSelectedBranch());
        }

        @Override
        public int hashCode() {
            return this.getSelectedBranch().hashCode();
        }
    }
}
