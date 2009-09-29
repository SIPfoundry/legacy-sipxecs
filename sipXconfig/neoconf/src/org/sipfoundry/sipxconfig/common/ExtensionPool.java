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


public class ExtensionPool extends BeanWithId implements NamedObject {

    private boolean m_enabled;
    private String m_name;
    private Integer m_firstExtension;
    private Integer m_lastExtension;
    private Integer m_nextExtension;

    public ExtensionPool() {
        super();
    }

    public ExtensionPool(boolean enabled, String name,
                         Integer firstExtension, Integer lastExtension, Integer nextExtension) {
        super();
        m_enabled = enabled;
        m_name = name;
        m_firstExtension = firstExtension;
        m_lastExtension = lastExtension;
        m_nextExtension = nextExtension;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    /** Return the first extension in the range for this pool */
    public Integer getFirstExtension() {
        return m_firstExtension;
    }

    /** Set the first extension in the range for this pool */
    public void setFirstExtension(Integer firstExtension) {
        m_firstExtension = firstExtension;
    }

    /** Set the first extension in the range for this pool */
    public void setFirstExtension(int firstExtension) {
        setFirstExtension(new Integer(firstExtension));
    }

    /** Return the last extension in the range for this pool */
    public Integer getLastExtension() {
        return m_lastExtension;
    }

    /** Set the last extension in the range for this pool */
    public void setLastExtension(Integer lastExtension) {
        m_lastExtension = lastExtension;
    }

    /** Set the last extension in the range for this pool */
    public void setLastExtension(int lastExtension) {
        setLastExtension(new Integer(lastExtension));
    }

    /** Return the next extension that we will assign automatically, if it is free */
    public Integer getNextExtension() {
        return m_nextExtension;
    }

    /** Set the next extension that we will assign automatically, if it is free */
    public void setNextExtension(Integer nextExtension) {
        m_nextExtension = nextExtension;
    }

    /** Set the next extension in the range for this pool */
    public void setNextExtension(int nextExtension) {
        setNextExtension(new Integer(nextExtension));
    }

}
