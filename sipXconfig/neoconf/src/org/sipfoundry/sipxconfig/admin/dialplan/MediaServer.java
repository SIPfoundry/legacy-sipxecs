/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.permission.PermissionName;

public abstract class MediaServer {

    public static enum Operation {
        Autoattendant, VoicemailRetrieve, VoicemailDeposit, SOS
    }

    protected static final List<String> ENCODE_EXCLUDES = Arrays.<String> asList(new String[] {
        "{digits}", "{vdigits}", "{digits-escaped}", "{vdigits-escaped}", "{host}"
    });

    private String m_hostname;

    private String m_serverExtension;

    /**
     * User visible label of for this media server.
     */
    private String m_label;

    /**
     * Localization hint for media server
     */
    private LocalizationContext m_localizationContext;

    /**
     * Get the name (type) of this media server
     * 
     * @return The name of this media server
     */
    public abstract String getName();

    /**
     * Returns a URL-encoded String representing the URI parameters for the specified operation on
     * this media server.
     * 
     * @param operation - The type of operation for which to get the URI params
     * @param digits - The user digits relevant to this call
     * @param additionalParams - Any additional parameters that are required by the media server
     *        for the specified operation
     * @return The URL-encoded URI parameter String
     */
    public abstract String getUriParameterStringForOperation(Operation operation,
            CallDigits userDigits, Map<String, String> additionalParams);

    /**
     * Returns a URL-encoded String representing the header parameters for the specified operation
     * on this media server.
     * 
     * @param operation - The type of operation for which to get the header params
     * @param digits - The user digits relevant to this call
     * @param additionalParams - Any additional parameters that are required by the media server
     *        for the specified operation
     * @return The URL-encoded header parameter String
     */
    public abstract String getHeaderParameterStringForOperation(Operation operation,
            CallDigits userDigits, Map<String, String> additionalParams);

    /**
     * Returns the call digits for the specified operation. These digits will usually be used as
     * part of the SIP URI, such as mediaServerDigits@host.com. The value of these digits is
     * usually either the Media Server's extension or the {digits} or {vdigits} placeholder, but
     * the MediaServer implementation is free to return what is most appropriate
     * 
     * @param operation The type of operation for which to return the digits
     * @param userDigits The digits for the user related to the operation
     * @return The CallDigits to use for the SIP URI
     */
    public abstract String getDigitStringForOperation(Operation operation, CallDigits userDigits);

    /**
     * Gets the PermissionName object associated with this type of MediaServer
     * 
     * @return A PermissionName specifying the permission that applies to this server
     */
    public abstract PermissionName getPermissionName();

    /**
     * Gets the hostname of this server
     * 
     * @return The hostname of this server
     */
    public String getHostname() {
        return m_hostname;
    }

    /**
     * Sets the hostname of this server
     * 
     * @param address - Hostname of this server
     */
    public void setHostname(String hostname) {
        m_hostname = hostname;
    }

    public String getLabel() {
        return m_label;
    }

    public void setLabel(String label) {
        m_label = label;
    }

    /**
     * Gets the extension used to dial the server
     * 
     * @return The String representation of the server's extension
     */
    public String getServerExtension() {
        return m_serverExtension;
    }

    /**
     * Sets the extension used to dial the server
     * 
     * @param serverExtension - The extension used to dial the server
     */
    public void setServerExtension(String serverExtension) {
        m_serverExtension = serverExtension;
    }

    /**
     * Encodes a param string (such as header params or uri params) using URL encoding. Supports
     * the ability to not encode any strings included in the 'exclude' parameter
     * 
     * @param paramString - The parameter string to encode
     * @param exclude - A list of strings to exclude from the encoding
     */
    protected String encodeParams(String paramString, List<String> excludes) {
        try {
            String encodingType = "UTF-8";
            String encodedParams = URLEncoder.encode(paramString, encodingType);

            if (excludes != null) {
                for (String string : excludes) {
                    if (paramString.contains(string)) {
                        String encodedExclude = URLEncoder.encode(string, encodingType);
                        encodedParams = encodedParams.replace(encodedExclude, string);
                    }
                }
            }
            return encodedParams;
        } catch (UnsupportedEncodingException e) {
            // UTF-8 is always supported
            throw new RuntimeException(e);
        }

    }

    public void setLocalizationContext(LocalizationContext localizationContext) {
        m_localizationContext = localizationContext;
    }

    /**
     * Add language parameter if media server supports it
     * 
     * @param params name->value map of additional params
     * @param locale locale indication
     */
    protected abstract void addLang(Map<String, String> params, String locale);

    /**
     * Builds a URL based on the provided digits, media server, and sip parameters.
     * 
     * @param userDigits - The digits for the relevant user (or null if none)
     * @param sipParams - any additional SIP params (can be null or empty string)
     * @return String representing the URL
     */
    public String buildUrl(CallDigits userDigits, Operation operation, String sipParams) {
        Map<String, String> langHint = new TreeMap<String, String>();
        String language = m_localizationContext.getCurrentLanguage();
        addLang(langHint, language);
        String uriParams = getUriParameterStringForOperation(operation, userDigits, langHint);
        String headerParams = getHeaderParameterStringForOperation(operation, userDigits, null);
        String hostname = getHostname();
        String digits = getDigitStringForOperation(operation, userDigits);
        return MappingRule.buildUrl(digits, hostname, uriParams, headerParams, sipParams);
    }
}
