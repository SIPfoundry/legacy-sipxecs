/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfengine;

import java.io.IOException;
import java.io.Writer;
import java.util.HashMap;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class CfDataFile implements ConfigurationFile {
    private String m_name;
    private Map<String, Boolean> m_classes = new HashMap<String, Boolean>();
    private Map<String, String> m_variables = new HashMap<String, String>();

    @Override
    public void write(Writer writer, Location location) throws IOException {
        for (Map.Entry<String, String> entry : m_variables.entrySet()) {
            writer.write('=');
            writer.write(entry.getKey());
            writer.write('=');
            writer.write(entry.getValue());
            writer.write('\n');
        }
        for (Map.Entry<String, Boolean> entry : m_classes.entrySet()) {
            writer.write(entry.getValue() ? '+' : '-');
            writer.write(entry.getKey());
            writer.write('\n');
        }
    }

    @Override
    public String getName() {
        // TODO Auto-generated method stub
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    @Override
    public String getPath() {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public boolean isReplicable(Location location) {
        return false;
    }

    @Override
    public boolean isLocationDependent() {
        return true;
    }

    @Override
    public boolean isRestartRequired() {
        return false;
    }

    public Map<String, Boolean> getClasses() {
        return m_classes;
    }

    public void setClasses(Map<String, Boolean> classes) {
        m_classes = classes;
    }

    public Map<String, String> getVariables() {
        return m_variables;
    }

    public void setVariables(Map<String, String> variables) {
        m_variables = variables;
    }
}
