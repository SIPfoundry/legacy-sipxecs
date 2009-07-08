package org.sipfoundry.sipcallwatcher;

import org.apache.log4j.Logger;
import org.sipfoundry.sipcallwatcher.MultipartMessages.MessagePart;
import org.xml.sax.InputSource;
import java.io.ByteArrayInputStream;
import java.util.ArrayList;

import javax.xml.xpath.*;

public class DialogInfoMessagePart extends MessagePart 
{
    private static Logger logger = Logger.getLogger(DialogInfoMessagePart.class);
    private ArrayList<DialogInfo> dialogsList = new ArrayList<DialogInfo>();
    private String entity;
    private String state;

    public class DialogInfo
    {
        private String id;
        private String state;
     
        public DialogInfo( String id, String state )
        {
            this.id = id;
            this.state = state;
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
        
        @Override
        public String toString()
        {
            return new StringBuilder("Dialog id='")
                         .append(this.getId())
                         .append("' state='")
                         .append(this.getState())
                         .append("'\n").toString();        
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

                String id = xpath.evaluate( dialogIdEvalString.toString(), source );
                is.reset();
                String state = xpath.evaluate( dialogStateEvalString.toString(), source );
                is.reset();
                
                dialogsList.add( new DialogInfo( id, state ) );
            }
		}
		catch( Exception ex )
		{
		    logger.error("Caught " + ex.getMessage() );
			ex.printStackTrace();
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
