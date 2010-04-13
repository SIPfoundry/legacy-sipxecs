/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.branch;

import org.apache.tapestry.IActionListener;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;

public abstract class BranchAdapter implements OptionAdapter, IActionListener {

    private Integer m_id;

    private Branch m_branch;

    public BranchAdapter(Branch branch) {
        m_branch = branch;
    }

    public Integer getId() {
        return m_id;
    }

    public void setId(Integer id) {
        m_id = id;
    }

    public Branch getSelectedBranch() {
        return m_branch;
    }

    public void setSelectedBranch(Branch branch) {
        m_branch = branch;
    }

    public Object getValue(Object option, int index) {
        return this;
    }

    public String getLabel(Object option, int index) {
        return m_branch.getName();
    }

    public String squeezeOption(Object option, int index) {
        return m_branch.getId().toString();
    }

    public String getMethodName() {
        return null;
    }
}
