/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.siptester;

import gov.nist.javax.sip.parser.StringMsgParser;

import java.io.File;
import java.util.Collection;
import java.util.LinkedList;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;


public class LogFileParser {
    
    private static Logger logger = Logger.getLogger(LogFileParser.class);
    
    public static String SIPTRACE = "sipTrace";
    public static String BRANCHNODE = "branchNode";
    public static String TIME = "time";
    public static String MESSAGE = "message";
    public static String SOURCE_ADDRESS = "sourceAddress";
    public static String DESTINATION_ADDRESS = "destinationAddress";
    public static String FRAME_ID = "frameId";
    public static String REMOTE_HOST_PORT = "remoteHostPort";
    public static String IS_OUTGOING = "isOutgoing";
    
    LinkedList<CapturedLogPacket> retval = new LinkedList<CapturedLogPacket>();
    
    
    public Collection<CapturedLogPacket> parseXml( String traceFile  ) {
      try {
        StringMsgParser.setComputeContentLengthFromMessage(true);
        File logFile = new File(traceFile);
        DocumentBuilder builder;
        builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
        Document traceFileDocument = builder.parse(logFile);
        walkXml(traceFileDocument);
        logger.debug("read " + retval.size());
        StringMsgParser.setComputeContentLengthFromMessage(false);      
        return retval;
      } catch ( Exception ex) {
          SipTester.fail("Unexpected exception parsing trae file",ex);
          throw new SipTesterException(ex);
      }
 
        
    }
    private void walkXml(Document traceFileDocument) {
      NodeList branchNodes = traceFileDocument.getElementsByTagName(BRANCHNODE);
      logger.debug("nrecords = " + branchNodes.getLength());
      for ( int i = 0; i < branchNodes.getLength(); i ++ ) {
          Node branchNode = branchNodes.item(i);
          Node next = branchNode.getFirstChild();
          CapturedLogPacket capturedPacket = new CapturedLogPacket();
          
          while ( next != null ) {
              if (next.getNodeType() == Node.ELEMENT_NODE) {
                   String name = next.getNodeName();
                  String text = next.getTextContent();
                  if ( name.equals(TIME)) {
                      capturedPacket.setTime(text);
                  } else if ( name.equals(MESSAGE)) {
                       capturedPacket.setMessage(text);
                  } else if ( name.equals(SOURCE_ADDRESS) ) {
                      capturedPacket.setSourceAddress(text);
                  } else if ( name.equals(DESTINATION_ADDRESS)) {
                      capturedPacket.setDestinationAddress(text);
                  } else if (name.equals(FRAME_ID)) {
                      capturedPacket.setFrameId(text);
                   } else if ( name.equals(REMOTE_HOST_PORT)) {
                      capturedPacket.setRemoteHostPort(text);
                  } else if ( name.equals(IS_OUTGOING)) {
                      capturedPacket.setOutbound(text);
                  }
                
              }
              next = next.getNextSibling();
          }
          capturedPacket.map();
          retval.add(capturedPacket);
          SipTester.addCapturedPacket(capturedPacket);

      }
        
    }
    
}
