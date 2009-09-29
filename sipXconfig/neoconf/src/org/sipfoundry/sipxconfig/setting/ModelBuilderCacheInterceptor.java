/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.io.File;
import java.io.Serializable;

import net.sf.ehcache.Cache;
import net.sf.ehcache.Element;

import org.aopalliance.intercept.MethodInterceptor;
import org.aopalliance.intercept.MethodInvocation;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class ModelBuilderCacheInterceptor implements MethodInterceptor {
    private static final Log LOG = LogFactory.getLog(ModelBuilderCacheInterceptor.class);

    private Cache m_cache;

    public void setCache(Cache cache) {
        m_cache = cache;
    }

    /**
     * Main method caches method result if method is configured. For caching method results must
     * be serializable
     */
    public Object invoke(MethodInvocation invocation) throws Throwable {
        Object[] arguments = invocation.getArguments();

        LOG.trace("looking for method result in cache");
        String cacheKey = getCacheKey(arguments);
        Element element = m_cache.get(cacheKey);
        if (element == null) {
            LOG.trace("calling intercepted method");
            Object result = invocation.proceed();

            LOG.debug("caching result");
            element = new Element(cacheKey, (Serializable) result);
            m_cache.put(element);
        }
        return element.getValue();
    }

    /**
     * Creates cache key - this implementation is fine tune to create key based on the name the
     * file passed as the first (and only) argument to the method
     */
    protected String getCacheKey(Object[] arguments) {
        File file = (File) arguments[0];
        return file.getPath();
    }
}
