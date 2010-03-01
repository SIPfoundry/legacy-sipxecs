package org.sipfoundry.sipxrelay;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import org.apache.log4j.Logger;

public class SipXrelaySemaphore extends Semaphore {
	private static Logger logger = Logger.getLogger(SipXrelaySemaphore.class);

	private String acquiredAt;

	 static String getStackTrace() {
			StringWriter sw = new StringWriter();
			PrintWriter pw = new PrintWriter(sw);
			StackTraceElement[] ste = new Exception().getStackTrace();
			// Skip the log writer frame and log all the other stack frames.
			for (int i = 0; i < ste.length; i++) {
				String callFrame = "[" + ste[i].getFileName() + ":"
						+ ste[i].getLineNumber() + "]";
				pw.print(callFrame);
			}
			pw.close();
			return sw.getBuffer().toString();

	}
	 
	public SipXrelaySemaphore(int permits) {
		super(permits);
	}

	@Override
	public boolean tryAcquire(long time, TimeUnit timeUnit)
			throws InterruptedException {

		boolean retval = super.tryAcquire(time, timeUnit);
		if (logger.isDebugEnabled()) {
			if (!retval) {
				logger.debug("Failed to acquire semaphore ");
				logger.debug(getStackTrace());
				logger.debug("Previously acquired at " + acquiredAt);
			} else {
				acquiredAt = getStackTrace();
			}
		}
		return retval;
	}
	
	@Override
	public void acquire() throws InterruptedException {
		super.acquire();
		if ( logger.isDebugEnabled() ) {
			this.acquiredAt = getStackTrace();
		}
	}

}
