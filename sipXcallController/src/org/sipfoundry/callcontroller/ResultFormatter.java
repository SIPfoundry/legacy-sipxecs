/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.callcontroller;

import org.restlet.data.Status;

public class ResultFormatter {
    public static String HEADER = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + "\n<processing-status "
    + "xmlns=\"http://www.sipfoundry.org/sipX/schema/xml/call-status-00-00\">\n";
    public static String FOOTER = "</processing-status>\n";
    public static String ERROR_TEXT = "error-text";
    public static String ERROR_CODE = "error-code";
    public static String ERROR_FLAG = "error-flag";
    
    public static String formatError(boolean errorFlag, int errorCode, String errorText)  {
        StringBuffer retval = new StringBuffer();
        retval.append(HEADER);
        retval.append("<"+ERROR_FLAG+">");
        retval.append(errorFlag? "ERROR" : "OK" );
        retval.append("</" + ERROR_FLAG + ">\n");
        retval.append("<" + ERROR_CODE + ">" + errorCode + "</" + ERROR_CODE + ">\n");
        retval.append("<" + ERROR_TEXT + ">" + errorText + "</" + ERROR_TEXT + ">\n");
        retval.append(FOOTER);
        return retval.toString();
    }
    
    public static String formatOkResponse() {
        return formatError(false,200,"Success");
    }
    
    public static String formatError(Status status, String error) {
        return formatError(true,status.getCode(),error );
    }

}
