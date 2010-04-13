/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import javax.sdp.SessionDescription;


/**
 * An internal class that is used to wait for an SDP answer after sending out an
 * SDP request.
 *
 * @author M. Ranganathan.
 */

class PendingSdpAnswer {

    /*
     * The outgoing rtp session ( this is fixed up when we get an sdp response).
     */

    //Sym outgoingSession;

    /*
     * The Sdp Offer that was sent to us from the Client Tx.
     */
    SessionDescription incomingSdpOffer;

    /*
     * The Sdp offer that we sent out
     */
    SessionDescription outgoingSdpOffer;

}
