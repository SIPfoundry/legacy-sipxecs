package org.sipfoundry.openfire.config;

public class WatcherConfig {
    private String sipxProxyDomain;
    private int sipxProxyPort = 5060;
    private String userName;
    private String password;
    private String sipxProxyTransport;
    private String logLevel = "INFO";
    private String watcherAddress;
    private int watcherPort;
    private String resourceList;
    private String logDirectory = "/var/log/sipxpbx";
    private int openfireXmlRpcPort = 9094;
    private String openfireHost;
    
    public String getSipxProxyTransport() {
        return sipxProxyTransport;
    }
    
    public String getLogLevel() {
        return logLevel;
    }
    
    public void setLogLevel(String logLevel) 
    {
        this.logLevel = logLevel;
    }

    /**
     * @param proxyAddress the proxyAddress to set
     */
    public void setProxyDomain(String proxyDomain) {
        this.sipxProxyDomain = proxyDomain;
    }
    /**
     * @return the proxyAddress
     */
    public String getProxyDomain() {
        return sipxProxyDomain;
    }
    /**
     * @param userName the userName to set
     */
    public void setUserName(String userName) {
        this.userName = userName;
    }
    /**
     * @return the userName
     */
    public String getUserName() {
        return userName;
    }
    /**
     * @param port the port to set
     */
    public void setProxyPort(int port) {
        this.sipxProxyPort = port;
    }
    /**
     * @return the port
     */
    public int getProxyPort() {
        return sipxProxyPort;
    }
    /**
     * @param password the password to set
     */
    public void setPassword(String password) {
        this.password = password;
    }
    /**
     * @return the password
     */
    public String getPassword() {
        return password;
    }

    /**
     * @param watcherAddress the watcherAddress to set
     */
    public void setWatcherAddress(String watcherAddress) {
        this.watcherAddress = watcherAddress;
    }

    /**
     * @return the watcherAddress
     */
    public String getWatcherAddress() {
        return watcherAddress;
    }

    /**
     * @param watcherPort the watcherPort to set
     */
    public void setWatcherPort(int watcherPort) {
        this.watcherPort = watcherPort;
    }

    /**
     * @return the watcherPort
     */
    public int getWatcherPort() {
        return watcherPort;
    }

    /**
     * @param logDirectory the logDirectory to set
     */
    public void setLogDirectory(String logDirectory) {
        this.logDirectory = logDirectory;
    }

    /**
     * @return the logDirectory
     */
    public String getLogDirectory() {
        return logDirectory;
    }

	public String getResourceList() {
		return resourceList;
	}

	public void setResourceList(String resourceList) {
		this.resourceList = resourceList;
	}
	
	public WatcherConfig() {
	    
	}

    /**
     * @param pluginXmlRpcPort the pluginXmlRpcPort to set
     */
    public void setOpenfireXmlRpcPort(int pluginXmlRpcPort) {
        this.openfireXmlRpcPort = pluginXmlRpcPort;
    }

    /**
     * @return the pluginXmlRpcPort
     */
    public int getOpenfireXmlRpcPort() {
        return openfireXmlRpcPort;
    }

    /**
     * @param openfireHost the openfireHost to set
     */
    public void setOpenfireHost(String openfireHost) {
        this.openfireHost = openfireHost;
    }

    /**
     * @return the openfireHost
     */
    public String getOpenfireHost() {
        return openfireHost;
    }
    
   
   

}
