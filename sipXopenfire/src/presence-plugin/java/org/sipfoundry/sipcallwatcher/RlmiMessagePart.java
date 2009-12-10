package org.sipfoundry.sipcallwatcher;

import java.io.ByteArrayInputStream;
import java.util.ArrayList;

import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathFactory;

import org.apache.log4j.Logger;
import org.sipfoundry.sipcallwatcher.MultipartMessages.MessagePart;
import org.xml.sax.InputSource;

public class RlmiMessagePart extends MessagePart 
{
    private static Logger logger = Logger.getLogger(RlmiMessagePart.class);
    private ArrayList<ResourceInfo> resourcesList = new ArrayList<ResourceInfo>();
    private int version;
    private String fullState;

    public RlmiMessagePart( String messagePart ) throws Exception
	{
		super( messagePart );	
		parse();
	}
	
    public class ResourceInfo
    {
        private String uri;
        private String state;
        private String cid;

        public ResourceInfo( String cid, String state, String uri )
        {
            this.cid = cid;
            this.state = state;
            this.uri = uri;
        }
        
        public String getUri()
        {
            return uri;
        }
        
        public void setUri( String uri )
        {
            this.uri = uri;
        }
        
        public String getState()
        {
            return state;
        }
        
        public void setState( String state )
        {
            this.state = state;
        }
        
        public String getCid()
        {
            return cid;
        }
        
        public void setCid( String cid )
        {
            this.cid = cid;
        }
        
        @Override
        public String toString()
        {
            return new StringBuilder("Resouce uri='")
                         .append(this.getUri())
                         .append("' state='")
                         .append(this.getState())
                         .append("' cid='")
                         .append(this.getCid())
                         .append("'\n").toString();        
        }        
        
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

			// get resource count
			String countAsString = xpath.evaluate( "count(/list/resource)", source );
			is.reset();
			int resourcesInList = Integer.parseInt( countAsString );

			// get version
			String versionAsString = xpath.evaluate( "/list/@version", source );
			is.reset();
			this.version = Integer.parseInt( versionAsString );
			
			// get fullState 
			this.fullState = xpath.evaluate( "/list/@fullState", source );
			is.reset();

			for( int index = 0; index < resourcesInList; index++ )
			{
				StringBuilder resouceUriEvalString = new StringBuilder("/list/resource[").append(index+1).append("]/@uri");
				StringBuilder resouceStateEvalString = new StringBuilder("/list/resource[").append(index+1).append("]/instance/@state");
				StringBuilder resouceCidEvalString = new StringBuilder("/list/resource[").append(index+1).append("]/instance/@cid");

				String uri = xpath.evaluate( resouceUriEvalString.toString(), source );
				is.reset();
				String state = xpath.evaluate( resouceStateEvalString.toString(), source );
				is.reset();
				String cid = xpath.evaluate( resouceCidEvalString.toString(), source );
				is.reset();
				
				this.resourcesList.add( new ResourceInfo( cid, state, uri ) );
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
        StringBuilder output = new StringBuilder("RlmiMessagePart version ")
                                   .append( version )
                                   .append(": fullState='")
                                   .append( fullState )
                                   .append( "'\n" );
        for( ResourceInfo ri : this.resourcesList )
        {
            output.append( ri.toString() );
        }
        return output.toString();
    }

    public ArrayList<ResourceInfo> getResourcesList()
    {
        return resourcesList;
    }

    public int getVersion()
    {
        return version;
    }

    public boolean isFullState()
    {
        return fullState.equals( "true" ) || fullState.equals( "1" ); 
    }	
}

	

