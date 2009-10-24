package org.sipfoundry.sipcallwatcher;

import java.lang.Exception;
import java.util.Collection;
import java.util.Map;
import java.util.HashMap;
import java.util.ArrayList;

import org.apache.log4j.Logger;
import org.sipfoundry.sipcallwatcher.MultipartMessages.MultipartMessage;
import org.sipfoundry.sipcallwatcher.MultipartMessages.MessagePart;

/**
 * Class that abstracts an RLMI multipart message typically received as 
 * message bodies of NOTIFYs for subscriptions to resource lists.
 */
public class RlmiMultipartMessage  extends MultipartMessage
{
    private static Logger logger = Logger.getLogger(RlmiMultipartMessage.class);    
    private RlmiMessagePart rlmiMessagePart = null;
    private boolean isFullState;
    private int     version;
    private static Collection<DialogInfoMessagePart.DialogInfo> emptyCollection 
                            = new ArrayList<DialogInfoMessagePart.DialogInfo>();
    private Map<String, DialogInfoMessagePart> dialogInfoMessagePartList 
                    = new HashMap<String, DialogInfoMessagePart>();
    
    public class ResourceDialogsDescriptor
    {
        private String resourceName;
        private boolean isFullState;
        private Collection<DialogInfoMessagePart.DialogInfo> dialogsList;

        ResourceDialogsDescriptor( String resourceName, 
                                   String stateAsString, 
                                   Collection<DialogInfoMessagePart.DialogInfo> dialogsList )
        {
            logger.debug("dialogsList = " + dialogsList);
            if( stateAsString.equals( "full") )
            {
                this.isFullState = true;
            }
            else
            {
                this.isFullState = false;
            }
            this.dialogsList = dialogsList;
            this.resourceName = resourceName;
        }

        public boolean isFullState() {
            return isFullState;
        }

        public Collection<DialogInfoMessagePart.DialogInfo> getDialogsList() {
            return dialogsList;
        }

        public String getResourceName() {
            return resourceName;
        }

        public void setResourceName(String resourceName) {
            this.resourceName = resourceName;
        }    
    }
    
    public RlmiMultipartMessage( String body, String boundary ) throws Exception
    {
        super( body, boundary, RlmiMessagePartFactory.CreateFactory() );
        logger.debug( "Processing new RLMI multpart message\n====================================");
        for( MessagePart messagePart : this.getMessagePartList() )
        {
            if( messagePart instanceof RlmiMessagePart )
            {
                rlmiMessagePart = (RlmiMessagePart)messagePart;
                isFullState = rlmiMessagePart.isFullState();
                version     = rlmiMessagePart.getVersion();
            }
            else if( messagePart instanceof DialogInfoMessagePart )
            {
                DialogInfoMessagePart dialogInfoPart = (DialogInfoMessagePart)messagePart;
                dialogInfoMessagePartList.put( dialogInfoPart.getContentId(), dialogInfoPart );
            }
            logger.debug( messagePart.toString() );
        }
        
        if( rlmiMessagePart == null )
        {
            throw new Exception("RLMI multipart message does not contain the required \"application/rlmi+xml\" part");
        }
    }
    
    public Collection<ResourceDialogsDescriptor> getUpdatedEntitiesStates()
    {
        Collection<ResourceDialogsDescriptor>stateUpdates = new ArrayList<ResourceDialogsDescriptor>();
        // walk through all the elements of the RLMI body message part
        for( RlmiMessagePart.ResourceInfo resourceInfo : rlmiMessagePart.getResourcesList() )
        {
            DialogInfoMessagePart dialogInfoMessagePart =
                getDialogInfoMessagePartForContentId( resourceInfo.getCid() );
            if( dialogInfoMessagePart != null )
            {
                Collection<DialogInfoMessagePart.DialogInfo> dialogsList;
                if( resourceInfo.getState().equals("active") )
                {
                    dialogsList = dialogInfoMessagePart.getDialogsList();
                }
                else
                {
                    // virtual subscription to resource is not active - provide an empty
                    // dialog list.
                    dialogsList = emptyCollection;
                }
                ResourceDialogsDescriptor desc = new ResourceDialogsDescriptor( 
                                                        resourceInfo.getUri(),
                                                        dialogInfoMessagePart.getState(),
                                                        dialogsList );
                stateUpdates.add( desc );
            }
            else
            {
                logger.warn("No DialogInfo message part found for resource '" + resourceInfo.getCid() + "'" );
            }
        }
        return stateUpdates;
    }

    private DialogInfoMessagePart getDialogInfoMessagePartForContentId( String contentId )
    {
        return dialogInfoMessagePartList.get( contentId );
    }

    public boolean isFullState() 
    {
        return isFullState;
    }

    public int getVersion() {
        return version;
    }
}
