/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.configstore;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public abstract class PersistedClass {
    private String key;
    
    protected void setKey(String key) {
        this.key = key;
    }
    
    protected String getKey() {
        return key;
    }
    
}
