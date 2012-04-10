/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
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
