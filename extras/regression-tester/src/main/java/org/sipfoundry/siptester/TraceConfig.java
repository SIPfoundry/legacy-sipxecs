package org.sipfoundry.siptester;

import java.util.HashSet;

public class TraceConfig {
      
    HashSet<TraceEndpoint> sutUACollection = new HashSet<TraceEndpoint>();
    
    
    public void addTraceEndpoint(TraceEndpoint sutUa) {
        this.sutUACollection.add(sutUa);
    }
    
    public HashSet<TraceEndpoint> getTraceEndpoints() {
        return this.sutUACollection;
    }
    
   

}
