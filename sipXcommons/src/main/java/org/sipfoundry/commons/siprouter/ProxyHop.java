package org.sipfoundry.commons.siprouter;

import javax.sip.address.Hop;


public class ProxyHop implements Hop, Comparable<ProxyHop> {
	
	private int priority;
	private String host;
	private int port;
	private String transport;
	
	public ProxyHop(String host, int port, String transport) {
		this.host = host;
		this.port = port;
		this.transport = transport;
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


	public void setPriority( int priority ) {
		this.priority = priority;
	}
	
	
	
	public int compareTo(ProxyHop otherHop ) {	
		if ( otherHop.priority == priority ) return 0;
		else if ( otherHop.priority > priority ) return -1;
		else return +1;
	}
	
	

}
