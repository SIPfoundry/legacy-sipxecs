/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipcallwatcher;

import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import org.apache.log4j.Logger;
import org.sipfoundry.sipcallwatcher.DialogInfoMessagePart.EndpointInfo;

/**
 * Encapsulates the dialog information for all the resources being monitored.
 * 
 */
class ResourcesDialogInformation {
    private static Logger logger = Logger.getLogger(ResourcesDialogInformation.class);
    
    /* update counter - will be used to track the resources that have been updated */
    private int updateId = 1;
    /*
     * maps a resource name to DialogInformation object storing that resource's active dialogs
     * and overall state.
     */
    private Map<String, DialogInformation> resourcesDialogInfoMap = new HashMap<String, DialogInformation>();

    /**
     * Updates the state of all the resources based on the supplied dialog updates.
     * 
     * @param isFullState - was RLMI a full report or not?
     * @param dialogsUpdate - list of dialog info updates
     * @return - map for resources names that experienced a state change as the result of the
     *         update along with their new state.
     */
    public Map<String, SipResourceState> update(boolean isFullState,
            Collection<RlmiMultipartMessage.ResourceDialogsDescriptor> dialogsUpdate) {
        logger.debug("update : isFullState " + isFullState + " ndescriptors "
                + dialogsUpdate.size());
        Map<String, SipResourceState> updatedResources = new HashMap<String, SipResourceState>();

        // update the resources one-by-one
        for (RlmiMultipartMessage.ResourceDialogsDescriptor dialogsDesc : dialogsUpdate) {
            SipResourceState newCompoundState;
            if ((newCompoundState = update(dialogsDesc)) != null) {
                updatedResources.put(dialogsDesc.getResourceName(), newCompoundState);

            } else {
                logger.debug("newCompoundState is null");
            }
        }

        // check whether this is a full state report or not.
        if (isFullState) {
            // now, discover all the resources that did not get updated. These represent
            // the resources that are no longer part of the full report which suggests that
            // they are no longer part of the set of resources that we need to worry about.
            // Find them and take them out and report their state to be changed to
            // 'undetermined'
            // if it is not already that.

            final Iterator<String> mapIter = resourcesDialogInfoMap.keySet().iterator();
            // do not use enhanced for-loop since we remove elements from the map while
            // iterating through it
            while (mapIter.hasNext()) {
                String resourceName = mapIter.next();
                DialogInformation dialogInfo = resourcesDialogInfoMap.get(resourceName);
                if (dialogInfo.getUpdateId() < updateId) {
                    // this record did not get updated by the full report, remove it.
                    if (!dialogInfo.getCompoundState().equals(SipResourceState.UNDETERMINED)) {
                        updatedResources.put(resourceName, SipResourceState.UNDETERMINED);

                    }
                    mapIter.remove();
                }
            }
        }
        updateId++;

        logger.debug("updatedResources = " + updatedResources);
        return updatedResources;
    }

    /**
     * Updates the state of a specific resource based on the supplied information
     * 
     * @param dialogsDesc - describes the dialog(s) for a specific resource. Note: a dialog
     *        descriptor with an empty set indicates that no dialogs are associated with the
     *        resource.
     * @return - if update caused a state change then a ResourceState representing the
     *         resource's new state is returned, otherwise null.
     */
    public SipResourceState update(RlmiMultipartMessage.ResourceDialogsDescriptor dialogsDesc) {
        try {
            SipResourceState newState = null;

            // locate the record for that resource in our resourcesDialogInfoMap
            DialogInformation dialogInfo = resourcesDialogInfoMap.get(dialogsDesc
                    .getResourceName());
            if (dialogInfo == null) {
                dialogInfo = new DialogInformation(dialogsDesc.getResourceName());
                resourcesDialogInfoMap.put(dialogsDesc.getResourceName(), dialogInfo);
            }

            if (dialogInfo.updateDialogStates(dialogsDesc.isFullState(), dialogsDesc
                    .getDialogsList(), updateId)) {
                newState = dialogInfo.getCompoundState();
            }
            return newState;
        } catch (RuntimeException ex) {
            logger.error("Unexpected exception ", ex);
            throw ex;
        }
    }
    
    public EndpointInfo getRemoteInfoForActiveDialog( String resourceName )
    {
        EndpointInfo remoteInfo = null;

        // locate the record for that resource in our resourcesDialogInfoMap
        DialogInformation dialogInfo = resourcesDialogInfoMap.get(resourceName);
        if (dialogInfo != null) {
            remoteInfo = dialogInfo.getActiveDialogRemoteInfo();
        }
        return remoteInfo;
        
    }
}
