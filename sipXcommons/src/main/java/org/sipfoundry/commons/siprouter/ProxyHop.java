/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.siprouter;

import java.util.Random;

import javax.sip.address.Hop;

public class ProxyHop implements Hop, Comparable<ProxyHop> {

    private int priority;
    private int weight;
    private String host;
    private int port;
    private String transport;
    private Random random;

    public ProxyHop(String host, int port, String transport) {
        this.host = host;
        this.port = port;
        this.transport = transport;
        this.random = new Random();
    }

    public String getHost() {
        return host;
    }

    public int getPort() {
        return port;
    }

    public String getTransport() {
        return transport;
    }

    public void setPriority(int priority) {
        this.priority = priority;
    }

    public void setWeight(int weight) {
        this.weight = weight;
    }

    /**
     * Does a compareTo operation of two hops. If the hop priorities
     * are the same, it uses the weights to make a prbabilistic determination.
     * 
     */
    public int compareTo(ProxyHop otherHop) {
        if (otherHop.priority > priority) {
            return +1;
        } else if (otherHop.priority < priority) {
            return -1;
        } else {
            double fraction = (double) otherHop.weight / (double) (otherHop.weight + this.weight);
            double guess = random.nextDouble();
            if (guess < fraction) {
                return -1;
            } else {
                return +1;
            }

        }
    }
    
    @Override
    public boolean equals(Object that) {
        if ( !(that instanceof ProxyHop)) {
            return false;
        } else {
            ProxyHop other = (ProxyHop) that;
            if (! this.getHost().equals(other.getHost()) || this.getPort() != other.getPort()) {
                return false;
            } else {
                return true;
            }
        }
    }

}
