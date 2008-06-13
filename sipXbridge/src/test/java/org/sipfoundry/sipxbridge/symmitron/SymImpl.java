package org.sipfoundry.sipxbridge.symmitron;
import java.util.Map;

import org.sipfoundry.sipxbridge.symmitron.SymEndpointInterface;
import org.sipfoundry.sipxbridge.symmitron.SymInterface;


public class SymImpl implements SymInterface {
    String id;
    
    private SymEndpointInterface receiver;
    
    private SymEndpointInterface transmitter;
    
 

    public String getId() {
        
        return id;
    }

    public SymEndpointInterface getReceiver() {
       
        return receiver;
    }

    public SymEndpointInterface getTransmitter() {
       
        return transmitter;
    }

    /**
     * @param receiver the receiver to set
     */
    public void setReceiver(SymEndpointInterface receiver) {
        this.receiver = receiver;
    }

    /**
     * @param transmitter the transmitter to set
     */
    public void setTransmitter(SymEndpointInterface transmitter) {
        this.transmitter = transmitter;
    }

  
}
