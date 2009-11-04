/*
 * 
 * 
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.sipxprovision.auto;

import java.io.DataOutputStream;
import java.io.IOException;
import java.net.URL;
import java.util.Date;
import java.util.HashMap;
import java.util.LinkedList;

import javax.net.ssl.HttpsURLConnection;

import org.apache.log4j.Logger;

import org.sipfoundry.sipxprovision.auto.Configuration;

/**
 * A queue that auto-provisions phones into sipXconfig from an internal thread.
 * 
 * @author Paul Mossman
 */
public class Queue implements Runnable {

    private static final Logger LOG = Logger.getLogger("Queue");

    protected static final short MAX_PHONES_PER_REST_CALL = 10; 
    
    public static final String ID_PREFIX = "ID: ";
    
    private Configuration m_config = null;

    protected Thread m_thread = null;

    private LinkedList<DetectedPhone> m_fifo = new LinkedList<DetectedPhone>();
    
    private boolean m_signalled = false;
    
    private boolean m_stop = true;
    
    public Queue(Configuration config) {
        m_config = config;
    }

    public void start() { 
        LOG.info("Start.");
        if (null == m_thread) {
            m_stop = false;
            m_signalled = false;
            m_thread = new Thread(this);
            m_thread.start();
        }
    }
    
    /**
     * Stops the queue, and waits forever for it's thread to die. 
     * <p>
     * Do not call this method from the queue thread.
     * 
     * @return true if the thread was stopped or did not exist, false otherwise.
     */
    public boolean stop() {
        LOG.info("Stop.");

        // Easy to handle when the thread isn't started.
        if (null == m_thread) {
            return true;
        }
        
        boolean result = false;

        // Tell the thread to stop.  (The 'm_signalled' is important, in case it is notified while
        // the thread is not waiting.)
        synchronized(m_fifo) {
            m_stop = true;
            m_signalled = true;
            m_fifo.notify();
        }
        
        // Wait for the thread to die.
        try {
            m_thread.join();

            if (!m_thread.isAlive()) {
                
                // The thread is dead.
                m_thread = null;

                // These entries will never be handled.
                LOG.info("  m_fifo.size(): " + m_fifo.size());
                m_fifo.clear();
                
                result = true;
            }
        }
        catch (InterruptedException e) {
            LOG.error("m_thread.join() interrupted:", e);
        }
        
        return result;
    }

    /**
     * Enqueue multiple phones for auto-provisioning into sipXconfig. 
     */
    public void addDetectedPhones(LinkedList<DetectedPhone> phones) {
        int new_size;
        synchronized(m_fifo) {
            m_fifo.addAll(phones); 
            new_size = m_fifo.size();
            m_signalled = true;
            m_fifo.notify();
        }
        phones.clear();
        LOG.debug("Enqueued: " + phones.size() + "   m_fifo.size: " + new_size);
    }

    /**
     * Enqueue a single phone for auto-provisioning into sipXconfig. 
     */
    public void addDetectedPhone(DetectedPhone phone) {
        int new_size;
        synchronized(m_fifo) {
            m_fifo.add(phone);
            new_size = m_fifo.size();
            m_signalled = true;
            m_fifo.notify();
        }
        LOG.debug("Enqueued one   m_fifo.size: " + new_size);
    }
    
    /**
     * Creates a (single-use) HTTPS connection to the sipXconfig REST Phones resource
     * server.
     * 
     *  @return the HTTPS connection, or null upon failure.
     */
    protected HttpsURLConnection createRestConnection() {
        LOG.debug("Creating connection: " + m_config.getConfigurationRestPhonesUri());
        
        HttpsURLConnection connection = null;
        try {
            URL url = new URL(m_config.getConfigurationRestPhonesUri());
            connection = (HttpsURLConnection) url.openConnection();

            // TODO --> org.apache.http.auth.UsernamePasswordCredentials ?????
            connection.setRequestProperty("Authorization", "Basic " + 
                    m_config.getBase64ConfigurationRestCredentials());
            connection.setDoOutput(true);
            connection.setDoInput(true);
            connection.setUseCaches(false);
            connection.setRequestMethod("POST");
            LOG.debug("Connection is good.");     
            
        } catch (Exception e) {
            LOG.error("Failed to create HttpsURLConnection:", e);
            connection =  null;
        }
        
        return connection;
    }

