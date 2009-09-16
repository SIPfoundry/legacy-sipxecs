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
import org.sipfoundry.sipxrest.RestServer;
import org.sipfoundry.sipxrest.SipHelper;

public class CallControllerRestlet extends Restlet {

    private static Logger logger = Logger.getLogger(CallControllerRestlet.class);

    private String getAddrSpec(String name) {
        if (name.indexOf("@") != -1) {
            return name;
        } else {
            return name + "@" + RestServer.getRestServerConfig().getSipxProxyDomain();
        }
    }

    public CallControllerRestlet(Context context) {
        super(context);
    }

    @Override
    public void handle(Request request, Response response) {
        try {
            Method httpMethod = request.getMethod();
            if (!httpMethod.equals(Method.POST) && !httpMethod.equals(Method.GET)) {
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

            if (callingParty == null || calledParty == null) {
                response.setStatus(Status.CLIENT_ERROR_BAD_REQUEST);
                return;
            }
            if (agentName == null) {
                agentName = callingParty;
            }
            if (method == null) {
                method = CallControllerParams.REFER;
            }

            UserCredentialHash credentials = RestServer.getAccountManager().getCredentialHash(
                    agentName);
            if (credentials == null) {
                logger.error("could not find credentials for agent " + agentName);
                response.setStatus(Status.CLIENT_ERROR_FORBIDDEN);
                return;
            }

            User agentUserRecord = RestServer.getAccountManager().getUser(agentName);
            String agentAddr = agentUserRecord.getIdentity();

            if (callingParty.indexOf("@") == -1) {
                callingParty = getAddrSpec(callingParty);
            }

            if (calledParty.indexOf("@") == -1) {
                calledParty = getAddrSpec(calledParty);
            }
            String key = agentAddr + ":" + callingParty + ":" + calledParty;

            logger.debug(String.format("method = %s key %s", httpMethod.toString(), key));

            if (httpMethod.equals(Method.POST)) {
                String fwdAllowed = (String) request.getAttributes().get(
                        CallControllerParams.FORWARDING_ALLOWED);
                boolean isForwardingAllowed = fwdAllowed == null ? false : Boolean
                        .parseBoolean(fwdAllowed);

                logger.debug("agentAddr = " + agentAddr);

                String subject = (String) request.getAttributes().get(
                        CallControllerParams.SUBJECT);

                int timeout = 180;
                if ((String) request.getAttributes().get(CallControllerParams.TIMEOUT) != null) {
                    timeout = Integer.parseInt((String) request.getAttributes().get(
                            CallControllerParams.TIMEOUT));
                }
                
                int cachetimeout = 180;
                
                if ((String) request.getAttributes().get(CallControllerParams.RESULTCACHETIME) != null) {
                    timeout = Integer.parseInt((String) request.getAttributes().get(
                            CallControllerParams.RESULTCACHETIME));
                }

                if (method.equals(CallControllerParams.REFER)) {
                    DialogContext dialogContext = SipUtils.createDialogContext(key, timeout, cachetimeout);
                    Dialog dialog = new SipServiceImpl().sendRefer(credentials, agentAddr,
                            agentUserRecord.getDisplayName(), callingParty, calledParty, subject,
                            isForwardingAllowed, dialogContext, timeout);

                    logger.debug("CallControllerRestlet : Dialog = " + dialog);

                } else {
                    response.setEntity("Method not supported " + key, MediaType.TEXT_PLAIN);
                    response.setStatus(Status.CLIENT_ERROR_NOT_ACCEPTABLE);

                    return;
                }

            } else {
                DialogContext dialogContext = SipUtils.getDialogContext(key);
                if (dialogContext == null) {
                    String emptyResponse = DialogContext.HEADER + DialogContext.FOOTER;
                    response.setEntity(emptyResponse, MediaType.TEXT_XML);
                    response.setStatus(Status.SUCCESS_OK);
                    return;
                } else {
                    logger.debug("status = " + dialogContext.getStatus());

                    response.setEntity(dialogContext.getStatus(), MediaType.TEXT_XML);
                    response.setStatus(Status.SUCCESS_OK);
                    return;
                }
            }

        } catch (Exception ex) {
            logger.error("An exception occured while processing the request. : ", ex);
            response.setEntity(ex.toString(), MediaType.TEXT_PLAIN);
            response.setStatus(Status.SERVER_ERROR_INTERNAL);
            return;

        }

    }

}
