package org.sipfoundry.siptester;

import java.util.HashSet;

public class TraceConfig {
      
    HashSet<TraceEndpoint> emulatedEndpoints = new HashSet<TraceEndpoint>();
    
    HashSet<TraceEndpoint> allEndpoints = new HashSet<TraceEndpoint>();
      
    
    public void addTraceEndpoint(TraceEndpoint traceEndpoint) {
        if ( traceEndpoint.isEmulated() ) {
            this.emulatedEndpoints.add(traceEndpoint);
        }
        this.allEndpoints.add(traceEndpoint);
      
        System.out.println("traceEndpoint = " + traceEndpoint.getTraceIpAddresses());
    }
    
    public HashSet<TraceEndpoint> getEmulatedEndpoints() {
        return this.emulatedEndpoints;
    }

    public boolean isEndpointOfInterest(HostPort targetHostPort) {
       for (TraceEndpoint traceEndpoint : this.allEndpoints) {
           if ( traceEndpoint.getTraceIpAddresses().contains(targetHostPort)) return true;
       }
       return false;
    }

	public boolean isEndpointEmulated(HostPort targetHostPort) {
		  for (TraceEndpoint traceEndpoint : this.emulatedEndpoints) {
	           if ( traceEndpoint.getTraceIpAddresses().contains(targetHostPort)) return true;
	       }
	       return false;
	}
    
   

}
