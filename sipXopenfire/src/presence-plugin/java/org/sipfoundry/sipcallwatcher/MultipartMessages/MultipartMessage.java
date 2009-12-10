package org.sipfoundry.sipcallwatcher.MultipartMessages;

import java.util.ArrayList;
import java.util.Collection;
import java.lang.Exception;

import org.apache.log4j.Logger;


/**
 * Generic Multipart Message abstraction that breaks up the supplied
 * multipart message body into individual message parts that it construct
 * using the services of the supplied message part factory.
 */
public class MultipartMessage 
{
	private static Logger logger = Logger.getLogger(MultipartMessage.class);
	private Collection<MessagePart> messagePartList = new ArrayList<MessagePart>();

	public MultipartMessage( String body, String boundary, MessagePartFactory factory ) throws Exception
	{
		// take out first boundary to streamline string split to come
		body = body.replaceFirst( "--" + boundary, "" );
		String[] bodies = body.split( "\\s*--" + boundary + "(--)?\\s*" );
		for( String aBody : bodies )
	    {
			MessagePart mp = factory.createMessagePart( aBody );
			if( mp != null )
			{
				messagePartList.add( mp );
			}
			else
			{
				throw new Exception("Could not create message part for '" + aBody + "'");
			}
	    }
	}

	public Collection<MessagePart> getMessagePartList() 
	{
		return messagePartList;
	}
}