    /**
     * Provision the specified phones via the the sipXconfig REST Phones resource
     * server.
     * 
     *  @param phones the phones to be provisioned.
     */
    protected void provisionPhones(LinkedList<DetectedPhone> phones) {
        
        LOG.info("provisionPhones - " + phones.size());
        boolean success = false;
        

        HttpsURLConnection connection = createRestConnection();
        if (null != connection) {
            
            DataOutputStream dstream;
            try {
                // Write the REST representation of the phone(s).
                Date date = new Date(); 
                dstream = new java.io.DataOutputStream(connection.getOutputStream());
                dstream.writeBytes("<phones>");
                for (DetectedPhone phone : phones ) {

                    dstream.writeBytes(String.format("<phone><serialNumber>%s</serialNumber>" +
                       "<model>%s</model><description>%s</description></phone>", 
                       phone.mac, phone.model.sipxconfig_id, getPhoneDescription(phone, date)));
                }
                dstream.writeBytes("</phones>");
                
                // Do the HTTPS POST.
                dstream.close();

                // Record the result.  (Only transport, internal, etc. errors will be reported.
                // Duplicate and/or invalid MACs for example will result in 200 OK.)
                if (HttpsURLConnection.HTTP_OK == connection.getResponseCode()) {
                    success = true;
                    LOG.info("REST HTTPS POST success.");
                } else {
                    LOG.error("REST HTTPS POST failed:" + connection.getResponseCode() + " - " +
                            connection.getResponseMessage() );
                }

            } catch (IOException e) {
                LOG.error("REST HTTPS POST failed:", e);
            }
        }
        
        // Log configuration failures.
        if (!success) {
            for (DetectedPhone phone : phones ) {
                LOG.error(" - " + phone);
            }
        }
        
        // In either case, clear the phone data.
        phones.clear();
    }
    
    /**
     * Create a Description text for the specified phone.
     * 
     *  @see provisionPhones
     *  @return the string description.
     */
    private static String getPhoneDescription(DetectedPhone phone, Date date) {
        return String.format(
                "Auto-provisioned\n  %s%s\n  MAC: %s\n  Model: %s\n  Version: %s\n  Date: %s", 
                ID_PREFIX, phone.id, phone.mac, phone.model.full_label, phone.version, date);
    }

    public void run() {

        LOG.info("Thread run() START.");

        // Run until told to stop.
        LinkedList<DetectedPhone> phones = new LinkedList<DetectedPhone>();
        while(!m_stop) {
        
            synchronized(m_fifo) {
                try {
                    if (!m_signalled){
                        LOG.debug("Thread wait.  (Not signalled.)");
                        m_fifo.wait();
                        LOG.debug("Thread wake!");  
                    }

                    // Extract up to MAX_PHONES_PER_REST_CALL entries, if not stopping.
                    if (!m_stop) {

                        short num_phones = 0;
                        while (null != m_fifo.peek() && MAX_PHONES_PER_REST_CALL > num_phones) {

                            phones.add(m_fifo.poll());
                            num_phones++;
                        }
                        
                        LOG.debug("Dequeued: " + phones.size() + "   m_fifo.size: " + m_fifo.size());
                        
                        // If there are more phones queued, then don't wait again after handling these.
                        // (Important, because the next notify() may come after exiting the synchronized 
                        // block, but before this thread makes it's next wait() call.  Without this, )
                        if (null == m_fifo.peek()) {
                            m_signalled = false;
                        }
                    }              
                }
                catch (InterruptedException e) {
                    m_stop = true;
                    LOG.error("m_fifo.wait() interrupted:", e);
                }
            }

            // Attempt to provision the phones.  (This will be empty if stopping.)
            if (!phones.isEmpty()) {
                provisionPhones(phones);
            }
            
            try { Thread.sleep(m_config.getQueueDebugPostProvisionSleepTime()); } catch (InterruptedException e) { }
        }
        
        LOG.info("Thread run() STOP.");
    }

    private int size() {
        synchronized(m_fifo) {
            return m_fifo.size();
        }
    }
    
    public static class PhoneModel {
        PhoneModel(String id, String label) {
            sipxconfig_id = id;
            full_label = label;
        }
        public final String sipxconfig_id;
        public final String full_label;
    }

    public static class DetectedPhone {
        public PhoneModel model = new PhoneModel("unknown", "unknown");
        public String version = new String("unknown");
        public String mac = null;
        public String id = null;       
        public String toString() {
            return "id: " + id + "  mac: " + mac + "  version: " + version + 
                "  model: " + model.full_label;
        }
    }

    static public PhoneModel lookupPhoneModel(String index) {
        return PHONE_MODEL_MAP.get(index);
    }

