/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrelay;

import java.util.Map;

import org.apache.commons.beanutils.PropertyUtils;

public class PortRange {

    private int lowerBound;

    private int higherBound;
    
    public PortRange( int lowbound, int highbound) {
        this.lowerBound = lowbound;
        this.higherBound = highbound;
    }

    /**
     * @param lowerBound
     *            the lowerBound to set
     */
    public void setLowerBound(int lowerBound) {
        this.lowerBound = lowerBound;
    }

    /**
     * @return the lowerBound
     */
    public int getLowerBound() {
        return lowerBound;
    }

    /**
     * @param higherBound
     *            the higherBound to set
     */
    public void setHigherBound(int higherBound) {
        this.higherBound = higherBound;
    }

    /**
     * @return the higherBound
     */
    public int getHigherBound() {
        return higherBound;
    }

    @SuppressWarnings("unchecked")
	public Map toMap() {
        try {
            Map retval = PropertyUtils.describe(this);
            retval.remove("class");
            return retval;
        } catch (Exception ex) {
            throw new SymmitronException("Error generating map", ex);
        }
    }

    public int range() {
        return this.higherBound - this.lowerBound;
    }
    
    public String toString() {
       return new StringBuffer().append("lowBound = " + this.lowerBound + "\n")
        .append("highBound = " + this.higherBound + "\n").toString();
    }

}
