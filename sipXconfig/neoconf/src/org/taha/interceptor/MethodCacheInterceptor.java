/*
 * Copyright 2002-2004 the original author or authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.taha.interceptor;

import java.io.Serializable;

import net.sf.ehcache.Cache;
import net.sf.ehcache.Element;

import org.aopalliance.intercept.MethodInterceptor;
import org.aopalliance.intercept.MethodInvocation;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.springframework.beans.factory.InitializingBean;
import org.springframework.util.Assert;

/**
 * @author <a href="mailto:irbouh@gmail.com">Omar Irbouh</a>
 * @since 2004.10.07
 */
public class MethodCacheInterceptor implements MethodInterceptor, InitializingBean {
    private static final Log logger = LogFactory.getLog(MethodCacheInterceptor.class);

    private Cache cache;

    /**
     * sets cache name to be used
     */
    public void setCache(Cache cache_) {
        this.cache = cache_;
    }

    /**
     * Checks if required attributes are provided.
     */
    public void afterPropertiesSet() throws Exception {
        Assert.notNull(cache, "A cache is required. Use setCache(Cache) to provide one.");
    }

    /**
     * main method caches method result if method is configured for caching method results must be
     * serializable
     */
    public Object invoke(MethodInvocation invocation) throws Throwable {
        String targetName = invocation.getThis().getClass().getName();
        String methodName = invocation.getMethod().getName();
        Object[] arguments = invocation.getArguments();
        Object result;

        logger.debug("looking for method result in cache");
        String cacheKey = getCacheKey(targetName, methodName, arguments);
        Element element = cache.get(cacheKey);
        if (element == null) {
            // call target/sub-interceptor
            logger.debug("calling intercepted method");
            result = invocation.proceed();

            // cache method result
            logger.debug("caching result");
            element = new Element(cacheKey, (Serializable) result);
            cache.put(element);
        }
        return element.getValue();
    }

    /**
     * creates cache key: targetName.methodName.argument0.argument1...
     */
    private String getCacheKey(String targetName, String methodName, Object[] arguments) {
        StringBuffer sb = new StringBuffer();
        sb.append(targetName).append(".").append(methodName);
        if ((arguments != null) && (arguments.length != 0)) {
            for (int i = 0; i < arguments.length; i++) {
                sb.append(".").append(getCacheKey(arguments[i]));
            }
        }

        return sb.toString();
    }

    public static String getCacheKey(Object o) {
        if (o == null) {
            return "";
        }

        // TODO Collections

        if (Object[].class.isAssignableFrom(o.getClass())) {
            Object[] a = (Object[]) o;
            StringBuffer sb = new StringBuffer();
            sb.append('[');
            for (int i = 0; i < a.length; i++) {
                sb.append(getCacheKey(a[i])).append(',');
            }
            sb.append(']');
            return sb.toString();
        }

        return o.toString();
    }
}
