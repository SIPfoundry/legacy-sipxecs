package org.sipfoundry.siptester;

import gov.nist.javax.sip.parser.StringMsgParser;

import java.util.Collection;
import java.util.concurrent.ConcurrentSkipListSet;

import org.apache.commons.digester.Digester;
import org.xml.sax.InputSource;

public class LogFileParser {
    
    public static String SIPTRACE = "sipTrace";
    public static String BRANCHNODE = "branchNode";
    public static String TIME = "time";
    public static String MESSAGE = "message";
    public static String SOURCE_ADDRESS = "sourceAddress";
    public static String DESTINATION_ADDRESS = "destinationAddress";
    public static String FRAME_ID = "frameId";
    
    
    private static void addRules(Digester digester) {
        digester.addObjectCreate(SIPTRACE, ConcurrentSkipListSet.class);
        digester.addObjectCreate(String.format("%s/%s", SIPTRACE,BRANCHNODE), CapturedLogPacket.class);
        digester.addSetNext(String.format("%s/%s", SIPTRACE,BRANCHNODE),"add");
        digester.addCallMethod(String.format("%s/%s/%s",SIPTRACE,BRANCHNODE,TIME),"setTime",0);
        digester.addCallMethod(String.format("%s/%s/%s", SIPTRACE,BRANCHNODE,MESSAGE), "setMessage", 0 ); 
        digester.addCallMethod(String.format("%s/%s/%s",SIPTRACE,BRANCHNODE,SOURCE_ADDRESS ), "setSourceAddress",0 );
        digester.addCallMethod(String.format("%s/%s/%s",SIPTRACE,BRANCHNODE,DESTINATION_ADDRESS ), "setDestinationAddress",0 );
        digester.addCallMethod(String.format("%s/%s/%s",SIPTRACE,BRANCHNODE,FRAME_ID ), "setFrameId",0);
    }
   
    public Collection<CapturedLogPacket> parse(String url) {
        Digester digester = new Digester();
        StringMsgParser.setComputeContentLengthFromMessage(true);
        addRules(digester);
        InputSource inputSource = new InputSource(url);
        try {
            digester.parse(inputSource);
        } catch (Exception e) {
            throw new SipTesterException(e);
        }
        StringMsgParser.setComputeContentLengthFromMessage(false);
        return (Collection<CapturedLogPacket>) digester.getRoot();
        
       
        
    }
}
