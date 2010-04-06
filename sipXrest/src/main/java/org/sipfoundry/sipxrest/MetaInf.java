/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrest;

public class MetaInf {
    private String pluginClass;
    private String security = LOCAL_AND_REMOTE;
    private String uriPrefix;
    private String serviceDescription;
    private String sipUserName;
    private String sipListenerClassName;
    private String remoteAuthenticationMethod = BASIC_AND_DIGEST;
    
    /*
     * Standard access control models.
     */
    public static String LOCAL_AND_REMOTE  = "LOCAL-AND-REMOTE";
    public static String LOCAL_ONLY = "LOCAL-ONLY";
    public static String HTTP_DIGEST = "HTTP-DIGEST";
    public static String HTTPS_BASIC = "HTTPS-BASIC";
    public static String BASIC_AND_DIGEST = "BASIC-AND-DIGEST";
    /**
     * @param pluginClass the fully qualified class path of the Plugin class.
     * @throws ClassNotFoundException 
     */
    public void setPluginClass(String pluginClass) throws ClassNotFoundException {
        Class<?> clazz = Class.forName(pluginClass);
        if ( ! Plugin.class.isAssignableFrom(clazz) ) {
            throw new IllegalArgumentException ( pluginClass + " not subclass of Plugin");
        }
        this.pluginClass = pluginClass;
    }
    
    /**
     * @return the restletClass
     */
    public String getPluginClass() {
        return pluginClass;
    }
    /**
     * @param security the security to set
     */
    public void setSecurity(String security) {
        if ( security.equals("LOCAL-AND-REMOTE") || security.equals("LOCAL-ONLY")) {
            this.security = security;
        } else {
            throw new IllegalArgumentException("Invalid security model " + security);
        }
    }
    /**
     * @return the security
     */
    public String getSecurity() {
        
        return security;
    }
    /**
     * @param urlPrefix the urlPrefix to set
     */
    public void setUriPrefix(String urlPrefix) {
        this.uriPrefix = urlPrefix;
    }
    /**
     * @return the urlPrefix
     */
    public String getUriPrefix() {
        return uriPrefix;
    }
    /**
     * @param serviceDescription the serviceDescription to set
     */
    public void setServiceDescription(String serviceDescription) {
        this.serviceDescription = serviceDescription;
    }
    /**
     * @return the serviceDescription
     */
    public String getServiceDescription() {
        return serviceDescription;
    }

    /**
     * @param sipUserName the sipUserName to set
     */
    public void setSipConvergenceName(String sipUserName) {
        this.sipUserName = sipUserName;
    }

    /**
     * @return the sipUserName
     */
    public String getSipConvergenceName() {
        return sipUserName;
    }

    
    /**
     * Get the Sip Listener class
     */
    public String getSipListenerClassName() {
        return this.sipListenerClassName;
    }

    /**
     * @param sipListenerClassName the sipListenerClassName to set
     */
    public void setSipListenerClassName(String sipListenerClassName) {
        this.sipListenerClassName = sipListenerClassName;
    }

    /**
     * @param remoteAuthenticationMethod the remoteAuthenticationMethod to set
     */
    public void setRemoteAuthenticationMethod(String remoteAuthenticationMethod) {
        if ( remoteAuthenticationMethod.equals(HTTPS_BASIC) || remoteAuthenticationMethod.equals(HTTP_DIGEST) ||
                remoteAuthenticationMethod.equals(BASIC_AND_DIGEST) )  {
            this.remoteAuthenticationMethod = remoteAuthenticationMethod;
        }  else throw new IllegalArgumentException("Illegal authentication method " + remoteAuthenticationMethod);
            
    }

    /**
     * @return the remoteAuthenticationMethod
     */
    public String getRemoteAuthenticationMethod() {
        return remoteAuthenticationMethod;
    }

}
