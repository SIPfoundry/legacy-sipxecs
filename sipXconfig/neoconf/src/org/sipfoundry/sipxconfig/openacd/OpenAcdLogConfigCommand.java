/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.openacd;

import java.util.LinkedList;
import java.util.List;

public class OpenAcdLogConfigCommand extends OpenAcdConfigObject {

    private final String m_logLevel;
    private final String m_logDir;

    public OpenAcdLogConfigCommand(String level, String logDir) {
        m_logLevel = level;
        m_logDir = logDir;
    }

    public String getLogLevel() {
        return m_logLevel;
    }

    public String getLogDir() {
        return m_logDir + "/openacd/";
    }

    @Override
    public List<String> getProperties() {
        List<String> props = new LinkedList<String>();
        props.add("logLevel");
        props.add("logDir");
        return props;
    }

    @Override
    public String getType() {
        return "log_configuration";
    }
}
