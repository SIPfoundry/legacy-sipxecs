/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxbridge;

import javax.sip.Dialog;
import javax.sip.ServerTransaction;
import javax.sip.message.Response;

/**
 * Store information that is specific to a Dialog. This is a temporary holding
 * place for dialog specific data.
 * 
 * @author M. Ranganathan
 * 
 */
class DialogApplicationData {

    /*
     * Whether or not to forward requests to the PeerDialogs of this dialog (
     * assuming the state of the Dialog is not TERMINATED).
     */
    boolean forwarding = true;

    /*
     * The Peer Dialog of this Dialog.
     */
    Dialog peerDialog;

    /*
     * The last response seen by the dialog.
     */
    Response lastResponse;

    /*
     * The B2BUA associated with the dialog.
     */
    BackToBackUserAgent backToBackUserAgent;

    /*
     * The replaced dialog (for Consultative XFer processing ).
     */
    Dialog replacedDialog;
    
    /*
     * The MOH server dialog
     */
    Dialog musicOnHoldDialog;
    

    DialogApplicationData(BackToBackUserAgent backToBackUserAgent) {
        this.backToBackUserAgent = backToBackUserAgent;
    }

}
