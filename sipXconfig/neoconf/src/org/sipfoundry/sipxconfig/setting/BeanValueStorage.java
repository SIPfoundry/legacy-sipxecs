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

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.lang.StringUtils;

/**
 * Treat any bean as a value storage, mapping properties to setting paths.
 *
 * Methods should return external representations, e.g. booleans instead of "0" or "1" for
 * example.
 *
 * Example:
 *
 * public static class MyDefaults {
 *
 * @SettingEntry(path = "hat/color") public String getHatColor() { return "red"; } }
 *
 * public void initialize() { addDefaultBeanValueStorage(new MyDefaults()); }
 */
public class BeanValueStorage implements SettingValueHandler {
    private Object m_bean;
    private Map<String, Method> m_methods = new HashMap<String, Method>();

    public BeanValueStorage(Object bean) {
        m_bean = bean;
        for (Method m : m_bean.getClass().getMethods()) {
            if (m.isAnnotationPresent(SettingEntry.class)) {
                SettingEntry entry = m.getAnnotation(SettingEntry.class);
                if (!StringUtils.isBlank(entry.path())) {
                    m_methods.put(entry.path(), m);
                } else {
                    for (String s : entry.paths()) {
                        m_methods.put(s, m);
                    }
                }
            }
        }
    }

    public SettingValue getSettingValue(Setting setting) {
        String path = setting.getPath();
        Method m = m_methods.get(path);
        if (m == null) {
            return null;
        }
        try {
            // should be getter (although not strictly nec), with no args
            Object result = m.invoke(m_bean);
            if (result == null) {
                // obstain on null.
                // NOTE: There is not way to return NULL as a setting value. If this
                // is nec., I would suggest adding field to SettingEntry annotation
                // called "obstainOn" and check if that matches this result.
                return null;
            }

            String svalue = setting.getType().convertToStringValue(result);
            return new SettingValueImpl(svalue);
        } catch (IllegalArgumentException e) {
            throw new RuntimeException(e);
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        } catch (InvocationTargetException e) {
            Throwable cause = e.getCause();
            if (cause instanceof NoValueException) {
                // this is OK - it means bean does not have any value to provide
                return null;
            }
            // unwrap RuntimeExceptions
            if (cause instanceof RuntimeException) {
                throw (RuntimeException) cause;
            }
            throw new RuntimeException(cause);
        }
    }

    /**
     * Exception to be thrown when Bean methods does not provide any setting value at the moment.
     * It's useful when you want to influence setting values only depending on current system
     * configuration.
     */
    public static class NoValueException extends RuntimeException {
    }

}