    private static final HashMap<String, PhoneModel> PHONE_MODEL_MAP;
    static {
        PHONE_MODEL_MAP = new HashMap<String, PhoneModel>();
        
        // Polycom SoundPoing IP family, see:
        //  - http://sipx-wiki.calivia.com/index.php/Polycom_SoundPoint_IP_family_table
        //  - plugins/polycom/src/org/sipfoundry/sipxconfig/phone/polycom/polycom-models.beans.xml
        PHONE_MODEL_MAP.put("SPIP_300", new PhoneModel("polycom300", "SoundPoint IP 300"));
        PHONE_MODEL_MAP.put("SPIP_301", new PhoneModel("polycom300", "SoundPoint IP 301"));

        PHONE_MODEL_MAP.put("SPIP_320", new PhoneModel("polycom330", "SoundPoint IP 320"));
        PHONE_MODEL_MAP.put("SPIP_321", new PhoneModel("polycom330", "SoundPoint IP 321"));
        PHONE_MODEL_MAP.put("SPIP_330", new PhoneModel("polycom330", "SoundPoint IP 330"));
        PHONE_MODEL_MAP.put("SPIP_331", new PhoneModel("polycom330", "SoundPoint IP 331"));

        PHONE_MODEL_MAP.put("SPIP_430", new PhoneModel("polycom430", "SoundPoint IP 430"));
        PHONE_MODEL_MAP.put("SPIP_450", new PhoneModel("polycom450", "SoundPoint IP 450"));

        PHONE_MODEL_MAP.put("SPIP_500", new PhoneModel("polycom500", "SoundPoint IP 500"));
        PHONE_MODEL_MAP.put("SPIP_501", new PhoneModel("polycom500", "SoundPoint IP 501"));

        PHONE_MODEL_MAP.put("SPIP_550", new PhoneModel("polycom550", "SoundPoint IP 550"));
        PHONE_MODEL_MAP.put("SPIP_560", new PhoneModel("polycom550", "SoundPoint IP 560"));

        PHONE_MODEL_MAP.put("SPIP_600", new PhoneModel("polycom600", "SoundPoint IP 600"));
        PHONE_MODEL_MAP.put("SPIP_601", new PhoneModel("polycom600", "SoundPoint IP 601"));

        PHONE_MODEL_MAP.put("SPIP_650", new PhoneModel("polycom650", "SoundPoint IP 650"));
        PHONE_MODEL_MAP.put("SPIP_670", new PhoneModel("polycom650", "SoundPoint IP 670"));

        PHONE_MODEL_MAP.put("SSIP_4000", new PhoneModel("polycom4000", "SoundStation IP 4000"));

        PHONE_MODEL_MAP.put("SSIP_6000", new PhoneModel("polycom6000", "SoundStation IP 6000"));

        PHONE_MODEL_MAP.put("SSIP_7000", new PhoneModel("polycom7000", "SoundStation IP 7000"));

        PHONE_MODEL_MAP.put("VVX_1500", new PhoneModel("polycomVVX1500", "Polycom VVX 1500"));

        // Nortel IP 12x0, see:
        //  - http://sipx-wiki.calivia.com/index.php/Polycom_SoundPoint_IP_family_table
        //  - plugins/nortel12x0/src/org/sipfoundry/sipxconfig/phone/nortel12x0/nortel12x0-models.beans.xml
        PHONE_MODEL_MAP.put("1210", new PhoneModel("nortel-1210", "Nortel IP 1210"));
        PHONE_MODEL_MAP.put("1220", new PhoneModel("nortel-1220", "Nortel IP 1220"));
        PHONE_MODEL_MAP.put("1230", new PhoneModel("nortel12x0PhoneStandard", "Nortel IP 1230"));
    }    
    
    public static void main(String[] args) {

        Configuration config = new Configuration();
       
        // Send log4j output to the console.
        org.apache.log4j.BasicConfigurator.configure();
        
        //test1(config);
        test2(config);
    }
    
    public static void test1(Configuration config) {
        
        Queue queue = new Queue(config);

        queue.start();

        DetectedPhone phone = new DetectedPhone();
        phone.id = "FUN TIMES";
        phone.mac = "000000c0ffee";
        phone.model = lookupPhoneModel("SPIP_670");
        queue.addDetectedPhone(phone);
        
        while(0 != queue.size()) {
            try{ Thread.sleep(100); } catch (Exception e ) {}
        }
        
        if(!queue.stop()){
            LOG.debug("Error stopping queue.");
        }
        if(!queue.stop()){
            LOG.debug("Error stopping queue.");
        }
    }
    
    public static void test2(Configuration config) {
        
        Queue queue = new Queue(config) {
            protected void provisionPhones(LinkedList<DetectedPhone> phones) {
                
                LOG.debug("PROVISION: " + phones.size());
                for (DetectedPhone phone : phones ) {
                    LOG.debug("   " + phone);
                }
                LOG.debug("---\n");
                
                phones.clear();
            }
        };

        queue.start();

        DetectedPhone phone = null;
        int x;
        for( x = 0; x < 200; x++) {
            phone = new DetectedPhone();
            phone.id = new String(String.format("%d", 100 + x));
            queue.addDetectedPhone(phone);
        }

        for( x = 500; x < 1000; x++ ) {
            phone = new DetectedPhone();
            phone.id = new String(String.format("%d", 100 + x));
            
            queue.addDetectedPhone(phone);
            
            try{ Thread.sleep((new java.util.Random()).nextInt() % 20); } catch (Exception e ) {}
        }
        
        while(0 != queue.size()) {
            try{ Thread.sleep(100); } catch (Exception e ) {}
        }
        
        if(!queue.stop()){
            LOG.debug("Error stopping queue.");
        }
        if(!queue.stop()){
            LOG.debug("Error stopping queue.");
        }
    }
}



