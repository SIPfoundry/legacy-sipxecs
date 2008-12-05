/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.List;

import org.apache.commons.lang.enums.Enum;

public interface SipxProcessModel {
    public static final class ProcessName extends Enum {
        public static final ProcessName REGISTRAR = new ProcessName("SIPRegistrar");
        public static final ProcessName STATUS = new ProcessName("SIPStatus");
        public static final ProcessName PROXY = new ProcessName("SIPXProxy");
        public static final ProcessName MEDIA_SERVER = new ProcessName("MediaServer");
        public static final ProcessName PARK_SERVER = new ProcessName("ParkServer");
        public static final ProcessName PRESENCE_SERVER = new ProcessName("PresenceServer");
        public static final ProcessName CONFIG_SERVER = new ProcessName("ConfigServer");
        public static final ProcessName KEEP_ALIVE = new ProcessName("KeepAlive");
        public static final ProcessName CONFIG_AGENT = new ProcessName("ConfigAgent");
        public static final ProcessName CALL_RESOLVER = new ProcessName("CallResolver");
        public static final ProcessName ACD_SERVER = new ProcessName("ACDServer");
        public static final ProcessName RL_SERVER = new ProcessName("ResourceListServer");
        public static final ProcessName PAGE_SERVER = new ProcessName("PageServer");
        public static final ProcessName SBC_BRIDGE = new ProcessName("SipXbridge");
        public static final ProcessName FREESWITCH_SERVER = new ProcessName("FreeSWITCH");
        public static final ProcessName RELAY = new ProcessName("SipXrelay");
        public static final ProcessName IVR = new ProcessName("sipXivr");
        public static final ProcessName MRTG = new ProcessName("sipXmrtg");

        private ProcessName(String name) {
            super(name);
        }

        public static ProcessName getEnum(String name) {
            return (ProcessName) getEnum(ProcessName.class, name);
        }
        
        public static List getAll() {
            return getEnumList(ProcessName.class);
        }
    }

    public Process getProcess(String name);

    public Process getProcess(ProcessName name);

    /**
     * This should be used to get list of all the services except KEEP_ALIVE and CONFIG_SERVER
     * 
     * @return list of the services that you want usually restart
     * 
     */
    public List<Process> getRestartable();
}
