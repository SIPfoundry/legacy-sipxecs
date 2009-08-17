package org.sipfoundry.callcontroller;

import gov.nist.javax.sip.clientauthutils.UserCredentialHash;

import org.apache.log4j.Logger;
import org.restlet.Context;
import org.restlet.Restlet;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.StringRepresentation;
import org.restlet.resource.Variant;
import org.sipfoundry.commons.userdb.User;

public class CallControllerRestlet extends Restlet {

    private static Logger logger = Logger.getLogger(CallControllerRestlet.class);

    private String callingParty;
    private String agentName;
    private String calledParty;
    private String method;
    private boolean isForwardingAllowed;

    private UserCredentialHash credentials;

    private String getAddrSpec(String name) {
        if (name.indexOf("@") != -1) {
            return name;
        } else {
            return name + "@" + CallController.getCallControllerConfig().getSipxProxyDomain();
        }
    }

    public CallControllerRestlet(Context context) {
        super(context);
    }

    

    @Override
    public void handle(Request request, Response response) {
      

        this.agentName = (String) request.getAttributes().get(CallControllerParams.AGENT);
        this.method = (String) request.getAttributes().get(CallControllerParams.METHOD);
        this.callingParty = (String) request.getAttributes().get(
                CallControllerParams.CALLING_PARTY);
        if ( this.callingParty == null ) {
            this.callingParty = agentName;
        }
        this.calledParty = (String) request.getAttributes()
                .get(CallControllerParams.CALLED_PARTY);
        String fwdAllowed = (String) request.getAttributes().get(
                CallControllerParams.FORWARDING_ALLOWED);
        this.isForwardingAllowed = fwdAllowed == null ? false : Boolean.parseBoolean(fwdAllowed);

        if (agentName == null || agentName.indexOf("@") != -1 || calledParty == null
                || (!method.equals("refer") && !method.equals("invite"))) {
            logger.error("Error processing request -- missing parameters ");
            response.setStatus(Status.CLIENT_ERROR_BAD_REQUEST);
            return;
        }

        this.credentials = CallController.getAccountManager().getCredentialHash(agentName);
        if (credentials == null) {
            logger.error("could not find credentials for agent " + agentName);
            response.setStatus(Status.CLIENT_ERROR_FORBIDDEN);
            return;
        }

        if (callingParty == null || callingParty.indexOf("@") == -1) {
            this.callingParty =  CallController.getAccountManager().getIdentity(callingParty);
            if ( this.callingParty == null ) {
                logger.error("Cannot find calling party " + callingParty);
                response.setStatus(Status.CLIENT_ERROR_NOT_FOUND);
                return;
            }
        }

        if (calledParty.indexOf("@") == -1) {
            calledParty = getAddrSpec(calledParty);
        }

      
        User agentUserRecord = CallController.getAccountManager().getUser(agentName);
        String agentAddr = agentUserRecord.getIdentity();   
        logger.debug("agentAddr = " + agentAddr);

        String subject = (String) request.getAttributes().get(CallControllerParams.SUBJECT);
        
        try {
            SipStackBean stackBean = SipStackBean.getInstance();
            if (this.method.equals(CallControllerParams.REFER)) {
                new SipServiceImpl(stackBean).sendRefer(this.credentials, agentAddr, agentUserRecord
                        .getDisplayName(), callingParty, calledParty,
                         subject, isForwardingAllowed);
            }

        } catch (Exception ex) {
            logger.error("An exception occured while processing the request. : ",  ex);
            response.setStatus(Status.CLIENT_ERROR_BAD_REQUEST);
            return;

        }
        response.setStatus(Status.SUCCESS_OK);

    }

}
