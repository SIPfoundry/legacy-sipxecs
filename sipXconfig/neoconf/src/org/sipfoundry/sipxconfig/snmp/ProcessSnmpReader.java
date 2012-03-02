/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.snmp;

import static java.lang.String.format;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.commserver.ServiceStatus;
import org.snmp4j.CommunityTarget;
import org.snmp4j.PDU;
import org.snmp4j.Snmp;
import org.snmp4j.Target;
import org.snmp4j.TransportMapping;
import org.snmp4j.event.ResponseEvent;
import org.snmp4j.mp.SnmpConstants;
import org.snmp4j.security.SecurityLevel;
import org.snmp4j.smi.Address;
import org.snmp4j.smi.Counter32;
import org.snmp4j.smi.GenericAddress;
import org.snmp4j.smi.OID;
import org.snmp4j.smi.OctetString;
import org.snmp4j.smi.VariableBinding;
import org.snmp4j.transport.DefaultUdpTransportMapping;
import org.snmp4j.util.PDUFactory;
import org.snmp4j.util.TableListener;
import org.snmp4j.util.TableUtils;

/**
 * Tip: to read you can run
 *   snmpwalk -v 2c -c public localhost 1.3.6.1.4.1.7142.100.1
 */
public class ProcessSnmpReader {
    private static final int[] BASE_OID = new int[] {
        1, 3, 6, 1, 4, 1, 7142, 100, 1, 10
    };
    private static final OID GET = new OID(BASE_OID);

    private static final OID PROCESS_NAME = new OID(BASE_OID, new int[] {
        2
    });
    private static final OID PROCESS_COUNT = new OID(BASE_OID, new int[] {
        5
    });
    private static final Log LOG = LogFactory.getLog(ProcessSnmpReader.class);
    private List<ServiceStatus> m_statuses;

    public List<ServiceStatus> read(String address) throws IOException {
        m_statuses = new ArrayList<ServiceStatus>();
        LOG.info(format("Connecting to %s to get SNMP information", address));
        Address targetAddress = GenericAddress.parse(format("udp:%s/161", address));
        TransportMapping< ? > transport = new DefaultUdpTransportMapping();
        Snmp snmp = new Snmp(transport);
        transport.listen();

        OctetString community = new OctetString("public");

        CommunityTarget target = new CommunityTarget();
        target.setCommunity(community);
        target.setAddress(targetAddress);
        target.setRetries(1);
        target.setTimeout(5000);
        target.setVersion(SnmpConstants.version2c);
        target.setSecurityLevel(SecurityLevel.NOAUTH_NOPRIV);

        PDU pdu = new PDU();
        VariableBinding vb = new VariableBinding(GET);
        pdu.add(vb);
        pdu.setType(PDU.GETNEXT);

        ResponseEvent response = snmp.send(pdu, target);
        PDU responsePDU = response.getResponse();
        if (responsePDU == null) {
            throw new IOException("Timeout connecting to SNMP server");
        }
        readTable(snmp, target, vb);
        return m_statuses;
    }

    void readTable(Snmp snmp, Target target, VariableBinding vb) throws IOException {
        snmp.listen();

        TableUtils tableUtils = new TableUtils(snmp, new PDUFactory() {
            @Override
            public PDU createPDU(Target target) {
                return new PDU();
            }
        });
        tableUtils.setMaxNumRowsPerPDU(10);
        Counter32 counter = new Counter32();

        OID[] columns = new OID[1];
        columns[0] = vb.getOid();
        long startTime = System.nanoTime();
        synchronized (counter) {
            StatusReader rdr = new StatusReader();
            TableListener listener = new SnmpTableResultsParser(rdr);
            tableUtils.getDenseTable(target, columns, listener, counter, null, null);
            try {
                counter.wait(5000);
            } catch (InterruptedException ex) {
                Thread.currentThread().interrupt();
            }
            rdr.getStatuses();
        }
        LOG.info("Table received in " + (System.nanoTime() - startTime) / 1000000 + " milliseconds.");
        snmp.close();
    }

    public final class StatusReader implements SnmpTableWalker {
        private String[] m_names;
        private ServiceStatus.Status[] m_states;
        private int m_index;

        public void next(VariableBinding vb) {
            if (vb.getOid().startsWith(PROCESS_NAME)) {
                m_names[m_index] = vb.getVariable().toString();
            } else if (vb.getOid().startsWith(PROCESS_COUNT)) {
                if (vb.getVariable().toInt() == 0) {
                    m_states[m_index] = ServiceStatus.Status.ShutDown;
                } else {
                    m_states[m_index] = ServiceStatus.Status.Running;
                }
            }
            m_index++;
        }

        public void up() {
            if (m_names == null) {
                m_names = new String[m_index];
            }
            if (m_states == null) {
                m_states = new ServiceStatus.Status[m_index];
            }
            m_index = 0;
        }

        public void getStatuses() {
            if (m_names != null) {
                for (int i = 0; i < m_names.length; i++) {
                    ServiceStatus status = new ServiceStatus(m_names[i], m_states[i], false, false);
                    m_statuses.add(status);
                }
            }
        }
    }
}
