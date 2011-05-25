/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxivr;

import java.io.IOException;
import java.io.PrintWriter;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.log4j.Logger;

public class RestfulRequestTestServlet extends HttpServlet {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private static final long serialVersionUID = -2691329114223188283L;

    public void doDelete(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        LOG.debug("RRTServlet::doDelete");
    }

    public void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        LOG.debug("RRTServlet::doPost");
        PrintWriter pw = response.getWriter();
        pw.write("Next time, use mail, not POST!\n");
        pw.close();
    }

    public void doPut(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        LOG.debug("RRTServlet::doPut");
    }

    public void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        LOG.debug("RRTServlet::doGet");
        PrintWriter pw = response.getWriter();
        pw.write("How now, fuzzy brown puppy!\n");
        pw.close();
    }

}
