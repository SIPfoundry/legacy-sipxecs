/**
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.ao;

import org.apache.log4j.*;

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
public class ActiveObjectGroup<K> {
	private Logger logger;
	private final String groupName;
	private final ExecutorPool executorPool;
	private final TimerPool timerPool;
	
	/**
	 * Mapping between ActiveObjects and keys.
	 */
	private final ConcurrentHashMap<K, Object> activeObjectMap;

	public ActiveObjectGroup(String groupName, int contextPriority, int maxContexts, long contextIdleThreshold) {
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

}
