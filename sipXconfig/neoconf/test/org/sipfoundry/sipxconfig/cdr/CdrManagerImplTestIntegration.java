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
import java.util.List;

import org.sipfoundry.sipxconfig.cdr.CdrSearch.Mode;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class CdrManagerImplTestIntegration extends IntegrationTestCase {
    private CdrManager m_cdrManager;
    
    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testGetCdrs() {
        List<Cdr> cdrs = m_cdrManager.getCdrs(null, null, new CdrSearch(), null);
        assertTrue(cdrs.size() > 0);
    }

    public void testGetCdrsCount() {
        int size = m_cdrManager.getCdrCount(null, null, new CdrSearch(), null);
        assertTrue(size > 0);
    }

    public void testDumpCdrs() throws Exception {
        OutputStreamWriter writer = new OutputStreamWriter(System.err);
        m_cdrManager.dumpCdrs(writer, null, null, new CdrSearch(), null);
    }

    public void testGetCsv() throws Exception {
        Writer writer = new OutputStreamWriter(System.err);
        m_cdrManager.dumpCdrs(writer, null, null, new CdrSearch(), null);
        writer.flush();
    }

    public void testGetJson() throws Exception {
        Writer writer = new OutputStreamWriter(System.err);
        m_cdrManager.dumpCdrsJson(writer);
        writer.flush();
    }

    public void testGetCdrsSearchFrom() {
        CdrSearch cdrSearch = new CdrSearch();
        cdrSearch.setMode(Mode.CALLER);
        cdrSearch.setTerm("200");
        List<Cdr> cdrs = m_cdrManager.getCdrs(null, null, cdrSearch, null);
        assertTrue(cdrs.size() == 3);
    }

    public void testGetCdrsSearchTo() {
        CdrSearch cdrSearch = new CdrSearch();
        cdrSearch.setMode(Mode.CALLEE);
        cdrSearch.setTerm("201");
        List<Cdr> cdrs = m_cdrManager.getCdrs(null, null, cdrSearch, null);
        assertTrue(cdrs.size() == 6);
    }

    public void testGetCdrsSearchAny() {
        CdrSearch cdrSearch = new CdrSearch();
        cdrSearch.setMode(Mode.ANY);
        cdrSearch.setTerm("200");
        List<Cdr> cdrs = m_cdrManager.getCdrs(null, null, cdrSearch, null);
        assertTrue(cdrs.size() == 5);
    }

    public void setCdrManager(CdrManager cdrManager) {
        m_cdrManager = cdrManager;
    }

}
