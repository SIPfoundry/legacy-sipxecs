package org.sipfoundry.sipxbridge;

import java.io.IOException;

import javax.sdp.SdpParseException;
import javax.sdp.SessionDescription;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.SipException;
import javax.sip.message.Request;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxbridge.symmitron.Sym;

public class RtpSession extends Sym {

    private static Logger logger = Logger.getLogger(RtpSession.class);

    public RtpSession() {
        super();
    }

    @Override
    public RtpReceiverEndpoint getReceiver() {
        return (RtpReceiverEndpoint) super.getReceiver();
    }

    @Override
    public RtpTransmitterEndpoint getTransmitter() {
        return (RtpTransmitterEndpoint) super.getTransmitter();
    }

    protected void setTransmitter(RtpTransmitterEndpoint endpoint) {
        super.setTransmitter(endpoint);
    }

    protected void setReceiver(RtpReceiverEndpoint receiverEndpoint) {
        super.setReceiver(receiverEndpoint);
    }

    /**
     * Reassign the session parameters ( possibly putting the media on hold and playing music ).
     * 
     * @param sessionDescription
     * @param dat -- the dialog application data
     * @return -- the recomputed session description.
     */
    SessionDescription reAssignSessionParameters(Request request, Dialog dialog)
            throws SdpParseException, SipException {
        try {
            SessionDescription sessionDescription = SipUtilities.getSessionDescription(request);
            int oldPort = this.getTransmitter().getPort();
            String oldIpAddress = this.getTransmitter().getIpAddress();

            int newport = SipUtilities.getSessionDescriptionMediaPort(sessionDescription);
            String newIpAddress = SipUtilities
                    .getSessionDescriptionMediaIpAddress(sessionDescription);

            /*
             * Get the a media attribute -- CAUTION - this only takes care of the first media.
             * Question - what to do when only one media stream is put on hold?
             */

            String mediaAttribute = SipUtilities
                    .getSessionDescriptionMediaAttributeDuplexity(sessionDescription);

            String sessionAttribute = SipUtilities
                    .getSessionDescriptionAttribute(sessionDescription);

            if (logger.isDebugEnabled()) {
                logger.debug("mediaAttribute = " + mediaAttribute);
                logger.debug("sessionAttribute = " + sessionAttribute);
            }

            String attribute = sessionAttribute != null ? sessionAttribute : mediaAttribute;

            if (newIpAddress.equals("0.0.0.0") && newport == oldPort) {
                /*
                 * RFC2543 specified that placing a user on hold was accomplished by setting the
                 * connection address to 0.0.0.0. This has been deprecated, since it doesn't allow
                 * for RTCP to be used with held streams, and breaks with connection oriented
                 * media. However, a UA MUST be capable of receiving SDP with a connection address
                 * of 0.0.0.0, in which case it means that neither RTP nor RTCP should be sent to
                 * the peer.
                 */
                if (logger.isDebugEnabled()) {
                    logger.debug("setting media on hold " + this.toString());
                }
                this.getTransmitter().setOnHold(true);
                if (Gateway.getMusicOnHoldAddress() != null) {

                    /*
                     * For the standard MOH, the URI is defined to be <sip:~~mh~@[domain]>. There
                     * is thought that other URIs in the ~~mh~ series can be allocated
                     */
                    Dialog mohDialog;
                    try {
                        mohDialog = Gateway.getCallControlManager()
                                .getBackToBackUserAgent(dialog).sendInviteToMohServer(
                                        (SessionDescription) this.getReceiver()
                                                .getSessionDescription().clone());
                    } catch (CloneNotSupportedException e) {
                        throw new RuntimeException("Unexpected exception ", e);
                    }
                    DialogApplicationData dat = (DialogApplicationData) dialog
                            .getApplicationData();
                    dat.musicOnHoldDialog = mohDialog;

                }
                SipUtilities.setDuplexity(this.getReceiver().getSessionDescription(), "recvonly");
                SipUtilities.incrementSessionVersion(this.getReceiver().getSessionDescription());
                return this.getReceiver().getSessionDescription();
            } else if (newport == oldPort && oldIpAddress.equals(newIpAddress)) {
                if (attribute == null || attribute.equals("sendrecv")) {
                    logger.debug("Remove media on hold!");
                    SipUtilities.setDuplexity(this.getReceiver().getSessionDescription(),
                            "sendrecv");
                    SipUtilities.incrementSessionVersion(this.getReceiver().getSessionDescription());
                    this.getTransmitter().setOnHold(false);

                    DialogApplicationData dat = (DialogApplicationData) dialog
                            .getApplicationData();
                    if (dat.musicOnHoldDialog != null
                            && dat.musicOnHoldDialog.getState() != DialogState.TERMINATED) {
                        BackToBackUserAgent b2bua = dat.getBackToBackUserAgent();
                        b2bua.sendByeToMohServer(dat.musicOnHoldDialog);
                    }
                } else if (attribute != null && attribute.equals("sendonly")) {
                    logger.debug("Setting media on hold.");
                    this.getTransmitter().setOnHold(true);
                    /*
                     * Whenever the phone puts an external call on hold, it sends a re-INVITE to
                     * the gateway with "a=sendonly". Normally, the gateway would respond with
                     * "a=recvonly". However, if the gateway desires to generate MOH for the call,
                     * it can generate SDP specifying "a=inactive". To the phone, this makes it
                     * appear that the external end of the call has also put the call on hold, and
                     * it should cause the phone to not generate/obtain MOH media.
                     */
                    if (Gateway.getMusicOnHoldAddress() != null) {
                        Dialog mohDialog;
                        try {
                            mohDialog = Gateway.getCallControlManager().getBackToBackUserAgent(
                                    dialog).sendInviteToMohServer(
                                    (SessionDescription) this.getReceiver()
                                            .getSessionDescription().clone());
                        } catch (CloneNotSupportedException e) {
                            throw new RuntimeException("Unexpected exception ", e);
                        }
                        DialogApplicationData dat = (DialogApplicationData) dialog
                                .getApplicationData();
                        dat.musicOnHoldDialog = mohDialog;
                    }

                    SipUtilities.setDuplexity(this.getReceiver().getSessionDescription(),
                            "recvonly");

                    SipUtilities.incrementSessionVersion(this.getReceiver().getSessionDescription());
                } else if (attribute != null && attribute.equals("inactive")) {
                    logger.debug("Setting media on hold. -- saw an inactive Session attribute");
                    this.getTransmitter().setOnHold(true);

                    if (Gateway.getMusicOnHoldAddress() != null) {
                        Dialog mohDialog;
                        try {
                            mohDialog = Gateway.getCallControlManager().getBackToBackUserAgent(
                                    dialog).sendInviteToMohServer(
                                    (SessionDescription) this.getReceiver()
                                            .getSessionDescription().clone());
                        } catch (CloneNotSupportedException e) {
                            throw new RuntimeException("Unexpected exception ", e);
                        }
                        DialogApplicationData dat = (DialogApplicationData) dialog
                                .getApplicationData();
                        dat.musicOnHoldDialog = mohDialog;
                    }
                    SipUtilities.setDuplexity(this.getReceiver().getSessionDescription(),
                            "inactive");

                    SipUtilities.incrementSessionVersion(this.getReceiver().getSessionDescription());
                }
                return this.getReceiver().getSessionDescription();
            } else {
                if (logger.isDebugEnabled()) {
                    logger
                            .debug("Changing Session Parameters -- this is not yet supported oldIpAddress = "
                                    + oldIpAddress
                                    + " oldPort = "
                                    + oldPort
                                    + " newIp = "
                                    + newIpAddress + " newPort = " + newport);
                }

                SessionDescription retval = this.getReceiver().getSessionDescription();
                this.getTransmitter().setIpAddress(newIpAddress);
                this.getTransmitter().setPort(newport);
                this.getTransmitter().connect();
                return retval;
            }
        } catch (IOException ex) {
            throw new SipException("Exception occured while connecting", ex);
        }

    }
}
