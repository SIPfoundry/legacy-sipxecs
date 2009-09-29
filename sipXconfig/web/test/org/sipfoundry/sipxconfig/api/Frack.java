/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.api;

/** sample test bean */
public class Frack {
    private String m_name;
    private String m_abode;
    public Frack() {
    }
    public Frack(String name, String abode) {
        setName(name);
        setAbode(abode);
    }
    public String getAbode() {
        return m_abode;
    }
    public void setAbode(String abode) {
        m_abode = abode;
    }
    public String getName() {
        return m_name;
    }
    public void setName(String name) {
        m_name = name;
    }
}
