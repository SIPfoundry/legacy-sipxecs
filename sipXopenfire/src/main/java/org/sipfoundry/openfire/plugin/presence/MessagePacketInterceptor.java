package org.sipfoundry.openfire.plugin.presence;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;
import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.methods.PutMethod;

import javax.net.ssl.HttpsURLConnection;

import org.apache.log4j.Logger;
import org.jivesoftware.openfire.interceptor.PacketInterceptor;
import org.xmpp.packet.Message;
import org.jivesoftware.openfire.XMPPServer;
import org.xmpp.packet.Packet;
import org.jivesoftware.openfire.session.Session;
import org.jivesoftware.openfire.interceptor.PacketRejectedException;


public class MessagePacketInterceptor implements PacketInterceptor
{
    private static Logger log = Logger.getLogger(MessagePacketInterceptor.class);
    private SipXOpenfirePlugin plugin;
    
    MessagePacketInterceptor( SipXOpenfirePlugin plugin )
    {
        this.plugin = plugin;
    }
    
    public void interceptPacket(Packet packet,
                         Session session,
                         boolean incoming,
                         boolean processed) throws PacketRejectedException
     {
        try
        {
            if( packet instanceof Message )
            {
                Message message = (Message)packet;
                if( message.getType() == Message.Type.chat )
                {
                    String chatText = message.getBody();
                    if( chatText != null )
                    {
                        if( incoming && processed )
                        {
                            log.debug("message is: " + chatText );
                            if( chatText.startsWith( "@call" ) || 
                                chatText.startsWith( "@xfer" ) )
                            {
                                // build the URI.
                                // First find out who is sending the message.  This will be the caller
                                log.debug("from : " + message.getFrom() );
                                log.debug("from node: " + message.getFrom().toBareJID() );
                                String fromSipId = plugin.getSipId( message.getFrom().toBareJID() );
                                String fromSipPassword = plugin.getSipPassword( message.getFrom().toBareJID() );
                                String toSipId   = plugin.getSipId( message.getTo().toBareJID() );
                                String toSipPassword = plugin.getSipPassword( message.getTo().toBareJID() );
                                log.debug( fromSipId + ":" + fromSipPassword + ":" + toSipId + ":" + toSipPassword );

                                if( chatText.startsWith( "@call" ) )
                                {
                                    if( fromSipId != null && fromSipPassword != null )
                                    {
                                        // check if there is a phone number after @call
                                        String[] result = chatText.split("\\s");
                                        String numberToCall;
                                        if( result.length >= 2 )
                                        {
                                            //TODO: turn this next block of code into a function
                                            String username =  result[1] + "@" + XMPPServer.getInstance().getServerInfo().getXMPPDomain();
                                            log.debug("call username is " + username );
                                            String calledSipId = null;
                                            try
                                            {
                                               calledSipId = plugin.getSipId( username );
                                            }
                                            catch( Exception e )
                                            {
                                                log.debug( "caught: " + e.getMessage()); 
                                            }

                                            if( calledSipId != null )
                                            {
                                                numberToCall = calledSipId.substring( 0, fromSipId.indexOf( '@' ) );
                                            }
                                            else
                                            {
                                                numberToCall = result[1];
                                            }
                                        }
                                        else
                                        {
                                            numberToCall = toSipId.substring( 0, fromSipId.indexOf( '@' ) );
                                        }
                                        String restCallCommand = buildRestCallCommand( fromSipId, fromSipPassword, numberToCall );
                                        sendRestRequest( restCallCommand );
                                    }
                                }
                                else if( chatText.startsWith( "@xfer" ) )
                                {
                                    if( toSipId != null && toSipPassword != null )
                                    {
                                        // check if there is a phone number after @call
                                        String[] result = chatText.split("\\s");
                                        String numberToCall;
                                        if( result.length >= 2 )
                                        {
                                            // figure out what that number is...
                                            // First, check if it is the name part of a JID on the system
                                            // that is mapped to a SIP ID.
                                            String username =  result[1] + "@" + XMPPServer.getInstance().getServerInfo().getXMPPDomain();
                                            log.debug("xfer username is " + username );
                                            String calledSipId = null;
                                            try
                                            {
                                                calledSipId = plugin.getSipId( username );
                                            }
                                            catch( Exception e )
                                            {
                                                log.debug( "caught: " + e.getMessage()); 
                                            }

                                            if( calledSipId != null )
                                            {
                                                numberToCall = calledSipId.substring( 0, fromSipId.indexOf( '@' ) );
                                            }
                                            else
                                            {
                                                numberToCall = result[1];
                                            }
                                        }
                                        else
                                        {
                                            numberToCall = toSipId.substring( 0, fromSipId.indexOf( '@' ) );
                                        }
                                        String restCallCommand = buildRestCallCommand( toSipId, toSipPassword, numberToCall );
                                        sendRestRequest( restCallCommand );
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        catch( Exception e )
        {
            log.debug("Caughtx: '" + e.getMessage() );
            e.printStackTrace(System.err);
        }
    }
         
    String buildRestCallCommand( String callerSipUri, String callerSipPassword, String calledNumber )
    {
        String restCallCommand = "https://" +
                                 callerSipUri.substring( 0, callerSipUri.indexOf( '@' ) ) +
                                  ":" +
                                  callerSipPassword +
                                  callerSipUri.substring( callerSipUri.indexOf( '@' ) ) +
                                  ":8443/sipxconfig/rest/call/" +
                                  calledNumber;
        log.debug( "rest call command is: " + restCallCommand );
        return restCallCommand;
    }
    
    void sendRestRequest( String url )
    {
      try{
        String command = "curl --insecure -X PUT " + url;
//        String command = "curl "+ ;
        log.debug( command );
        String line;
        Process p = Runtime.getRuntime().exec(command);
        log.debug( command );
        BufferedReader input =
          new BufferedReader
            (new InputStreamReader(p.getErrorStream()));
        while ((line = input.readLine()) != null) {
          log.debug( "curl:" + line );
        }
        input.close();
      }
      catch (Exception err) {
         log.debug("rest caught: " + err.getMessage() );
         err.printStackTrace();
      }    
    }
}
    
    
    
