/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxivr.eslrequest;

import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;

import org.springframework.util.Assert;

public class EslRequestScopeAttributes {

    protected final Map<String, Object> hBeans = new HashMap<String, Object>();
    protected final Map<String, Runnable> hRequestDestructionCallbacks = new LinkedHashMap<String, Runnable>();

    protected final Map<String, Object> getBeanMap() {
        return hBeans;
    }

    /**
     * Register the given callback as to be executed after request completion.
     * 
     * @param name The name of the bean.
     * @param callback The callback of the bean to be executed for destruction.
     */
    protected final void registerRequestDestructionCallback(String name, Runnable callback) {
        Assert.notNull(name, "Name must not be null");
        Assert.notNull(callback, "Callback must not be null");

        hRequestDestructionCallbacks.put(name, callback);
    }

    /**
     * Clears beans and processes all bean destruction callbacks.
     */
    public final void clear() {
        processDestructionCallbacks();
        hBeans.clear();
    }

    /**
     * Processes all bean destruction callbacks.
     */
    private final void processDestructionCallbacks() {
        for (String name : hRequestDestructionCallbacks.keySet()) {
            Runnable callback = hRequestDestructionCallbacks.get(name);
            callback.run();
        }
        hRequestDestructionCallbacks.clear();
    }

}
