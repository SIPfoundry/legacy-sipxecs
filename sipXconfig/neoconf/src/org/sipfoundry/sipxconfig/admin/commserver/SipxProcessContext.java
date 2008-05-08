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

import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;

public interface SipxProcessContext {
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
    public void manageServices(Location location, Collection<Process> services, Command command);

    public void manageServices(Collection<Process> services, Command command);

    /**
     * Delayed version of manageServices strictly for restarting services. Restart commands is not
     * send until the event of the specific class is received.
     * 
     * @param services list of services that will receive the command
     * @param eventClass class of event that will trigger the command
     */
    public void restartOnEvent(Collection<Process> services, Class eventClass);

    /**
     * Apply the specified command to the processes/services. 
     */
    public void manageService(Location location,  Collection<Process> processes, Command command);
    
    /**
     * This should be used to get list of restartable Processes
     * 
     * @return list of the services that you usually want to restart
     * 
     */
    public List<Process> getRestartable();
    
    public Process getProcess(String name);
    public Process getProcess(ProcessName name);
    public Process[] getProcess(ProcessName[] names);
}
