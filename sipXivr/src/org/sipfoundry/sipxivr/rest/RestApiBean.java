/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
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