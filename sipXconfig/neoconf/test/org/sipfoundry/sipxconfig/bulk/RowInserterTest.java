/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.bulk;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.job.JobContext;
import org.springframework.transaction.PlatformTransactionManager;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class RowInserterTest extends TestCase {

    private DummyRowInserter m_ri;

    protected void setUp() throws Exception {
        m_ri = new DummyRowInserter();
    }

    public void testExecute() throws Exception {
        PlatformTransactionManager ptm = createNiceMock(PlatformTransactionManager.class);
        JobContext jobContext = createMock(JobContext.class);
        jobContext.schedule("Import data: xyz");
        expectLastCall().andReturn("jobId");
        jobContext.start("jobId");
        jobContext.success("jobId");
        replay(ptm, jobContext);

        m_ri.setTransactionManager(ptm);
        m_ri.setJobContext(jobContext);
        DummyType dummy = new DummyType();
        assertEquals(0, m_ri.m_rows);
        m_ri.execute(dummy);
        assertEquals(1, m_ri.m_rows);

        verify(ptm, jobContext);
    }

    private static class DummyRowInserter extends RowInserter<DummyType> {
        public int m_rows;

        protected String dataToString(DummyType input) {
            return "xyz";
        }

        protected void insertRow(DummyType input) {
            m_rows++;
        }
    }

    private static class DummyType {
    }
}
