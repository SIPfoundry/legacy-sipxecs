/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrelay;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.net.InetAddress;

import org.apache.log4j.Logger;

public class SymmitronConfig {

	private static Logger logger = Logger.getLogger(SymmitronConfig.class);

	private int portRangeLowerBound = 15000;
	private int portRangeUpperBound = 15500;
	private int xmlRpcPort = 8089;
	private String localAddress;
	private String logFileName = "sipxrelay.log";
	private String logFileDirectory = null;
	private String logLevel = "DEBUG";
	private String publicAddress;
	private String stunServerAddress;
	private int rediscoveryTime = 60;
	private boolean useHttps = true;
	private boolean behindNat = true;
	private int sipxSupervisorXmlRpcPort = 0;
	private String sipxSupervisorHost;

    private boolean rejectStrayPackets = true;

	public SymmitronConfig() {

	}

	public void setXmlRpcPort(int xmlRpcPort) {
		this.xmlRpcPort = xmlRpcPort;
	}

	/**
	 * @param portRange
	 */
	public void setPortRange(String portRange) {
		String[] ports = portRange.split(":");
		if (ports.length != 2) {
			throw new IllegalArgumentException(
					"Must have format lower:upper bound");
		}
		String lowBound = ports[0];
		String highBound = ports[1];
		this.portRangeLowerBound = new Integer(lowBound).intValue();
		this.portRangeUpperBound = new Integer(highBound).intValue();
		if (this.portRangeLowerBound >= this.portRangeUpperBound
				|| this.portRangeLowerBound < 0 || this.portRangeUpperBound < 0) {
			throw new IllegalArgumentException(
					"Port range should be lower:upper bound integers and postivie");
		}
	}

	/**
	 * @param localAddress
	 *            the localAddress to set
	 */
	public void setLocalAddress(String localAddress) {
		try {
			this.localAddress = InetAddress.getByName(localAddress)
					.getHostAddress();
		} catch (Exception ex) {
			throw new IllegalArgumentException("invalid address : "
					+ localAddress);
		}
	}

	/**
	 * @return the xmlRpcPort
	 */
	public int getXmlRpcPort() {
		return xmlRpcPort;
	}

	/**
	 * @return the localAddress
	 */
	public String getLocalAddress() {
		return localAddress;
	}

	public int getPortRangeLowerBound() {
		return portRangeLowerBound;
	}

	/**
	 * @param portRangeLowerBound
	 *            the portRangeLowerBound to set
	 */
	public void setPortRangeLowerBound(int portRangeLowerBound) {
		this.portRangeLowerBound = portRangeLowerBound;
	}

	/**
	 * @param portRangeUpperBound
	 *            the portRangeUpperBound to set
	 */
	public void setPortRangeUpperBound(int portRangeUpperBound) {
		this.portRangeUpperBound = portRangeUpperBound;
	}

	/**
	 * @return the portRangeUpperBound
	 */
	public int getPortRangeUpperBound() {
		return portRangeUpperBound;
	}

	public void setLogFileDirectory(String logFileDirectory) {
		this.logFileDirectory = logFileDirectory;
	}

	public String getLogFileDirectory() {
		return logFileDirectory;
	}

	/**
	 * @param logLevel
	 *            the logLevel to set
	 */
	public void setLogLevel(String logLevel) {
		this.logLevel = logLevel;
	}

	/**
	 * @return the logLevel
	 */
	public String getLogLevel() {
		return logLevel;
	}

	/**
	 * @param logFileName
	 *            the logFileName to set
	 */
	public void setLogFileName(String logFileName) {
		this.logFileName = logFileName;
	}

	/**
	 * @return the logFileName
	 */
	public String getLogFileName() {
		return logFileName;
	}

	public void setPublicAddress(String publicAddress) {
		try {
			this.publicAddress = InetAddress.getByName(publicAddress)
					.getHostAddress();
		} catch (Exception ex) {
			throw new IllegalArgumentException(ex);
		}
	}

	/**
	 * @return the public address of the relay.
	 */
	public String getPublicAddress() {
		if (this.publicAddress != null) {
			return this.publicAddress;
		} else if (!this.behindNat) {
			return this.getLocalAddress();
		} else {
			return null;
		}
	}

	/**
	 * @param stunServerAddress
	 *            the stunServerAddress to set
	 */
	public void setStunServerAddress(String stunServerAddress) {
		this.stunServerAddress = stunServerAddress;
	}

	/**
	 * @return the stunServerAddress
	 */
	public String getStunServerAddress() {
		return stunServerAddress;
	}

	/**
	 * @param rediscoveryTime
	 *            the rediscoveryTime to set
	 */
	public void setRediscoveryTime(int rediscoveryTime) {
		this.rediscoveryTime = rediscoveryTime;
	}

	/**
	 * @return the rediscoveryTime
	 */
	public int getRediscoveryTime() {
		return rediscoveryTime;
	}

	public void setUseHttps(boolean flag) {
		this.useHttps = flag;
	}

	public boolean getUseHttps() {
		return this.useHttps;
	}

	/**
	 * @param behindNat
	 *            the behindNat to set
	 */
	public void setBehindNat(boolean behindNat) {
		this.behindNat = behindNat;
	}

	/**
	 * @return the behindNat
	 */
	public boolean isBehindNat() {
		return behindNat;
	}

	public void setSipXSupervisorXmlRpcPort(int port) {
		this.sipxSupervisorXmlRpcPort = port;
	}

	public int getSipXSupervisorXmlRpcPort() {
		return this.sipxSupervisorXmlRpcPort;
	}

	public void setSipXSupervisorHost(String hostName) {
		this.sipxSupervisorHost = hostName;
	}

	public String getSipXSupervisorHost() {
		return this.sipxSupervisorHost;
	}
	
	public void setRejectStrayPackets(boolean rejectStrayPackets) {
	    this.rejectStrayPackets = rejectStrayPackets;
	}

    /**
     * @return the rejectStrayPackets
     */
    public boolean isRejectStrayPackets() {
        return rejectStrayPackets;
    }
	
	

}
