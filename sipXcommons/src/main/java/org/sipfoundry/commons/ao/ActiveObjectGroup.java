/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.ao;

import org.apache.log4j.*;

import java.util.Iterator;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Service locator for Active Objects.
 * <p>
 *
 * @author Mardy Marshall
 *
 * @param <K>
 *            Class type of service location keys.
 */
public class ActiveObjectGroup<K> implements ActiveObjectGroupInterface, Iterable<Object> {
	private final Logger logger;
	@SuppressWarnings("unused")
    private final String groupName;
	private final ExecutorPool executorPool;
	private final TimerPool timerPool;

	/**
	 * Mapping between ActiveObjects and keys.
	 */
	private final ConcurrentHashMap<K, Object> activeObjectMap;

	public ActiveObjectGroup(String groupName, int contextPriority, int maxContexts, long contextIdleThreshold, Logger logger) {
	    this.logger = logger;
		executorPool = new ExecutorPool(groupName, contextPriority, maxContexts, contextIdleThreshold);
		timerPool = new TimerPool(groupName);
		this.groupName = groupName;

		// Instantiate the ActiveObject map.
		activeObjectMap = new ConcurrentHashMap<K, Object>();
	}

	public Object newActiveObject(ActiveObject object, K key) {
		Object proxyInstance =  ActiveObject.newActiveObject(object, executorPool, timerPool, logger);

		// Add the instance to the map.
		activeObjectMap.put(key, proxyInstance);
		object.setID(key);

		// Set the parent.
		object.setActiveObjectGroup(this);

		return proxyInstance;
	}

	/**
	 * Attempt to find the ActiveObject instance that has previously been mapped to a specific key.
	 *
	 * @param key
	 *            The key that has previously been mapped to a ActiveObject instance.
	 * @return The ActiveObject instance that the given key maps to. If no mapping exists, returns null.
	 */
	public synchronized Object findInstance(K key) {
		if (key != null) {
			return activeObjectMap.get(key);
		} else {
			return null;
		}
	}

	@SuppressWarnings("unchecked")
	public synchronized void deleteInstance(ActiveObject object) {
		K key = (K)object.getID();
		if (activeObjectMap.containsKey(key)) {
			activeObjectMap.remove(key);
		}
	}

	public synchronized int size() {
	    return activeObjectMap.size();
	}

	public synchronized boolean isEmpty() {
	    return activeObjectMap.isEmpty();
	}

	public Iterator<Object> iterator() {
	    return new ActiveObjectGroupIterator();
	}

	public class ActiveObjectGroupIterator implements Iterator<Object> {
	    private final Object[] collection;
	    private int index;

	    public ActiveObjectGroupIterator() {
	        collection = activeObjectMap.entrySet().toArray();
	        index = 0;
	    }

        public boolean hasNext() {
            if (collection.length > index) {
                return true;
            } else {
                return false;
            }
        }

        public Object next() {
            if (collection.length > index) {
                return collection[index++];
            } else {
                return null;
            }
        }

        public void remove() {
            throw new UnsupportedOperationException();
        }
	}
}
