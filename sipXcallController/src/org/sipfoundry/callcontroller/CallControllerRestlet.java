/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
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

public class CallControllerRestlet extends Restlet {

    private static String CONF_BRIDGE_PIN_PARAM = "X-ConfPin";

    private static Logger logger = Logger.getLogger(CallControllerRestlet.class);

    private static String getAddrSpec(String name) {
        String addrSpec;
    	if (name.indexOf("@") != -1) {
            addrSpec = name;
        } else {
            addrSpec = name + "@" + RestServer.getRestServerConfig().getSipxProxyDomain();
        }

    	return addrSpec;
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

            logger.debug("sipMethod = " + method);

            String callingParty = (String) request.getAttributes().get(
                    CallControllerParams.CALLING_PARTY);
            String calledParty = (String) request.getAttributes().get(
                    CallControllerParams.CALLED_PARTY);

            if (callingParty == null || calledParty == null) {
                response
                        .setEntity(
                                ResultFormatter
                                        .formatError(Status.CLIENT_ERROR_BAD_REQUEST,
                                                "Missing a required parameter - need both callingParty and calledParty URL Parameters"),
                                MediaType.TEXT_XML);
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
                response.setEntity(ResultFormatter.formatError(Status.CLIENT_ERROR_FORBIDDEN,
                        "could not find credentials for " + agentName), MediaType.TEXT_XML);
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

            logger.debug(String.format("http method = %s key %s", httpMethod.toString(), key));

            String conferencePin = (String) request.getAttributes().get(
                    CallControllerParams.CONFERENCE_PIN);
            if (conferencePin != null) {
                calledParty += "?" + CONF_BRIDGE_PIN_PARAM + "=" + conferencePin;
            }

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
                if (timeout < 0) {
                    String result = ResultFormatter.formatError(true,
                            Status.CLIENT_ERROR_BAD_REQUEST.getCode(), "Bad parameter timeout = "
                                    + timeout);
                    response.setEntity(result, MediaType.TEXT_XML);
                    response.setStatus(Status.CLIENT_ERROR_BAD_REQUEST);
                    return;
                }
                int cachetimeout = 180;

                if ((String) request.getAttributes().get(CallControllerParams.RESULTCACHETIME) != null) {
                    cachetimeout = Integer.parseInt((String) request.getAttributes().get(
                            CallControllerParams.RESULTCACHETIME));
                }

                logger.debug("cachetimeout = " + cachetimeout);
                if (cachetimeout < 0) {
                    String result = ResultFormatter.formatError(true,
                            Status.CLIENT_ERROR_BAD_REQUEST.getCode(),
                            "Bad parameter resultCacheTime = " + cachetimeout);
                    response.setEntity(result, MediaType.TEXT_XML);
                    response.setStatus(Status.CLIENT_ERROR_BAD_REQUEST);
                    return;
                }

                String action = (String) request.getAttributes().get(CallControllerParams.ACTION);

                if ( action == null ) {
                    action = CallControllerParams.CALL;
                }

                if ( action.equals(CallControllerParams.CALL)) {
                    if (method.equalsIgnoreCase(CallControllerParams.REFER)) {
                        DialogContext dialogContext = SipUtils.createDialogContext(key, timeout,
                                cachetimeout, credentials,method,agentAddr,callingParty,calledParty);
                        Dialog dialog = new SipServiceImpl().sendRefer(credentials, agentAddr,
                                agentUserRecord.getDisplayName(), callingParty, calledParty, subject,
                                isForwardingAllowed, dialogContext, timeout);
                        logger.debug("CallControllerRestlet : Dialog = " + dialog);
                    } else if (method.equalsIgnoreCase(CallControllerParams.INVITE)) {
                        DialogContext dialogContext = SipUtils.getDialogContext(key);
                        if ( dialogContext != null  && dialogContext.isCallInProgress()) {
                            String result = ResultFormatter.formatError(true,
                                    Status.CLIENT_ERROR_FORBIDDEN.getCode(), "Call in progress "
                                    + method + " not supported");
                            response.setEntity(result, MediaType.TEXT_XML);
                            response.setStatus(Status.CLIENT_ERROR_NOT_ACCEPTABLE);
                            return;
                        }
                        dialogContext = SipUtils.createDialogContext(key, timeout,
                                cachetimeout, credentials,method,agentAddr,callingParty,calledParty);
                        Dialog dialog = new SipServiceImpl().sendInvite(credentials, agentAddr,
                                agentUserRecord.getDisplayName(), callingParty, calledParty, subject,
                                isForwardingAllowed, dialogContext, timeout);
                        logger.debug("CallControllerRestlet : Dialog = " + dialog);

                    } else {
                        String result = ResultFormatter.formatError(true,
                                Status.CLIENT_ERROR_NOT_ACCEPTABLE.getCode(), "Call Setup Method "
                                + method + " not supported");
                        response.setEntity(result, MediaType.TEXT_XML);
                        response.setStatus(Status.CLIENT_ERROR_NOT_ACCEPTABLE);
                        return;
                    }
                } else if (action.equals(CallControllerParams.TRANFSFER)) {
                    String target = (String) request.getAttributes().get(CallControllerParams.TARGET);
                    if ( target == null ) {
                        String result = ResultFormatter.formatError(true,
                                Status.CLIENT_ERROR_BAD_REQUEST.getCode(), "Need a transfer target parameter");
                        response.setEntity(result, MediaType.TEXT_XML);
                        response.setStatus(Status.CLIENT_ERROR_BAD_REQUEST);
                        return;
                    }
                    if ( target.indexOf("@") == -1) {
                        target = target + "@" + RestServer.getRestServerConfig().getSipxProxyDomain();
                    }
                    DialogContext dialogContext = SipUtils.getDialogContext(key);
                    if ( dialogContext == null || !dialogContext.isCallInProgress()) {
                        String result = ResultFormatter.formatError(true,
                                Status.CLIENT_ERROR_FORBIDDEN.getCode(), "Call not found.");
                        response.setEntity(result, MediaType.TEXT_XML);
                        response.setStatus(Status.CLIENT_ERROR_NOT_ACCEPTABLE);
                        return;
                    }
                    dialogContext.transferCaller(target);
                }

            } else {
                DialogContext dialogContext = SipUtils.getDialogContext(key);
                if (dialogContext == null) {
                	String emptyResponse = null;
                	if ( method.equalsIgnoreCase("refer")) {
                		emptyResponse = DialogContext.HEADER + DialogContext.FOOTER;
                	} else {
                		emptyResponse = DialogContext.DIALOGS + DialogContext.DIALOGS_FOOTER;
                	}
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
