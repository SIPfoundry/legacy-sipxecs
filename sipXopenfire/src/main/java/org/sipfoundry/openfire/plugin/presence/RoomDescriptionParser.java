/*
 * 
 * 
 * Copyright (C) 2009 Nortel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */

package org.sipfoundry.openfire.plugin.presence;


public class RoomDescriptionParser {
    
    public static String generateDescriptionDocument(String subdomain, String roomName, String bridgeExtension) {
        StringBuffer retval =  new StringBuffer();
        retval.append("<?xml version=\"1.0\" ?>\n");
        retval.append("<room-description xmlns=\"http://www.sipfoundry.org/sipX/schema/xml/roomdescription-00-00\">\n");      
        retval.append("<subdomain>");
        retval.append(subdomain);
        retval.append("</subdomain>\n");
        retval.append("<room-name>"); 
        retval.append(roomName);
        retval.append("</room-name>\n");
        if ( bridgeExtension != null ) {
            retval.append("<bridge-extension>");
            retval.append(bridgeExtension);
            retval.append("</bridge-extension>\n");
        }      
        retval.append("</room-description>\n");
        return retval.toString();
    }

}
