package org.sipfoundry.sipxconfig.paging;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class PagingFeatureContextImplTestIntegration extends IntegrationTestCase {
    private PagingContext m_pagingContext;
    private PagingFeatureContext m_pagingFeatureContext;

    public void setPagingContext(PagingContext pagingContext) {
        m_pagingContext = pagingContext;
    }

    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        db().execute("select truncate_all()");
        sql("paging/paging.sql");
    }

    public void testDeletePagingGroupsById() throws Exception {
        List<PagingGroup> groups = m_pagingContext.getPagingGroups();
        assertEquals(3, groups.size());
        List<Integer> groupsIds = new ArrayList<Integer>();
        groupsIds.add(101);
        groupsIds.add(102);
        m_pagingFeatureContext.deletePagingGroupsById(groupsIds);
        groups = m_pagingContext.getPagingGroups();

        // 2 paging groups should disappear
        assertEquals(1, groups.size());
    }

    public void setPagingFeatureContext(PagingFeatureContext pagingFeatureContext) {
        m_pagingFeatureContext = pagingFeatureContext;
    }
}
