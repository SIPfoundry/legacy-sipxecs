/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrelay;

/**
 *  little utility is because java 5 does not support concurrent sets.
 * 
 */
import java.nio.channels.DatagramChannel;
import java.util.Collection;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

import org.apache.log4j.Logger;


class ConcurrentSet implements Set<Sym> {
	private static Logger logger = Logger.getLogger(ConcurrentSet.class);

    Map<String, Sym> map = new ConcurrentHashMap<String, Sym>();
    private Bridge bridge;
    private static Map<DatagramChannel,Bridge> bridgeMap = 
        new ConcurrentHashMap<DatagramChannel,Bridge>();
    
    public ConcurrentSet(Bridge bridge) {
        this.bridge = bridge;
    }

    public boolean add(Sym element) {
    	if ( logger.isDebugEnabled()) {
    		logger.debug("addSym " + element.getId());
    	}
        map.put(element.getId(), element);
        if ( element.getBridge() != null  && element.getBridge() != this.bridge) {
            element.getBridge().removeSym(element);
        }
        element.setBridge(this.bridge);
        DatagramChannel channel = element.getReceiver().getDatagramChannel();
        if (! bridgeMap.containsKey(channel)) {  
            DataShuffler.initializeSelectors();
           
        }
        bridgeMap.put(channel, bridge);
        return true;
        
    }
    
    public static Bridge getBridge(DatagramChannel channel) {
        return bridgeMap.get(channel);
    }
    
    public static void removeChannel(DatagramChannel channel) {
        bridgeMap.remove(channel);
        DataShuffler.initializeSelectors();
    }

    public boolean addAll(Collection<? extends Sym> collection) {
        for (Sym t : collection) {
            this.add(t);
        }
        return true;
    }

    public void clear() {
        map.clear();
    }

    public boolean contains(Object obj) {
        return map.containsKey(((Sym)obj).getId());

    }

    public boolean containsAll(Collection<?> collection) {
        Collection<Sym> set = map.values();
        return set.containsAll(collection);
    }

    public boolean isEmpty() {
        return map.isEmpty();
    }

    public Iterator<Sym> iterator() {

        return map.values().iterator();
    }

    public boolean remove(Object obj) {
        this.map.remove(((Sym)obj).getId());
        ((Sym)obj).setBridge(null);
        Sym element = ( Sym ) obj;
        DatagramChannel channel = element.getReceiver().getDatagramChannel();
        bridgeMap.remove(channel);
        DataShuffler.initializeSelectors();
       
        
        return true;
    }

    public boolean removeAll(Collection<?> collection) {
        for (Object obj : collection) {
            this.map.remove(obj);
        }
        return true;
    }

    public boolean retainAll(Collection<?> collection) {
        throw new UnsupportedOperationException("Unsupported");
    }

    public int size() {
        return map.size();
    }

    public Object[] toArray() {

        return this.map.keySet().toArray();
    }

    public  Sym[] toArray(Sym[] array) {
        return this.map.keySet().toArray(array);
    }

    public <T> T[] toArray(T[] array) {
        return map.values().toArray(array);
    }

    public static Collection<Bridge> getBridges() {
        return bridgeMap.values();
    }
    
   

}
