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

import java.util.Map;

import org.springframework.beans.factory.ObjectFactory;
import org.springframework.beans.factory.config.Scope;

public class EslRequestScope implements Scope {

    @Override
    public Object get(String name, ObjectFactory< ? > factory) {
        Object result = null;
        Map<String, Object> hBeans = EslRequestScopeContextHolder.currentEslRequestScopeAttributes().getBeanMap();
        if (!hBeans.containsKey(name)) {
            result = factory.getObject();
            hBeans.put(name, result);
        } else {
            result = hBeans.get(name);
        }
        return result;
    }

    @Override
    public String getConversationId() {
        return Thread.currentThread().getName();
    }

    @Override
    public void registerDestructionCallback(String name, Runnable callback) {
        EslRequestScopeContextHolder.currentEslRequestScopeAttributes().registerRequestDestructionCallback(name,
                callback);
    }

    @Override
    public Object remove(String name) {
        Object result = null;
        Map<String, Object> hBeans = EslRequestScopeContextHolder.currentEslRequestScopeAttributes().getBeanMap();
        if (hBeans.containsKey(name)) {
            result = hBeans.get(name);
            hBeans.remove(name);
        }
        return result;
    }

    @Override
    public Object resolveContextualObject(String arg0) {
        // TODO Auto-generated method stub
        return null;
    }

}
