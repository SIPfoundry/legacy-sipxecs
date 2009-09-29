/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.io.Serializable;

/**
 * Ultimate parent for all objects. Represent the entire configuration for a particular company or
 * organization
 */
public class Organization extends BeanWithId implements Serializable, NamedObject {

    private static final long serialVersionUID = 1L;

    private String m_name;

    private String m_dnsDomain;

    // TODO : Enumerate stereotypes
    private int m_stereotype = 1;

    /**
     * @return Returns the dnsDomain.
     */
    public String getDnsDomain() {
        return m_dnsDomain;
    }
    /**
     * @param dnsDomain The dnsDomain to set.
     */
    public void setDnsDomain(String dnsDomain) {
        m_dnsDomain = dnsDomain;
    }

    /**
     * @return Returns the stereoType.
     */
    public int getStereotype() {
        return m_stereotype;
    }
    /**
     * @param stereotype The stereoType to set.
     */
    public void setStereotype(int stereotype) {
        m_stereotype = stereotype;
    }

    /**
     * @return Returns the name.
     */
    public String getName() {
        return m_name;
    }
    /**
     * @param name The name to set.
     */
    public void setName(String name) {
        m_name = name;
    }

}
