/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxprovision.auto;

import java.util.LinkedList;

import javax.net.ssl.HttpsURLConnection;

import org.sipfoundry.sipxprovision.auto.Queue.DetectedPhone;

import junit.framework.TestCase;

/**
 * Tests for the Queue class.
 *
 * @see Queue
 *
 * @author Paul Mossman
 */
public class QueueTest extends TestCase {

    private Queue.DetectedPhone buildPhone(String id, String mac, String model) {

        Queue.DetectedPhone phone = new Queue.DetectedPhone();
        phone.id = id;
        phone.mac = mac;
        phone.model = Queue.lookupPhoneModel(model);

        return phone;
    }

    public void testLookupPhoneModelFailure() {

        assertEquals(null, Queue.lookupPhoneModel("nope"));
    }

    class RestDisabledQueue extends Queue {

        RestDisabledQueue() {
            super(null);
        }

        protected HttpsURLConnection createRestConnection() {
            return null;
        }

        public Thread getThread() {
            return m_thread;
        }
    }

    public void testThreadStartingAndStopping() {

        RestDisabledQueue queue = new RestDisabledQueue();

        // Stopped by default (with no thread.)
        assertNull(queue.getThread());

        // Stopping is easy to handle when the thread isn't started.
        assertTrue(queue.stop());
        assertNull(queue.getThread());

        // First start.
        queue.start();
        Thread thread = queue.getThread();
        assertNotNull(thread);

        // First (actual) stop.
        assertTrue(queue.stop());
        assertNull(queue.getThread());

        // No-op stop.
        assertTrue(queue.stop());
        assertNull(queue.getThread());

        // Second (actual) start.
        queue.start();
        thread = queue.getThread();
        assertNotNull(thread);

        // No-op start.
        queue.start();
        assertEquals(thread, queue.getThread());

        // Final stop.
        assertTrue(queue.stop());
        assertNull(queue.getThread());
    }

    public void testProvisionPhonesClearsInputParameter() {

        class TestQueue extends RestDisabledQueue {
            public void start() {
                // No need to start the thread.
            }
            public void publicProvisionPhones(LinkedList<Queue.DetectedPhone> phones) {
                provisionPhones(phones);
            }
        }

        LinkedList<Queue.DetectedPhone> phones = new LinkedList<Queue.DetectedPhone>();
        phones.add(buildPhone("ID", "000000c0ffee", "SPIP_670"));

        (new TestQueue()).publicProvisionPhones(phones);

        assertEquals(0, phones.size());
    }

    class ProvisionCollectorDisabledQueue extends RestDisabledQueue {

        public LinkedList<DetectedPhone> m_CollectorPhones = null;

        public boolean m_CollectorSignalled = false;

        ProvisionCollectorDisabledQueue() {
            m_CollectorPhones = new LinkedList<DetectedPhone>();
        }

        protected void provisionPhones(LinkedList<DetectedPhone> phones) {
            synchronized(m_CollectorPhones) {
                m_CollectorPhones.addAll(phones);
                m_CollectorSignalled = true;
                m_CollectorPhones.notify();
            }
        }

        public short getMaxPhonesPerProvisionAttempt() {
            return MAX_PHONES_PER_REST_CALL;
        }
    }

    public void testAddSinglePhoneResultsInProvisionAttempt() throws InterruptedException {

        ProvisionCollectorDisabledQueue queue = new ProvisionCollectorDisabledQueue();
        queue.start();

        String id = "ID1";
        queue.addDetectedPhone(buildPhone(id, "000000c0ffee", "SPIP_670"));

        synchronized(queue.m_CollectorPhones) {

            if (!queue.m_CollectorSignalled){
                queue.m_CollectorPhones.wait(30 * 1000);
            }

            assertEquals(1, queue.m_CollectorPhones.size());
            assertEquals(id, queue.m_CollectorPhones.getFirst().id);
        }
    }


    public void testAddMultiplePhonesResultsInProvisionAttempt() throws InterruptedException {

        ProvisionCollectorDisabledQueue queue = new ProvisionCollectorDisabledQueue();
        queue.start();

        LinkedList<DetectedPhone> phones = new LinkedList<DetectedPhone>();
        for (int x = 0; x < queue.getMaxPhonesPerProvisionAttempt(); x++) {
            phones.add(buildPhone("ID" + x, "000000c0ffee", "SPIP_670"));
        }
        queue.addDetectedPhones(phones);

        synchronized(queue.m_CollectorPhones) {

            if (!queue.m_CollectorSignalled){
                queue.m_CollectorPhones.wait(30 * 1000);
            }

            assertEquals(queue.getMaxPhonesPerProvisionAttempt(), queue.m_CollectorPhones.size());
        }
    }

    // TODO: test thread auto-start, stop, stop (while stopped), re-start, stop (after re-start)

}


