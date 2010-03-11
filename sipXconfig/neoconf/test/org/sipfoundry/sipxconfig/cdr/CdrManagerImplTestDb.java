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

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.cdr.CdrSearch.Mode;
import org.springframework.context.ApplicationContext;

public class CdrManagerImplTestDb extends SipxDatabaseTestCase {

    public void testGetCdrs() {
        ApplicationContext app = TestHelper.getApplicationContext();
        CdrManager cdrManager = (CdrManager) app.getBean(CdrManager.CONTEXT_BEAN_NAME);
        List<Cdr> cdrs = cdrManager.getCdrs(null, null, new CdrSearch(), null);
        assertTrue(cdrs.size() > 0);
    }

    public void testGetCdrsCount() {
        ApplicationContext app = TestHelper.getApplicationContext();
        CdrManager cdrManager = (CdrManager) app.getBean(CdrManager.CONTEXT_BEAN_NAME);
        int size = cdrManager.getCdrCount(null, null, new CdrSearch(), null);
        assertTrue(size > 0);
    }

    public void testDumpCdrs() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        CdrManager cdrManager = (CdrManager) app.getBean(CdrManager.CONTEXT_BEAN_NAME);
        OutputStreamWriter writer = new OutputStreamWriter(System.err);
        cdrManager.dumpCdrs(writer, null, null, new CdrSearch(), null);
    }

    public void testGetCsv() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        CdrManager cdrManager = (CdrManager) app.getBean(CdrManager.CONTEXT_BEAN_NAME);
        Writer writer = new OutputStreamWriter(System.err);
        cdrManager.dumpCdrs(writer, null, null, new CdrSearch(), null);
        writer.flush();
    }

    public void testGetJson() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        CdrManager cdrManager = (CdrManager) app.getBean(CdrManager.CONTEXT_BEAN_NAME);
        Writer writer = new OutputStreamWriter(System.err);
        cdrManager.dumpCdrsJson(writer);
        writer.flush();
    }

    public void testGetCdrsSearchFrom() {
        ApplicationContext app = TestHelper.getApplicationContext();
        CdrManager cdrManager = (CdrManager) app.getBean(CdrManager.CONTEXT_BEAN_NAME);
        CdrSearch cdrSearch = new CdrSearch();
        cdrSearch.setMode(Mode.CALLER);
        cdrSearch.setTerm("200");
        List<Cdr> cdrs = cdrManager.getCdrs(null, null, cdrSearch, null);
        assertTrue(cdrs.size() == 3);
    }

    public void testGetCdrsSearchTo() {
        ApplicationContext app = TestHelper.getApplicationContext();
        CdrManager cdrManager = (CdrManager) app.getBean(CdrManager.CONTEXT_BEAN_NAME);
        CdrSearch cdrSearch = new CdrSearch();
        cdrSearch.setMode(Mode.CALLEE);
        cdrSearch.setTerm("201");
        List<Cdr> cdrs = cdrManager.getCdrs(null, null, cdrSearch, null);
        assertTrue(cdrs.size() == 6);
    }

    public void testGetCdrsSearchAny() {
        ApplicationContext app = TestHelper.getApplicationContext();
        CdrManager cdrManager = (CdrManager) app.getBean(CdrManager.CONTEXT_BEAN_NAME);
        CdrSearch cdrSearch = new CdrSearch();
        cdrSearch.setMode(Mode.ANY);
        cdrSearch.setTerm("200");
        List<Cdr> cdrs = cdrManager.getCdrs(null, null, cdrSearch, null);
        assertTrue(cdrs.size() == 5);
    }

}
