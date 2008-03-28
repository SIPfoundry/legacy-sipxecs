package org.sipfoundry.sipxbridge;

import java.io.Serializable;

/**
 * Port range. Just a simple Low and high bound structure.
 * 
 * @author mranga
 *
 */
public class PortRange implements Serializable {
    
    private int lowBound;
    
    private int highBound;

    /**
     * @param lowBound the lowBound to set
     */
    public void setLowBound(int lowBound) {
        this.lowBound = lowBound;
    }

    /**
     * @return the lowBound
     */
    public int getLowBound() {
        return lowBound;
    }

    /**
     * @param highBound the highBound to set
     */
    public void setHighBound(int highBound) {
        this.highBound = highBound;
    }

    /**
     * @return the highBound
     */
    public int getHighBound() {
        return highBound;
    }
    
    

}
