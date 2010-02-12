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
