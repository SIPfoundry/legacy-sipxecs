package org.sipfoundry.siptester;

import java.util.HashSet;

public class TraceConfig {
      
    HashSet<TraceEndpoint> traceEndpointCollection = new HashSet<TraceEndpoint>();
    
    HashSet<HostPort> endpointsOfInterest = new HashSet<HostPort> ();
    
    
    public void addTraceEndpoint(TraceEndpoint traceEndpoint) {
        if ( traceEndpoint.isEmulated() ) {
            this.traceEndpointCollection.add(traceEndpoint);
        } 
        this.endpointsOfInterest.add(traceEndpoint);
    }
    
    public HashSet<TraceEndpoint> getEmulatedEndpoints() {
        return this.traceEndpointCollection;
    }

    public boolean isEndpointOfInterest(HostPort targetHostPort) {
       return endpointsOfInterest.contains(targetHostPort);
    }

    public void printEndpoints() {
      System.out.println(this.endpointsOfInterest)  ;
    }
    
   

}
