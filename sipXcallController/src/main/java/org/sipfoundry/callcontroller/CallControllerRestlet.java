package org.sipfoundry.callcontroller;

import gov.nist.javax.sip.clientauthutils.UserCredentialHash;

import javax.sip.Dialog;

import org.apache.log4j.Logger;
import org.restlet.Context;
import org.restlet.Restlet;
import org.restlet.data.MediaType;
import org.restlet.data.Method;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.sipfoundry.commons.userdb.User;

public class CallControllerRestlet extends Restlet {

    private static Logger logger = Logger.getLogger(CallControllerRestlet.class);

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
        try {
            Method httpMethod = request.getMethod();
            if (!httpMethod.equals(Method.GET) && !httpMethod.equals(Method.PUT)) {
                response.setStatus(Status.CLIENT_ERROR_METHOD_NOT_ALLOWED);
                return;
            }

            /*
             * PUT is used to set up the call. GET is used to query the status of call setup.
             */
            String agentName = (String) request.getAttributes().get(CallControllerParams.AGENT);

            String method = (String) request.getAttributes().get(CallControllerParams.METHOD);
            String callingParty = (String) request.getAttributes().get(
                    CallControllerParams.CALLING_PARTY);
            String calledParty = (String) request.getAttributes().get(
                    CallControllerParams.CALLED_PARTY);
            
            if ( callingParty == null || calledParty == null ) {
                response.setStatus(Status.CLIENT_ERROR_BAD_REQUEST);
                return;
            }
            if(agentName == null ) {
                agentName = callingParty;
            }
            if ( method == null ) {
                method = CallControllerParams.REFER;
            }

            UserCredentialHash credentials = CallController.getAccountManager()
                    .getCredentialHash(agentName);
            if (credentials == null) {
                logger.error("could not find credentials for agent " + agentName);
                response.setStatus(Status.CLIENT_ERROR_FORBIDDEN);
                return;
            }

            User agentUserRecord = CallController.getAccountManager().getUser(agentName);
            String agentAddr = agentUserRecord.getIdentity();
            /*
             * If the caller is somebody in our domain then we need to find a record for him in
             * our databse.
             */
            if (callingParty == null || callingParty.indexOf("@") == -1) {
                callingParty = CallController.getAccountManager().getIdentity(callingParty);
                if (callingParty == null) {
                    logger.error("Cannot find calling party " + callingParty);
                    response.setStatus(Status.CLIENT_ERROR_NOT_FOUND);
                    return;
                }
            }

            if (calledParty.indexOf("@") == -1) {
                calledParty = getAddrSpec(calledParty);
            }
            String key = agentAddr + ":" + callingParty + ":" + calledParty;

            SipStackBean stackBean = SipStackBean.getInstance();

            if (httpMethod.equals(Method.PUT)) {
                String fwdAllowed = (String) request.getAttributes().get(
                        CallControllerParams.FORWARDING_ALLOWED);
                boolean isForwardingAllowed = fwdAllowed == null ? false : Boolean
                        .parseBoolean(fwdAllowed);

                logger.debug("agentAddr = " + agentAddr);

                String subject = (String) request.getAttributes().get(
                        CallControllerParams.SUBJECT);
                if (method.equals(CallControllerParams.REFER)) {
                    DialogContext dialogContext = stackBean.createDialogContext(key);
                    Dialog dialog = new SipServiceImpl(stackBean).sendRefer(credentials,
                            agentAddr, agentUserRecord.getDisplayName(), callingParty,
                            calledParty, subject, isForwardingAllowed);
                    dialog.setApplicationData(dialogContext);
                    logger.debug("CallControllerRestlet : Dialog = " + dialog);

                }
             
            } else {
                DialogContext dialogContext = SipStackBean.getInstance().getDialogContext(key);
                if (dialogContext == null) {
                    response.setStatus(Status.CLIENT_ERROR_NOT_FOUND);
                    response.setEntity("Could not find call setup record for " + key,
                            MediaType.TEXT_PLAIN);
                    return;
                }
                response.setEntity(dialogContext.getStatus(), MediaType.TEXT_PLAIN);
            }
            response.setStatus(Status.SUCCESS_OK);
        } catch (Exception ex) {
            logger.error("An exception occured while processing the request. : ", ex);
            response.setStatus(Status.SERVER_ERROR_INTERNAL);
            return;

        }

    }

}
