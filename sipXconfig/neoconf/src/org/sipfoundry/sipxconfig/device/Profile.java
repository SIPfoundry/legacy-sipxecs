/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.device;

/**
 * Encapsulates the details of creating a profile/configuration file. Default implementation
 * creates context and filter by consulting device. If device generates additional profiles it
 * should return define sublclasses of this class and overwrite createContext and createFilter
 * methods.
 */
public class Profile {
    private String m_name;
    private String m_mimeType;

    public Profile(Device device) {
        this(device.getProfileFilename());
    }

    public Profile(String name) {
        this(name, "text/plain");
    }

    public Profile(String name, String mimeType) {
        m_name = name;
        m_mimeType = mimeType;
    }

    /**
     * Usually name of the file related to generating
     */
    public String getName() {
        return m_name;
    }

    public String getMimeType() {
        return m_mimeType;

    }

    public final void generate(Device device, ProfileLocation location) {
        ProfileGenerator generator = device.getProfileGenerator();
        ProfileFilter profileFilter = createFilter(device);
        ProfileContext context = createContext(device);
        if (context != null) {
            generator.generate(location, context, profileFilter, getName());
        }
    }

    protected ProfileContext createContext(Device device) {
        return device.createContext();
    }

    protected ProfileFilter createFilter(Device device) {
        return device.getProfileFilter();
    }
}
