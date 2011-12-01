/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxivr.rest;

public class RestApiBean {
    private String m_name;
    private String m_pathSpec;
    private String m_servletClass;

    public RestApiBean(String name, String pathSpec, String servletClass) {
        m_name = name;
        m_pathSpec = pathSpec;
        m_servletClass = servletClass;
    }

    public String getName() {
        return m_name;
    }
    public String getPathSpec() {
        return m_pathSpec;
    }
    public String getServletClass() {
        return m_servletClass;
    }
    
}