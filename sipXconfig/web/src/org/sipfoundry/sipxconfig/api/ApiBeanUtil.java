/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.api;

import java.lang.reflect.Array;
import java.lang.reflect.InvocationTargetException;
import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.beanutils.PropertyUtils;

public final class ApiBeanUtil {

    private static final String INDEX_PROP = "position";

    private ApiBeanUtil() {
    }

    public static void toApiObject(ApiBeanBuilder builder, Object apiObject, Object myObject) {
        Set properties = getProperties(apiObject);
        properties.remove(INDEX_PROP);
        builder.toApiObject(apiObject, myObject, properties);
    }

    public static void toMyObject(ApiBeanBuilder builder, Object myObject, Object apiObject) {
        Set properties = getProperties(apiObject);
        properties.remove(INDEX_PROP);
        builder.toMyObject(myObject, apiObject, properties);
    }

    public static void setProperties(Object object, Property[] properties) {
        for (int i = 0; i < properties.length; i++) {
            setProperty(object, properties[i].getProperty(), properties[i].getValue());
        }
    }

    public static Set getSpecifiedProperties(Property[] properties) {
        Set props = new HashSet(properties.length);
        for (int i = 0; i < properties.length; i++) {
            props.add(properties[i].getProperty());
        }
        return props;
    }

    /**
     * @return null if not found
     */
    public static Property findProperty(Property[] properties, String name) {
        for (Property property : properties) {
            if (property.getProperty().equals(name)) {
                return property;
            }
        }
        return null;
    }

    public static Object[] newArray(Class elementClass, int size) {
        Object[] to = (Object[]) Array.newInstance(elementClass, size);
        for (int i = 0; i < size; i++) {
            to[i] = newInstance(elementClass);
        }

        return to;
    }

    /**
     * Convert an array of server objects to an array of equivalent SOAP API objects (instances of
     * apiClass), using the supplied bean builder. Return the array of API objects.
     */
    public static Object[] toApiArray(ApiBeanBuilder builder, Object[] myObjects, Class apiClass) {
        int numObjects = myObjects != null ? myObjects.length : 0;
        Object[] apiArray = ApiBeanUtil.newArray(apiClass, numObjects);
        if (numObjects == 0) {
            return apiArray;
        }
        Set properties = ApiBeanUtil.getProperties(apiArray[0]);
        boolean updatePosition = properties.remove(INDEX_PROP);
        for (int i = 0; i < numObjects; i++) {
            builder.toApiObject(apiArray[i], myObjects[i], properties);
            if (updatePosition) {
                setProperty(apiArray[0], INDEX_PROP, i);
            }
        }
        return apiArray;
    }

    public static Object[] toApiArray(ApiBeanBuilder builder, Collection myObjects, Class apiClass) {
        if (myObjects == null) {
            return new Object[0];
        }
        int numObjects = myObjects.size();
        if (numObjects == 0) {
            return new Object[0];
        }
        Object[] myArray = newArray(myObjects.iterator().next().getClass(), numObjects);
        myArray = myObjects.toArray(myArray);
        return toApiArray(builder, myArray, apiClass);
    }

    /**
     * Convert an array of SOAP API objects to an array of equivalent server objects (instances of
     * myClass), using the supplied bean builder. Return the array of server objects.
     */
    public static Object[] toMyArray(ApiBeanBuilder builder, Object[] apiObjects, Class myClass) {
        int numObjects = apiObjects != null ? apiObjects.length : 0;
        Object[] myArray = newArray(myClass, numObjects);
        if (numObjects == 0) {
            return myArray;
        }
        for (int i = 0; i < apiObjects.length; i++) {
            Object myObj = newInstance(myClass);
            toMyObject(builder, myObj, apiObjects[i]);
            myArray[i] = myObj;
        }
        return myArray;
    }

    public static Object[] toMyArray(ApiBeanBuilder builder, Collection apiObjects, Class myClass) {
        if (apiObjects == null) {
            return new Object[0];
        }
        int numObjects = apiObjects.size();
        if (numObjects == 0) {
            return new Object[0];
        }
        Object[] apiArray = (Object[]) Array.newInstance(apiObjects.iterator().next().getClass(), numObjects);
        apiArray = apiObjects.toArray(apiArray);
        return toMyArray(builder, apiArray, myClass);
    }

    public static Set getProperties(Object o) {
        try {
            Map description = BeanUtils.describe(o);
            description.remove("class");
            return description.keySet();
        } catch (IllegalAccessException e) {
            throw new UnexpectedException(e);
        } catch (InvocationTargetException e) {
            throw new UnexpectedException(e);
        } catch (NoSuchMethodException e) {
            throw new UnexpectedException(e);
        }
    }

    public static void copyProperties(Object to, Object from, Set<String> properties, Set ignoreList) {
        for (String name : properties) {
            if (ignoreList != null && ignoreList.contains(name)) {
                continue;
            }
            try {
                Object value = PropertyUtils.getSimpleProperty(from, name);
                BeanUtils.copyProperty(to, name, value);
            } catch (IllegalAccessException e) {
                throw new PropertyException(name, e);
            } catch (InvocationTargetException e) {
                throw new PropertyException(name, e);
            } catch (NoSuchMethodException e) {
                throw new PropertyException(name, e);
            }
        }
    }

    public static void setProperty(Object o, String property, Object value) {
        try {
            BeanUtils.copyProperty(o, property, value);
        } catch (IllegalAccessException e) {
            throw new PropertyException(property, e);
        } catch (InvocationTargetException e) {
            throw new PropertyException(property, e);
        }
    }

    /** Like Class.newInstance, but convert checked exceptions to runtime exceptions */
    public static Object newInstance(Class klass) {
        try {
            return klass.newInstance();
        } catch (InstantiationException e) {
            throw new UnexpectedException(e);
        } catch (IllegalAccessException e) {
            throw new UnexpectedException(e);
        }
    }

    private static class UnexpectedException extends RuntimeException {
        public UnexpectedException(Exception e) {
            super("Unexpected bean error", e);
        }
    }

    private static class PropertyException extends RuntimeException {
        public PropertyException(String property, Exception e) {
            super("Error accessing property " + property, e);
        }
    }
}
