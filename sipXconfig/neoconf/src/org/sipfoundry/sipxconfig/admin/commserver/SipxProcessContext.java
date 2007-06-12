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

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.lang.enums.Enum;

public interface SipxProcessContext {
    // TODO: this should be created by reading ProcessDefinition.xml
    public static final class Process extends Enum {
        public static final Process REGISTRAR = new Process("SIPRegistrar");
        public static final Process AUTH_PROXY = new Process("SIPAuthProxy");
        public static final Process STATUS = new Process("SIPStatus");
        public static final Process PROXY = new Process("SIPProxy");
        public static final Process MEDIA_SERVER = new Process("MediaServer");
        public static final Process PARK_SERVER = new Process("ParkServer");
        public static final Process PRESENCE_SERVER = new Process("PresenceServer");
        public static final Process CONFIG_SERVER = new Process("ConfigServer");
        public static final Process KEEP_ALIVE = new Process("KeepAlive");
        public static final Process CONFIG_AGENT = new Process("ConfigAgent");
        public static final Process CALL_RESOLVER = new Process("CallResolver");
        public static final Process ACD_SERVER = new Process("ACDServer");
        public static final Process RL_SERVER = new Process("ResourceListServer");

        private Process(String name) {
            super(name);
        }

        public static Process getEnum(String name) {
            return (Process) getEnum(Process.class, name);
        }

        public static List getAll() {
            return getEnumList(Process.class);
        }

        /**
         * This should be used to get list of all the services except KEEP_ALIVE and CONFIG_SERVER
         * 
         * @return list of the services that you want usually restart
         * 
         */
        public static List getRestartable() {
            Process[] noRestart = {
                KEEP_ALIVE, CONFIG_SERVER
            };
            List processes = new LinkedList(getAll());
            processes.removeAll(Arrays.asList(noRestart));
            return Collections.unmodifiableList(processes);
        }
    }

    public static class Command extends Enum {
        public static final Command START = new Command("start");
        public static final Command STOP = new Command("stop");
        public static final Command RESTART = new Command("restart");
        public static final Command STATUS = new Command("status");

        public Command(String name) {
            super(name);
        }
    }

    /**
     * Return an array containing a ServiceStatus entry for each process on the first server
     * machine. This is a first step towards providing status for all server machines.
     */
    public ServiceStatus[] getStatus(Location location);

    public Location[] getLocations();

    /**
     * Apply the specified command to the named services. This method handles only commands that
     * don't need output, which excludes the "status" command.
     * 
     * @param services list of services that will receive the command
     * @param command command to send
     * @param location information about the host on which services are running
     */
    public void manageServices(Location location, Collection services, Command command);

    public void manageServices(Collection services, Command command);

    /**
     * Delayed version of manageServices strictly for restarting services. Restart commands is not
     * send until the event of the specific class is received.
     * 
     * @param services list of services that will receive the command
     * @param eventClass class of event that will trigger the command
     */
    public void restartOnEvent(Collection services, Class eventClass);

    /**
     * Apply the specified command to the process/service. This method handles only commands that
     * don't need output, which excludes the "status" command.
     */
    public void manageService(Location location, Process process, Command command);
}
