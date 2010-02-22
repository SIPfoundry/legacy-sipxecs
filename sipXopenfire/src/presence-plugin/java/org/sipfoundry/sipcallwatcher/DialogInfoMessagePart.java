package org.sipfoundry.sipcallwatcher;

import org.apache.log4j.Logger;
import org.sipfoundry.openfire.config.XmppGroup;
import org.sipfoundry.sipcallwatcher.MultipartMessages.MessagePart;
import org.xml.sax.InputSource;
import java.io.ByteArrayInputStream;
import java.util.ArrayList;

import javax.xml.xpath.*;

public class DialogInfoMessagePart extends MessagePart 
{
    /* example of a Dialog Info message Part
     </dialog-info>
      <dialog id=\"wpf89Mza531937,VMqJbf,12119508-ABAA659F;id68366dbf\" call-id=\"f88dfabce84b6a2787ef024a7dbe8749-reniatniamtan\" local-tag=\"C7FB5316-7F6E152D\" remote-tag=\"30543f3483e1cb11ecb40866edd3295b-reniatniamtan\" direction=\"recipient\">
        <state>confirmed</state>
        <local>
          <target uri=\"sip:anonymous@anonymous.invalid\">
            <param pname=\"+sip.rendering\" pval=\"yes\" />
          </target>
          <identity display=\"2176\">
            sip:2176@scstrial.ca;sipx-noroute=VoiceMail;sipx-userforward=false</identity>
        </local>
        <remote>
          <identity display=\"SipXecs Keepalive\">
               sip:anonymous@anonymous.invalid
          </identity>
          <target uri=\"sip:anonymous@anonymous.invalid\" />
        </remote>
      </dialog>
    </dialog-info>
    */
    
    private static Logger logger = Logger.getLogger(DialogInfoMessagePart.class);
    private ArrayList<DialogInfo> dialogsList = new ArrayList<DialogInfo>();
    private String entity;
    private String state;

    public class EndpointInfo
    {
        String displayName;
        String identity;
        String targetUri;
        // ^^^NOTE: extend equals() override when adding new members

        public EndpointInfo( String displayName, String identity,
                String targetUri )
        {
            this.displayName = displayName;
            this.identity = identity;
            this.targetUri = targetUri;
        }

        public String getDisplayName()
        {
            return displayName;
        }

        public void setDisplayName( String displayName )
        {
            this.displayName = displayName;
        }

        public String getIdentity()
        {
            return identity;
        }

        public void setIdentity( String identity )
        {
            this.identity = identity;
        }

        public String getTargetUri()
        {
            return targetUri;
        }

        public void setTargetUri( String targetUri )
        {
            this.targetUri = targetUri;
        }
        
        @Override
        public String toString()
        {
            return new StringBuilder("display name='")
                         .append(this.getDisplayName())
                         .append("' identity='")
                         .append(this.getIdentity())
                         .append("' target URI='")
                         .append(this.getTargetUri()).toString();
        }
        
        @Override
        public boolean equals( Object other )
        {
            //check for self-comparison
            if ( this == other ) return true;

            if ( !(other instanceof EndpointInfo) ) return false;

            //cast to native object is now safe
            EndpointInfo otherEndpointInfo = (EndpointInfo)other;
            
            return displayName.equals( otherEndpointInfo.displayName ) &&
                   identity.equals( otherEndpointInfo.identity )       &&
                   targetUri.equals( otherEndpointInfo.targetUri );
        }

        @Override
        public int hashCode()
        {
            return displayName.hashCode() * identity.hashCode() * targetUri.hashCode();
        }

    }
    
    public class DialogInfo
    {
        private String id;
        private String state;
        private EndpointInfo localInfo;
        private EndpointInfo remoteInfo;
     
        public DialogInfo( String id, String state, EndpointInfo localInfo, EndpointInfo remoteInfo )
        {
            this.id = id;
            this.state = state;
            this.localInfo = localInfo;
            this.remoteInfo = remoteInfo;
        }
               
        public String getId()
        {
            return id;
        }
        
        public void setId( String id )
        {
            this.id = id;
        }
        
        public String getState()
        {
            return state;
        }

        public void setState( String state )
        {
            this.state = state;
        }

        public EndpointInfo getLocalInfo()
        {
            return localInfo;
        }

        public void setLocalInfo( EndpointInfo localInfo )
        {
            this.localInfo = localInfo;
        }

        public EndpointInfo getRemoteInfo()
        {
            return remoteInfo;
        }

        public void setRemoteInfo( EndpointInfo remoteInfo )
        {
            this.remoteInfo = remoteInfo;
        }

        
        @Override
        public String toString()
        {
            return new StringBuilder("Dialog id='")
                         .append(this.getId())
                         .append("' state='")
                         .append(this.getState())
                         .append("' local info='")
                         .append(this.getLocalInfo())
                         .append("' remote info='")
                         .append(this.getRemoteInfo())
                         .append("'\n").toString();        
        }
        
