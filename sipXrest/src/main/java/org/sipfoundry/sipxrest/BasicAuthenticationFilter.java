package org.sipfoundry.sipxrest;

import java.net.InetAddress;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Collection;

import javax.sip.address.Hop;
import javax.sip.address.SipURI;

import org.apache.commons.codec.digest.DigestUtils;
import org.apache.log4j.Logger;
import org.restlet.Filter;
import org.restlet.Restlet;
import org.restlet.data.ChallengeRequest;
import org.restlet.data.ChallengeResponse;
import org.restlet.data.ChallengeScheme;
import org.restlet.data.MediaType;
import org.restlet.data.Protocol;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.sipfoundry.commons.siprouter.FindSipServer;
import org.sipfoundry.commons.userdb.User;

public class BasicAuthenticationFilter extends Filter {
    
    private static Logger logger = Logger.getLogger(BasicAuthenticationFilter.class);
    
    private SipURI sipUri;


    private Plugin plugin;

  
    public BasicAuthenticationFilter(Plugin plugin) throws Exception {
        String proxyDomain = RestServer.getRestServerConfig().getSipxProxyDomain();
         this.sipUri = RestServer.getAddressFactory().createSipURI(null, proxyDomain);
         this.plugin = plugin;
    }
  
  
    
    @Override
    protected int beforeHandle(Request request, Response response) {
      String remoteAddr = request.getClientInfo().getAddress();  
      try {     
          
          logger.debug("Authentication request " + remoteAddr );
         
          if ( ! request.getProtocol().equals(Protocol.HTTPS)  ) {
              logger.debug("Request was not recieved over HTTPS protocol");
              return Filter.STOP;
          }
          
          Collection<Hop> hops = new FindSipServer(logger).getSipxProxyAddresses(sipUri);
          
          for (Hop hop : hops) {
              if (InetAddress.getByName(hop.getHost()).getHostAddress().equals(remoteAddr)) {
                  logger.debug("Authenticated request from sipx domain");
                  return Filter.CONTINUE;
              }
          }
        
          String agentName = plugin.getAgent(request);
       
          User user = RestServer.getAccountManager().getUser(agentName);
          
         
          if (user == null) {
              logger.debug("User not found");
              response.setEntity("User Not found " + agentName, MediaType.TEXT_PLAIN);
              response.setStatus(Status.CLIENT_ERROR_UNAUTHORIZED);
              return Filter.STOP;
          }
          
          ChallengeResponse challengeResponse = request.getChallengeResponse();
          char[] secret = challengeResponse.getSecret();
          if ( secret == null ) {
              logger.debug("Requesting BASIC credentials");
              ChallengeRequest challengeRequest = new ChallengeRequest(ChallengeScheme.HTTP_BASIC,
                      RestServer.getRestServerConfig().getSipxProxyDomain());
              response.setChallengeRequest(challengeRequest);
              response.setStatus(Status.CLIENT_ERROR_PROXY_AUTHENTIFICATION_REQUIRED);
              return Filter.STOP;
          }
          String passWord = new String(challengeResponse.getSecret());
          
          String userName = user.getUserName();
          logger.debug("userName = " + userName);

          String userDomainPassword =  userName +":" + 
          RestServer.getRestServerConfig().getSipxProxyDomain() + ":" +
          passWord;

          String hashVal = DigestUtils.md5Hex(userDomainPassword);
          logger.debug("pintoken " + user.getPintoken());
          logger.debug("hashval "  + hashVal);
          if (user.getPintoken() == null || hashVal.equals(user.getPintoken())) {
              return Filter.CONTINUE;
          } else {
              logger.debug("User not authenticated - token mismatch ");
              response.setEntity("pin mismatch", MediaType.TEXT_PLAIN);
              response.setStatus(Status.CLIENT_ERROR_UNAUTHORIZED);
              return Filter.STOP;
          }
      } catch (Exception ex) {
          logger.error("Exception in processing request", ex);
          response.setEntity("Processing Error " + ex.getMessage(), MediaType.TEXT_PLAIN);
          response.setStatus(Status.CLIENT_ERROR_UNAUTHORIZED);
          return Filter.STOP;
      } 
    }
    
    

}
