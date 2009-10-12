/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.branch;

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.common.DataObjectSource;

public interface BranchManager extends DataObjectSource<Branch> {

    Branch getBranch(String name);

    Branch getBranch(Integer branchId);

    void saveBranch(Branch branch);

    void deleteBranch(Branch branch);

    void deleteBranches(Collection<Integer> allSelected);

    List<Branch> getBranches();

    List<Branch> loadBranchesByPage(final int firstRow, final int pageSize, final String[] orderBy,
            final boolean orderAscending);

    void clear();
}
