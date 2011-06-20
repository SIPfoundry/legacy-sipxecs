/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipcallwatcher;

import java.util.LinkedList;
import java.util.List;
import java.util.Collection;
import org.apache.log4j.Logger;
import org.sipfoundry.sipcallwatcher.DialogInfoMessagePart.EndpointInfo;

/**
 * Encapsulates information about the active dialogs of a specific resource.
 * 
 */
class DialogInformation {
    private static Logger logger = Logger.getLogger(DialogInformation.class);

    /*
     * track info about active dialogs
     */
    private LinkedList<DialogInfoMessagePart.DialogInfo> activeDialogs = new LinkedList<DialogInfoMessagePart.DialogInfo>();
    /* state of a resource considering all its dialogs */
    private SipResourceState compoundState = SipResourceState.UNDETERMINED;
    /* internal Id used to track entries that have been updated */
    private int updateId = -1;
    private String resourceName;

    public DialogInformation(String resourceName) {
        logger.debug("creating dialog Information for " + resourceName);
        this.resourceName = resourceName;

    }

    public boolean updateDialogStates(boolean isFullState,
            Collection<DialogInfoMessagePart.DialogInfo> updatedDialogs, int updateId) {
        boolean hasStateChanged = false;
        this.updateId = updateId;
        if (updatedDialogs.size() == 0) {
            if (isFullState) {
                // Full state of the resource is an empty list of dialogs...
                // There are no dialogs associated with this resource so it
                // is clearly idle.
                activeDialogs.clear();
                if (!this.compoundState.equals(SipResourceState.IDLE)) {
                    hasStateChanged = true;
                    this.compoundState = SipResourceState.IDLE;
                }
            }
        } else {
            if (isFullState) {
                /*
                 * the list of dialogs is complete. We can replace all the dialogs we had with
                 * the ones supplied if any.
                 */
                activeDialogs.clear();
                hasStateChanged = true;
            }
            // update the dialogs we have with the ones supplied.
            for (DialogInfoMessagePart.DialogInfo dialogInfo : updatedDialogs) {
                // we only track 'active' dialogs
                if (!dialogInfo.getState().equals("terminated")) {
                    if( activeDialogs.contains( dialogInfo ) == false ) {
                        activeDialogs.addFirst( dialogInfo );
                        hasStateChanged = true;
                    }
                } else {
                    // dialog is terminated - discontinue its tracking
                    activeDialogs.remove(dialogInfo);
                    hasStateChanged = true;
                }
            }
            hasStateChanged |= computeCompoundState();
        }

        logger.debug("resource " + this.resourceName + " hasStateChanged = "
                + hasStateChanged + " (# of active dialogs = " + activeDialogs.size() + ")");
        return hasStateChanged;
    }

    /**
     * Computes the state of a resource considering all its dialogs. This method sets the
     * value of instance variable 'compoundState' to either BUSY or IDLE accordingly.
     * 
     * @return - true if compoundState got changed
     */
    private boolean computeCompoundState() {
        boolean hasStateChanged = false;
        SipResourceState newCompoundState;
        if (activeDialogs.size() > 0) {
            // at least one active dialog - that makes the resource busy.
            newCompoundState = SipResourceState.BUSY;
        } else {
            // no active dialogs - that makes the resource idle.
            newCompoundState = SipResourceState.IDLE;
        }
        // Set hasStateChanged flag to true and update the compound state
        // if a state change was detected.
        if (!this.compoundState.equals(newCompoundState)) {

            hasStateChanged = true;
            this.compoundState = newCompoundState;
            logger.debug("resource " + this.resourceName + " stateChanged ");
        }
        return hasStateChanged;
    }

    public SipResourceState getCompoundState() {
        return compoundState;
    }

    public EndpointInfo getActiveDialogRemoteInfo() {
        EndpointInfo requestedEndpointInfo = null;
        if( activeDialogs.size() != 0 ){
            requestedEndpointInfo = activeDialogs.getFirst().getRemoteInfo();
        }
        return requestedEndpointInfo;
    }
    
    public int getUpdateId() {
        return updateId;
    }
}
