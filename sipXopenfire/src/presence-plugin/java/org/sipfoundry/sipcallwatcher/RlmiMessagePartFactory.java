package org.sipfoundry.sipcallwatcher;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.sipfoundry.sipcallwatcher.MultipartMessages.MessagePart;
import org.sipfoundry.sipcallwatcher.MultipartMessages.MessagePartFactory;

public class RlmiMessagePartFactory implements MessagePartFactory 
{
	static RlmiMessagePartFactory instance = null;
	static final private Pattern contentTypeRegex = Pattern.compile(".*CONTENT-TYPE:\\s*(\\p{Print}+)\\s*.*", Pattern.DOTALL | Pattern.MULTILINE);

	static MessagePartFactory CreateFactory()
	{
		if( instance == null )
		{
			instance = new RlmiMessagePartFactory();
		}
		return instance;
	}
	
	public MessagePart createMessagePart(String messageBody ) throws Exception
	{
		MessagePart messagePart = null;
		Matcher m = contentTypeRegex.matcher( messageBody );
		if( m.matches() )
		{
			String contentType = m.group(1);
			if( contentType.equals( "application/dialog-info+xml" ) )
			{
				messagePart = new DialogInfoMessagePart( messageBody );
			}
			else if( contentType.equals( "application/rlmi+xml" ) )
			{
				messagePart = new RlmiMessagePart( messageBody );
			}
		}
		return messagePart;
	}
	
	// private c'tor used to enforce singleton pattern
	private RlmiMessagePartFactory()
	{
		
	}

}
