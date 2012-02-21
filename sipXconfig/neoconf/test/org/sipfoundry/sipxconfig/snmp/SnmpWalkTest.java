/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.snmp;

import java.io.IOException;
import java.util.List;

import org.junit.Test;
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
import org.snmp4j.smi.Variable;
import org.snmp4j.smi.VariableBinding;
import org.snmp4j.tools.console.SnmpRequest;
import org.snmp4j.transport.DefaultUdpTransportMapping;
import org.snmp4j.util.PDUFactory;
import org.snmp4j.util.TableEvent;
import org.snmp4j.util.TableListener;
import org.snmp4j.util.TableUtils;

public class SnmpWalkTest {

    // disabled
    @Test()
    public void console() {
        String cmd = "-c public -v 2c -Ot localhost 1.3.6.1.4.1.2021.2";
        String[] args = cmd.split(" ");
        SnmpRequest.main(args);
    }

    @Test()
    public void test() throws IOException {
        Address targetAddress = GenericAddress.parse("udp:127.0.0.1/161");
        TransportMapping< ? > transport = new DefaultUdpTransportMapping();
        Snmp snmp = new Snmp(transport);
//        USM usm = new USM(SecurityProtocols.getInstance(), new OctetString(MPv3.createLocalEngineID()), 0);
//        SecurityModels.getInstance().addSecurityModel(usm);
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
        String procOid = "1.3.6.1.4.1.2021.2";
        VariableBinding vb = new VariableBinding(new OID(procOid));
        pdu.add(vb);
        pdu.setType(PDU.GETNEXT);

        ResponseEvent response = snmp.send(pdu, target);
        PDU responsePDU = response.getResponse();
        if (responsePDU == null) {
            throw new IllegalStateException("Timeout connecting to SNMP server");
        }
        dump(snmp, target, vb);
    }

    static void dump(Snmp snmp, Target target, VariableBinding vb) throws IOException {
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
            TableListener listener = new TextTableListener();
            tableUtils.getDenseTable(target, columns, listener, counter, null, null);
            try {
                counter.wait(60000);
            } catch (InterruptedException ex) {
                Thread.currentThread().interrupt();
            }
        }
        System.out.println("Table received in " + (System.nanoTime() - startTime) / 1000000 + " milliseconds.");
        snmp.close();
    }

    public static class TextTableListener implements TableListener {
        private boolean finished;

        public void finished(TableEvent event) {
            System.out.println();
            System.out.println("Table walk completed with status " + event.getStatus() + ". Received "
                    + event.getUserObject() + " rows.");
            finished = true;
            synchronized (event.getUserObject()) {
                event.getUserObject().notify();
            }
        }

        public boolean next(TableEvent event) {
            System.out.println("Index = " + event.getIndex() + ":");
            for (VariableBinding vb : event.getColumns()) {
                System.out.println(vb);
                Variable v = vb.getVariable();
                System.out.println(v);
            }
            System.out.println();
            ((Counter32) event.getUserObject()).increment();
            return true;
        }

        public boolean isFinished() {
            return finished;
        }
    }
}
