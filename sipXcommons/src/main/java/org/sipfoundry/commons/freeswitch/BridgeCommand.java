/*
 * Copyright (c) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */

package org.sipfoundry.commons.freeswitch;

public class BridgeCommand extends CallCommand {

    public BridgeCommand(FreeSwitchEventSocketInterface fses, String sipURI, String sipContext) {
        super(fses);
        // Send a REFER
        m_command = "bridge\nexecute-app-arg: sofia/";
        m_command += sipContext;
        m_command += "/";
        // sipURI MUST have sip: in there (Can be display URI)
        if (sipURI.toLowerCase().contains("sip:")) {
            m_command += sipURI;
        } else {
            m_command += "sip:" + sipURI;
        }
        m_command += "\n";
        m_command += "event-lock: true";
    }
}
