/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.userdb;

import java.util.HashMap;

public class Distributions {
    
    private HashMap<String, String[]>m_Lists = new HashMap<String, String[]>();
    private StringBuilder m_indices = new StringBuilder(); // A string with each index concatenated.
    
    public void addList(String index, String[] dests) {
        m_Lists.put(index, dests);
        m_indices.append(index);
    }

    public String[] getList(String index) {
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
