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

import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.List;
import java.util.Set;
import java.util.Vector;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;

public class SipxProcessContextImplTest extends TestCase {
    private SipxProcessContextImpl m_processContextImpl;
    private LocationsManager m_locationsManager;

    private int m_numberOfCalls;
    private List m_urlStrings;
    private List m_methodNameStrings;
    private List m_paramVectors;

    private final static Process[] STOPPROCESSLIST = new Process[] {
        new Process(ProcessName.REGISTRAR), new Process(ProcessName.MEDIA_SERVER)
    };

    private final static Process[] STARTPROCESSLIST = new Process[] {
        new Process(ProcessName.PRESENCE_SERVER)
    };

    private final static Process[] RESTARTPROCESSLIST = new Process[] {
        new Process(ProcessName.REGISTRAR), new Process(ProcessName.MEDIA_SERVER),
        new Process(ProcessName.PRESENCE_SERVER), new Process(ProcessName.ACD_SERVER)
    };

    private final static ServiceStatus[] SERVICESTATUS = new ServiceStatus[] {
        new ServiceStatus(new Process(ProcessName.REGISTRAR), ServiceStatus.Status.STARTING),
        new ServiceStatus(new Process(ProcessName.MEDIA_SERVER), ServiceStatus.Status.STARTED),
        new ServiceStatus(new Process(ProcessName.PRESENCE_SERVER), ServiceStatus.Status.STOPPED),
        new ServiceStatus(new Process(ProcessName.PROXY), ServiceStatus.Status.FAILED),
        new ServiceStatus(new Process(ProcessName.ACD_SERVER), ServiceStatus.Status.UNKNOWN)
    };

    protected void setUp() throws Exception {
        m_locationsManager = new LocationsManagerImpl() {
            protected InputStream getTopologyAsStream() {
                return LocationsManagerImplTest.class.getResourceAsStream("topology.test.xml");
            }
        };

        m_processContextImpl = new SipxProcessContextImpl() {

            protected Object invokeXmlRpcRequest(Location location, String methodName, Vector params) {
                m_numberOfCalls++;
                m_urlStrings.add(location.getProcessMonitorUrl());
                m_methodNameStrings.add(methodName);
                m_paramVectors.add(params);

                Hashtable result = new Hashtable();
                if ("getStateAll" == methodName) {
                    for (int x = 0; x < SERVICESTATUS.length; x++) {
                        result.put(SERVICESTATUS[x].getServiceName(), SERVICESTATUS[x].getStatus().getName());
                    }
                } else {
                    for (int x = 0; x < STOPPROCESSLIST.length; x++) {
                        result.put(STOPPROCESSLIST[x].getName(), "true");
                    }
                }

                return result;

                // throw new RuntimeException("Unrecognized methodName: '" + methodName + "'.");
            }
        };

        m_processContextImpl.setLocationsManager(m_locationsManager);

        m_methodNameStrings = new ArrayList();
        m_urlStrings = new ArrayList();
        m_paramVectors = new ArrayList();

        m_processContextImpl.setProcessModel(new SimpleSipxProcessModel());
    }

    public void testGetStatus() {
        ServiceStatus[] resultServiceStatus = m_processContextImpl.getStatus(m_locationsManager.getLocations()[0]);

        assertEquals(1, m_numberOfCalls);

        // Build the set of expected Process-ServiceStatus combinations. The order is not
        // important.
        Set<String> expectedCombinations = new HashSet<String>();
        for (int x = 0; x < SERVICESTATUS.length; x++) {
            expectedCombinations.add(SERVICESTATUS[x].getServiceName() + SERVICESTATUS[x].getStatus().getName());
        }

        // Compare the expected Process-ServiceStatus combinations to what actually occured.
        for (int x = 0; x < resultServiceStatus.length; x++) {
            String value = resultServiceStatus[x].getServiceName() + resultServiceStatus[x].getStatus().getName();
            assertTrue(expectedCombinations.remove(value));
        }
        assertEquals(0, expectedCombinations.size());
    }

    public void testManageService() {
        Location[] locations = {
            m_locationsManager.getLocations()[0]
        };
        Command command = Command.STOP;

        m_processContextImpl.manageServices(locations[0], Arrays.asList(STOPPROCESSLIST), command);

        checkManageServicesResults(STOPPROCESSLIST, locations, command);
    }

    public void testManageServices() {
        Location[] locations = m_locationsManager.getLocations();
        Command command = Command.STOP;

        m_processContextImpl.manageServices(Arrays.asList(STOPPROCESSLIST), command);

        checkManageServicesResults(STOPPROCESSLIST, locations, command);
    }

    public void testManageServicesRestart() {
        Location[] locations = m_locationsManager.getLocations();
        Command command = Command.RESTART;

        m_processContextImpl.manageServices(Arrays.asList(RESTARTPROCESSLIST), command);

        checkManageServicesResults(RESTARTPROCESSLIST, locations, command);

    }

    public void testManageServicesLocation() {
        Location[] locations = {
            m_locationsManager.getLocations()[1]
        };
        Command command = Command.START;

        m_processContextImpl.manageServices(locations[0], Arrays.asList(STARTPROCESSLIST), command);

        checkManageServicesResults(STARTPROCESSLIST, locations, command);
    }

    private void checkManageServicesResults(final Process[] PROCESSES, final Location[] LOCATIONS, final Command COMMAND) {
        assertEquals(LOCATIONS.length, m_numberOfCalls);

        for (int x = 0; x < m_numberOfCalls; x++) {
            assertEquals(COMMAND.getName(), m_methodNameStrings.get(x));
        }

        // Build the set of expected Process-Location combinations. The order is not important.
        Set<String> expectedCombinations = new HashSet<String>();
        for (int x = 0; x < PROCESSES.length; x++) {
            for (int y = 0; y < LOCATIONS.length; y++) {
                expectedCombinations.add(PROCESSES[x].getName() + LOCATIONS[y].getProcessMonitorUrl());
            }
        }

        Process[] Processes;
        if (COMMAND == Command.STOP) {
            Processes = STOPPROCESSLIST;
        } else if (COMMAND == Command.START) {
            Processes = STARTPROCESSLIST;
        } else if (COMMAND == Command.RESTART) {
            Processes = RESTARTPROCESSLIST;
        } else {
            Processes = STOPPROCESSLIST;
        }
        // Compare the expected Process-Location combinations to what actually occured.
        for (int y = 0; y < LOCATIONS.length; y++) {
            for (int x = 0; x < Processes.length; x++) {
                List ProcList = (List) ((Vector<Object>) m_paramVectors.get(y)).elementAt(0);
                String value = ProcList.get(x).toString() + m_urlStrings.get(y);
                assertTrue(expectedCombinations.remove(value));
            }
        }
        assertEquals(0, expectedCombinations.size());
    }
}
