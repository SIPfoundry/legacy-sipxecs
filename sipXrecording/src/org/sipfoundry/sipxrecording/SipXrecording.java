/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrecording;

import java.net.Socket;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import org.sipfoundry.commons.confdb.ConfReadConverter;
import org.sipfoundry.commons.confdb.Conference;
import org.sipfoundry.commons.confdb.ConferenceService;
import org.sipfoundry.commons.confdb.ConferenceServiceImpl;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.sipfoundry.commons.mongo.MongoFactory;
import org.sipfoundry.conference.ConfRecordThread;
import org.sipfoundry.conference.ConferenceContextImpl;
import org.sipfoundry.conference.WebServer;
import org.springframework.core.convert.converter.Converter;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.convert.CustomConversions;
import org.springframework.data.mongodb.core.convert.MappingMongoConverter;

import com.mongodb.DBObject;
import com.mongodb.Mongo;

public class SipXrecording implements Runnable {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxrecording");
    private static RecordingConfiguration s_config;

    private Socket m_clientSocket;
    private static ConferenceService m_conferenceService;

    public SipXrecording(Socket client) {
        m_clientSocket = client;
    }

    /**
     * Determine what to do based on the SIP request.
     *
     * Run the autoattendant if action=autoattendant
     * Otherwise, hang up.
     */
    @Override
	public void run() {
        LOG.debug("SipXrecording::run Starting SipXrecording thread with client " + m_clientSocket);
        LOG.debug("SipXrecording::run Ending SipXrecording thread with client " + m_clientSocket);
    }


    /**
     * Load the configuration from the sipxrecording.properties file.
     *
     * @throws Throwable
     */
    static void init() throws Throwable {
        // Load the configuration
        s_config = RecordingConfiguration.get();

        // Configure log4j
        Properties props = new Properties();
        props.setProperty("log4j.rootLogger", "warn, file");
        props.setProperty("log4j.logger.org.sipfoundry.sipxrecording", SipFoundryLayout
                .mapSipFoundry2log4j(s_config.getLogLevel()).toString());
        props.setProperty("log4j.appender.file", "org.sipfoundry.commons.log4j.SipFoundryAppender");
        props.setProperty("log4j.appender.file.File", s_config.getLogFile());
        props.setProperty("log4j.appender.file.layout", "org.sipfoundry.commons.log4j.SipFoundryLayout");
        props.setProperty("log4j.appender.file.layout.facility", "sipXrecording");
        PropertyConfigurator.configure(props);

        initConferenceService();
        ConferenceContextImpl.getInstance().setConferenceService(m_conferenceService);
        ConferenceContextImpl.getInstance().setRecordingConfig(s_config);

        // Start conference recording task
        ConfRecordThread confThread = new ConfRecordThread(s_config);
        confThread.setConferenceContext(ConferenceContextImpl.getInstance());
        confThread.start();
        // Start the Jetty Server
        WebServer.getInstance().startServer();
    }

    private static void initConferenceService() throws Exception {
        Mongo mongo = MongoFactory.fromConnectionFile();
        List<Converter<DBObject, Conference>> converters = new ArrayList<Converter<DBObject, Conference>>();
        ConfReadConverter confReadConverter = new ConfReadConverter();
        converters.add(confReadConverter);
        CustomConversions cc = new CustomConversions(converters);
        MongoTemplate entityDb = new MongoTemplate(mongo, "imdb");
        MappingMongoConverter mappingConverter = (MappingMongoConverter)entityDb.getConverter();
        mappingConverter.setCustomConversions(cc);
        mappingConverter.afterPropertiesSet();
        m_conferenceService = new ConferenceServiceImpl();
        ((ConferenceServiceImpl) m_conferenceService).setTemplate(entityDb);
    }

    /**
     * Main entry point for sipXrecording
     * @param args
     */
    public static void main(String[] args) {
        try {
            init();
        } catch (Exception e) {
            LOG.fatal(e,e);
            e.printStackTrace();
            System.exit(1);
        } catch (Throwable t) {
            LOG.fatal(t,t);
            t.printStackTrace();
            System.exit(1);
        }
    }
}
