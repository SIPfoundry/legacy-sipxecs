/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrelay;

import java.util.Comparator;
import java.util.Iterator;
import java.util.PriorityQueue;
import java.util.TreeMap;

import org.apache.log4j.Logger;


/**
 * Resource manager for the ports that are managed by sipxbridge. Allocates
 * ports in the order of best fit. It uses a pair of tree maps and a priority 
 * queue to manage ranges of ports.
 * 
 * @author mranga
 * 
 */
public class PortRangeManager {
    
    private static Logger logger  = Logger.getLogger(PortRangeManager.class.getPackage().getName());

    TreeMap<Integer, PortRange> portRangeLowboundMap;

    TreeMap<Integer, PortRange> portRangeHighboundMap;

    PriorityQueue<PortRange> portRangeQueue;

    class PortRangeComparator implements Comparator<PortRange> {

        public int compare(PortRange portRange1, PortRange portRange2) {

            return portRange1.range() == portRange2.range() ? 0 : (portRange1
                    .range() < portRange2.range() ? +1 : -1);
        }

    }

    public PortRangeManager(int lowbound, int highbound) {
    	logger.debug("PortRangeManger: lowbound = " + lowbound + " highbound = " + highbound);
        this.portRangeLowboundMap = new TreeMap<Integer, PortRange>();
        this.portRangeHighboundMap = new TreeMap<Integer, PortRange>();
        this.portRangeQueue = new PriorityQueue<PortRange>(100,
                new PortRangeComparator());
        PortRange portRange = new PortRange(lowbound, highbound);
        this.insert(portRange);

    }

    private void remove(PortRange candidate) {
        this.portRangeLowboundMap.remove(candidate.getLowerBound());
        this.portRangeHighboundMap.remove(candidate.getHigherBound());
        this.portRangeQueue.remove(candidate);
    }

    private void insert(PortRange portRange) {
        int highbound = portRange.getHigherBound();
        PortRange rightNeighbor = this.portRangeLowboundMap.get(highbound);
        PortRange leftNeighbor = this.portRangeHighboundMap.get(portRange
                .getLowerBound());

        if (rightNeighbor != null) {
            portRange.setHigherBound(rightNeighbor.getHigherBound());
            this.remove(rightNeighbor);
        }

        if (leftNeighbor != null) {
            portRange.setLowerBound(leftNeighbor.getLowerBound());
            this.remove(leftNeighbor);
        }

        this.portRangeHighboundMap.put(portRange.getHigherBound(), portRange);
        this.portRangeLowboundMap.put(portRange.getLowerBound(), portRange);
        this.portRangeQueue.add(portRange);

    }

    public synchronized PortRange allocate(int size, Parity parity) {
        Iterator<PortRange> it = portRangeQueue.iterator();
        int res;
        if (parity.equals(Parity.EVEN))
            res = 0;
        else
            res = 1;
        
        logger.debug("PortRangeManger: allocate size = " + size + " Parity = " + parity);
        while (it.hasNext()) {
            PortRange candidate = it.next();

            if (candidate.range() >= size
                    && candidate.getLowerBound() % 2 == res) {

                this.remove(candidate);
                int remainder = candidate.range() - size;
                if (remainder > 0) {
                    PortRange portRange = new PortRange(candidate
                            .getLowerBound()
                            + size, candidate.getHigherBound());
                    this.insert(portRange);
                }
                candidate.setHigherBound(candidate.getLowerBound() + size);
                return candidate;
            } else if (candidate.range() > size) {
                PortRange fragment = new PortRange(candidate.getLowerBound(), 
                        candidate.getLowerBound() + 1);
                this.remove(candidate);
                candidate.setLowerBound( candidate.getLowerBound() + 1 );
                this.insert(fragment);
                
                int remainder = candidate.range() - size;
                if (remainder > 0) {
                    PortRange portRange = new PortRange(candidate
                            .getLowerBound()
                            + size, candidate.getHigherBound());
                    this.insert(portRange);
                }
                candidate.setHigherBound(candidate.getLowerBound() + size);
                return candidate;
            } else  {
            	logger.debug("Allocaton failed available port ranges = " + this.toString());
                return null;
            }

        }
        return null;

    }

    /**
     * Free a port range and update our accounting structures.
     * 
     * @param portRange
     */
    public synchronized void free(PortRange portRange) {
        
        logger.debug("Freeing " + portRange.getLowerBound() + " " + portRange.getHigherBound());

        this.insert(portRange);

    }

    int getRangeCount() {
        return this.portRangeHighboundMap.size();
    }

    boolean checkIntegrity() {
        return this.portRangeHighboundMap.size() == this.portRangeLowboundMap
                .size()
                && this.portRangeQueue.size() == this.portRangeHighboundMap
                        .size();
    }

    @Override
    public String toString() {
        StringBuffer retval = new StringBuffer();
        for (PortRange portRange : this.portRangeQueue) {
            retval.append(portRange);
        }
        return retval.toString();
    }

}
