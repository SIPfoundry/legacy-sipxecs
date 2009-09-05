package org.sipfoundry.sipxrest;

public class MetaInf {
    private String pluginClass;
    private String security = LOCAL_AND_REMOTE;
    private String urlPrefix;
    private String serviceDescription;
    /*
     * Standard access control models.
     */
    public static String LOCAL_AND_REMOTE  = "LOCAL-AND-REMOTE";
    public static String LOCAL_ONLY = "LOCAL-ONLY";
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
    public void setUrlPrefix(String urlPrefix) {
        this.urlPrefix = urlPrefix;
    }
    /**
     * @return the urlPrefix
     */
    public String getUrlPrefix() {
        return urlPrefix;
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

}
