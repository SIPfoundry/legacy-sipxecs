package org.sipfoundry.callcontroller;

import java.net.InetAddress;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;
import java.util.Random;

import javax.sip.address.Hop;
import javax.sip.address.SipURI;

import org.apache.log4j.Logger;
import org.restlet.Filter;
import org.restlet.data.ChallengeRequest;
import org.restlet.data.ChallengeResponse;
import org.restlet.data.ChallengeScheme;
import org.restlet.data.MediaType;
import org.restlet.data.Parameter;
import org.restlet.data.Protocol;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.util.Series;
import org.sipfoundry.commons.siprouter.FindSipServer;
import org.sipfoundry.commons.userdb.User;

import com.noelios.restlet.http.HttpResponse;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class CallControllerDigestAuthenticationFilter extends Filter {
    private static Logger logger = 
        Logger.getLogger(CallControllerDigestAuthenticationFilter.class);
    
    private static Random random = new Random();
    /**
     * Defined in rfc 2617 as KD(secret, data) = H(concat(secret, ":", data))
     * 
     * @param data data
     * @param secret secret
     * @return H(concat(secret, ":", data));
     */
    private static String KD(String secret, String data) {
        return H(secret + ":" + data);
    }

    /**
     * to hex converter
     */
    private static final char[] toHex = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    /**
     * Converts b[] to hex string.
     * 
     * @param b the bte array to convert
     * @return a Hex representation of b.
     */
    private static String toHexString(byte b[]) {
        int pos = 0;
        char[] c = new char[b.length * 2];
        for (int i = 0; i < b.length; i++) {
            c[pos++] = toHex[(b[i] >> 4) & 0x0F];
            c[pos++] = toHex[b[i] & 0x0f];
        }
        return new String(c);
    }
    
    /**
     * Defined in rfc 2617 as H(data) = MD5(data);
     * 
     * @param data data
     * @return MD5(data)
     */
    private static String H(String data) {
        try {
            MessageDigest digest = MessageDigest.getInstance("MD5");

            return toHexString(digest.digest(data.getBytes()));
        } catch (NoSuchAlgorithmException ex) {
            // shouldn't happen
            throw new RuntimeException("Failed to instantiate an MD5 algorithm", ex);
        }
    }
    @Override
    protected int beforeHandle(Request request, Response response) {
      String remoteAddr = request.getClientInfo().getAddress();
      int httpPort = request.getHostRef().getHostPort();
      String domain = request.getHostRef().getHostDomain();
      try {
          SipURI sipUri = SipStackBean.getInstance().getProxySipURI();
          
          logger.debug("Authentication request " + remoteAddr );
          Collection<Hop> hops = new FindSipServer(logger).getSipxProxyAddresses(sipUri);
          
          for (Hop hop : hops) {
              if (InetAddress.getByName(hop.getHost()).getHostAddress().equals(remoteAddr)) {               
                  if ( httpPort != CallController.getCallControllerConfig().getHttpPort() ) {
                      return Filter.STOP;
                  }
                  if (! request.getProtocol().equals(Protocol.HTTPS) && 
                          CallController.isSecure) {
                      logger.debug("Request from Proxy must be over HTTPS ");
                      return Filter.STOP;
                  }
                
                  logger.debug("Request from sipx domain");
                  return Filter.CONTINUE;
              }
          }         
          String agentName = ((String) request.getAttributes().get(CallControllerParams.AGENT));
          logger.debug("AgentName = " + agentName);
          if ( agentName == null ) {
              agentName = (String)request.getAttributes().get(CallControllerParams.CALLING_PARTY);
          }

          User user = CallController.getAccountManager().getUser(agentName);
          
         
          if (user == null) {
              logger.debug("User not found");
              response.setEntity("User Not found " + agentName, MediaType.TEXT_PLAIN);
              response.setStatus(Status.CLIENT_ERROR_FORBIDDEN);
              return Filter.STOP;
          }
          
          if ( user.getPintoken() == null ) {
              logger.debug("PINTOKEN is null" );
              return Filter.CONTINUE;
          }
          ChallengeResponse challengeResponse = request.getChallengeResponse();
            
          logger.debug("challengeResponse is " + challengeResponse);
          if ( challengeResponse == null ) {
              logger.debug("Requesting DIGEST credentials");
              ChallengeRequest challengeRequest = new ChallengeRequest(ChallengeScheme.HTTP_DIGEST,
                      CallController.getCallControllerConfig().getSipxProxyDomain());
              response.setChallengeRequest(challengeRequest);
              response.setStatus(Status.CLIENT_ERROR_UNAUTHORIZED);
                 
              return Filter.STOP;
          }
          Series<Parameter> parameters  = challengeResponse.getParameters();
          if ( parameters.isEmpty() ) {
              String nonce = H(Long.toString( Math.abs(random.nextLong())));
              ChallengeRequest challengeRequest = new ChallengeRequest(ChallengeScheme.HTTP_DIGEST,
                      CallController.getCallControllerConfig().getSipxProxyDomain());
              response.setChallengeRequest(challengeRequest);
              response.setStatus(Status.CLIENT_ERROR_UNAUTHORIZED);
        
              parameters.add("qop", "auth");
              parameters.add("nonce",nonce);
              parameters.add("algorithm","MD5");
              parameters.add("realm",  CallController.getCallControllerConfig().getSipxProxyDomain());
              challengeResponse.setCredentialComponents(parameters);
              logger.debug("sending DIGEST challenge");
              return Filter.STOP;
          }
         
           if ( parameters == null || parameters.isEmpty()) {
              logger.debug("Requesting DIGEST credentials");
              ChallengeRequest challengeRequest = new ChallengeRequest(ChallengeScheme.HTTP_DIGEST,
                      CallController.getCallControllerConfig().getSipxProxyDomain());
              response.setChallengeRequest(challengeRequest);
              response.setStatus(Status.CLIENT_ERROR_PROXY_AUTHENTIFICATION_REQUIRED);
              return Filter.STOP;
          }
          
          logger.debug("credentials = " + challengeResponse.getCredentials());
            
           
          String nonce = challengeResponse.getParameters().getFirstValue("nonce",true);
          String cnonce = challengeResponse.getParameters().getFirstValue("cnonce",true);
          String uri = challengeResponse.getParameters().getFirstValue("uri",true);
          String nc = challengeResponse.getParameters().getFirstValue("nc",true);
          String qop = challengeResponse.getParameters().getFirstValue("qop", true);
          String response_param = challengeResponse.getParameters().getFirstValue("response");
          String method = request.getMethod().getName();
          
          logger.debug(String.format("nonce %s cnonce %s uri %s nc %s qop %s response %s method %s",
                  nonce, cnonce,uri,nc,qop,response_param,method));
          String A2 = null;
          if (qop == null || qop.trim().length() == 0
                  || qop.trim().equalsIgnoreCase("auth")) {
              A2 = method + ":" + uri;
          } else {
              String entity_digest = response.getEntity().getDigest().toString();
              
              A2 = method + ":" + uri + ":" + entity_digest;
          }
          String pintoken = user.getPintoken();
          String expectedValue = null;

          if (cnonce != null && qop != null && nc != null
                  && (qop.equalsIgnoreCase("auth") || qop.equalsIgnoreCase("auth-int")))

          {
              expectedValue = KD(pintoken, nonce + ":" + nc + ":" + cnonce + ":"
                      + qop + ":" + H(A2));

          } else {
              expectedValue = KD(pintoken, nonce + ":" + H(A2));
          }
       
          if (expectedValue.equals(response_param)) {
              logger.debug("Digest authentication succeeded");
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
