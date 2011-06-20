/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.voicemail;

import java.util.HashMap;
import java.util.Vector;

public class Distributions {
    
    private HashMap<String, Vector<String>>m_Lists = new HashMap<String, Vector<String>>();
    private StringBuilder m_indices = new StringBuilder(); // A string with each index concatenated.
    
    public void addList(String index, Vector<String> dests) {
        m_Lists.put(index, dests);
        m_indices.append(index);
    }

    public Vector<String> getList(String index) {
        return m_Lists.get(index);
    }
    
    /**
     * Return a string with all the indices that are provisioned.
     * Used as a list of valid digits to dial
     * @return
     */
    public String getIndices() {
        return m_indices.toString();
    }
}
