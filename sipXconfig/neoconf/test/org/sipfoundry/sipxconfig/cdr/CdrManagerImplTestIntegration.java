/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cdr;

import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.Date;
import java.util.List;
import java.util.TimeZone;

import org.easymock.EasyMock;
import org.joda.time.DateTime;
import org.joda.time.DateTimeZone;
import org.sipfoundry.sipxconfig.cdr.CdrSearch.Mode;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.time.NtpManager;

public class CdrManagerImplTestIntegration extends IntegrationTestCase {
    private CdrManagerImpl m_cdrManagerImpl;
    
    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        NtpManager ntpManager = EasyMock.createMock(NtpManager.class);
        ntpManager.getSystemTimezone();
        EasyMock.expectLastCall().andReturn(TimeZone.getTimeZone("GMT+1").getID());
        EasyMock.replay(ntpManager);
        m_cdrManagerImpl.setNtpManager(ntpManager);
        clear();
    }

    public void testGetCdrs() {
        List<Cdr> cdrs = m_cdrManagerImpl.getCdrs(null, null, new CdrSearch(), null);
        assertTrue(cdrs.size() > 0);
    }

    public void testGetCdrsCount() {
        int size = m_cdrManagerImpl.getCdrCount(null, null, new CdrSearch(), null);
        assertTrue(size > 0);
    }

    public void testDumpCdrs() throws Exception {
        OutputStreamWriter writer = new OutputStreamWriter(System.err);
        m_cdrManagerImpl.dumpCdrs(writer, null, null, new CdrSearch(), null);
    }

    public void testGetCsv() throws Exception {
        Writer writer = new OutputStreamWriter(System.err);
        m_cdrManagerImpl.dumpCdrs(writer, null, null, new CdrSearch(), null);
        writer.flush();
    }

    public void testGetJson() throws Exception {
        Writer writer = new OutputStreamWriter(System.err);
        m_cdrManagerImpl.dumpCdrsJson(writer);
        writer.flush();
    }

    public void testGetCdrsSearchFrom() {
        CdrSearch cdrSearch = new CdrSearch();
        cdrSearch.setMode(Mode.CALLER);
        cdrSearch.setTerm(new String[] {
            "200"
        });
        List<Cdr> cdrs = m_cdrManagerImpl.getCdrs(null, null, cdrSearch, null);
        assertTrue(cdrs.size() == 3);
    }

    public void testGetCdrsSearchTo() {
        CdrSearch cdrSearch = new CdrSearch();
        cdrSearch.setMode(Mode.CALLEE);
        cdrSearch.setTerm(new String[] {
            "201"
        });
        List<Cdr> cdrs = m_cdrManagerImpl.getCdrs(null, null, cdrSearch, null);
        assertTrue(cdrs.size() == 6);
    }

    public void testGetCdrsSearchAny() {
        CdrSearch cdrSearch = new CdrSearch();
        cdrSearch.setMode(Mode.ANY);
        cdrSearch.setTerm(new String[] {
            "200"
        });
        List<Cdr> cdrs = m_cdrManagerImpl.getCdrs(null, null, cdrSearch, null);
        assertTrue(cdrs.size() == 5);
    }

    /*timestamp should display in system timezone
    defined in the tests as UTC+2
    */
    public void testGetCdrTimestamp(){
        DateTime from = new DateTime(2010, 1, 16, 0, 0, 0, 0);
        DateTime to = new DateTime(2010, 1, 17, 0, 0, 0, 0);
        List<Cdr> cdrs = m_cdrManagerImpl.getCdrs(from.toDate(), to.toDate(), null);
        assertTrue(cdrs.size() == 1);
        Cdr cdr = cdrs.get(0);
        DateTime start = new DateTime(cdr.getStartTime());
        assertEquals(10, start.getHourOfDay());
    }
    
    public void setCdrManagerImpl(CdrManagerImpl cdrManager) {
        m_cdrManagerImpl = cdrManager;
    }

}
