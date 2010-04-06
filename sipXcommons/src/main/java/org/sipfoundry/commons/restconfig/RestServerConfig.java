/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.restconfig;

import java.util.Date;

import org.apache.log4j.Level;

public class RestServerConfig {
	
	private String ipAddress;
	private int sipPort;
	private int httpPort;
	private int publicHttpPort;
	
	private String loggingLevel = Level.INFO.toString();
	private String logDirectory = "/var/log/sipxpbx/";
	private String sipxProxyDomain;
	
	public void setIpAddress(String ipAddress) {
		this.ipAddress = ipAddress;
	}
	
	public String getIpAddress() {
		return ipAddress;
	}
	
	public void setSipPort(int sipPort) {
		this.sipPort = sipPort;
	}
	
	public int getSipPort() {
		return sipPort;
	}
	
	public void setHttpPort(int httpPort) {
		this.httpPort = httpPort;
	}
	
	public int getHttpPort() {
		return httpPort;
	}
	
	public void setPublicHttpPort(int publicHttpPort) {
	    this.publicHttpPort = publicHttpPort;
	}
	
	public int getPublicHttpPort() {
	    return this.publicHttpPort;
	}
	
	public void setLogLevel(String loggingLevel) {
		this.loggingLevel = loggingLevel;
	}
	
	public String getLogLevel() {
		return loggingLevel;
	}
	
	public void setLogDirectory(String logDirectory) {
		this.logDirectory = logDirectory;
	}
	
	public String getLogDirectory() {
		return logDirectory;
	}
	
	public void setSipxProxyDomain(String sipxProxyDomain) {
		this.sipxProxyDomain = sipxProxyDomain;
	}
	
	public String getSipxProxyDomain() {
		return sipxProxyDomain;
	}

    public int getCacheTimeout() {
        return 30;
    }

}
