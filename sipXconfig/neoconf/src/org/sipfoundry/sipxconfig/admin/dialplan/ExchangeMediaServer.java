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

import java.util.ArrayList;
import java.util.Formatter;
import java.util.List;
import java.util.Map;

public class ExchangeMediaServer extends MediaServer {

    private static final String NAME = "ExchangeUM";
    private static final String HEADER_PARAM_TEMPLATE = "<sip:{%s}@{host}>;reason=no-answer;screen=no;privacy=off";

    /**
     * Create a new instance of ExchangeMediaServer
     */
    public ExchangeMediaServer() {
        // create empty media server object
    }
    
    /**
     * Create a new instance of ExchangeMediaServer with the specified parameters
     * @param serverAddress - The address/hostname of the server
     * @param voicemailExtension - The extension of the server
     */
    public ExchangeMediaServer(String serverAddress, String voicemailExtension) {
        setHostname(serverAddress);
        setServerExtension(voicemailExtension);
    }

    public String getHeaderParameterStringForOperation(Operation operation, CallDigits digits,
            Map<String, String> additionalParams) {
        String paramString = null;

        switch (operation) {
        case VoicemailDeposit:
            StringBuffer paramBuffer = new StringBuffer();
            Formatter paramFormatter = new Formatter(paramBuffer);
            paramFormatter.format(HEADER_PARAM_TEMPLATE, digits.getName());
            List<String> encodeExcludes = new ArrayList<String>(ENCODE_EXCLUDES);
            encodeExcludes.add("sip:");
            paramString = encodeParams(paramBuffer.toString(), encodeExcludes);
            break;
        default:
            break;
        }

        if (paramString == null) {
            return "";
        } else {
            return "Diversion=" + paramString;
        }
    }

    public String getUriParameterStringForOperation(Operation operation, CallDigits digits,
            Map<String, String> additionalParams) {
        return "transport=tcp";
    }

    public String getName() {
        return NAME;
    }

    public String getDigitStringForOperation(Operation operation, CallDigits userDigits) {
        return getServerExtension();
    }
}