        @Override
        public boolean equals( Object other )
        {
            //check for self-comparison
            if ( this == other ) return true;

            if ( !(other instanceof DialogInfo) ) return false;

            //cast to native object is now safe
            DialogInfo otherDialogInfo = (DialogInfo)other;
            return id.equals( otherDialogInfo.id );
        }

        @Override
        public int hashCode()
        {
            return id.hashCode();
        }
        
    }
    
    public DialogInfoMessagePart( String messagePart ) throws Exception
    {
        super( messagePart );    
        parse();
    }
    
    synchronized private void parse()
    {
        try
        {
            // XPath does not handle default XML namespaces very well.
            // Although we could set up a NamespaceContext to map the 
            // urn:ietf:params:xml:ns:dialog-info namespace to a prefix
            // that we could use in our subsequent 'xpath.evaluate()' calls
            // this leads to ugly code.  Because the dialog info XML
            // body only uses the default namespace, we are taking a 
            // shortcut here and remove the default namespace from to
            // body.
            
            XPath xpath = XPathFactory.newInstance().newXPath();
            String namespaceLessContent = this.getContent().replace( "xmlns", "dummy" );
            ByteArrayInputStream is  = new ByteArrayInputStream( namespaceLessContent.getBytes() );
            InputSource source = new InputSource( is );

            // get entity info
            this.entity = xpath.evaluate( "/dialog-info/@entity", source );
            is.reset();

            // get state info
            this.state = xpath.evaluate( "/dialog-info/@state", source );
            is.reset();

             // get dialog states
            String countAsString = xpath.evaluate( "count(/dialog-info/dialog)", source );
            is.reset();
            int dialogCount = Integer.parseInt( countAsString );

            // process each dialog
            for( int index = 0; index < dialogCount; index++ )
            {
                StringBuilder dialogIdEvalString = new StringBuilder("/dialog-info/dialog[").append(index+1).append("]/@id");
                StringBuilder dialogStateEvalString = new StringBuilder("/dialog-info/dialog[").append(index+1).append("]/state");
                StringBuilder localDisplayNameEvalString = new StringBuilder("/dialog-info/dialog[").append(index+1).append("]/local/identity/@display");
                StringBuilder localIdentityEvalString = new StringBuilder("/dialog-info/dialog[").append(index+1).append("]/local/identity");
                StringBuilder localTargetUriEvalString = new StringBuilder("/dialog-info/dialog[").append(index+1).append("]/local/target/@uri");
                StringBuilder remoteDisplayNameEvalString = new StringBuilder("/dialog-info/dialog[").append(index+1).append("]/remote/identity/@display");
                StringBuilder remoteIdentityEvalString = new StringBuilder("/dialog-info/dialog[").append(index+1).append("]/remote/identity");
                StringBuilder remoteTargetUriEvalString = new StringBuilder("/dialog-info/dialog[").append(index+1).append("]/remote/target/@uri");

                String id = xpath.evaluate( dialogIdEvalString.toString(), source );
                is.reset();
                String state = xpath.evaluate( dialogStateEvalString.toString(), source );
                is.reset();
                String localDisplayName = xpath.evaluate( localDisplayNameEvalString.toString(), source );
                is.reset();
                String localIdentity = xpath.evaluate( localIdentityEvalString.toString(), source );
                is.reset();
                String localTargetUri = xpath.evaluate( localTargetUriEvalString.toString(), source );
                is.reset();
                String remoteDisplayName = xpath.evaluate( remoteDisplayNameEvalString.toString(), source );
                is.reset();
                String remoteIdentity = xpath.evaluate( remoteIdentityEvalString.toString(), source );
                is.reset();
                String remoteTargetUri = xpath.evaluate( remoteTargetUriEvalString.toString(), source );
                is.reset();
                
                EndpointInfo localInfo = new EndpointInfo( localDisplayName, localIdentity, localTargetUri );
                EndpointInfo remoteInfo = new EndpointInfo( remoteDisplayName, remoteIdentity, remoteTargetUri );
                DialogInfo dialogInfo = new DialogInfo( id, state, localInfo, remoteInfo );
                dialogsList.add( dialogInfo );
            }
        }
        catch( Exception ex )
        {
            logger.error("DialogInfoMessagePart::parse caught " + ex.getMessage() );
        }
    }
    
    @Override
    public String toString()
    {
        StringBuilder output = new StringBuilder("DialogInfoMessagePart for ")
                                   .append( entity )
                                   .append(": state='")
                                   .append( state )
                                   .append( "'\n" );
        for( DialogInfo di : this.dialogsList )
        {
            output.append( di.toString() );
        }
        return output.toString();
    }

    public ArrayList<DialogInfo> getDialogsList()
    {
        return dialogsList;
    }

    public String getEntity()
    {
        return entity;
    }

    public String getState()
    {
        return state;
    }
}
