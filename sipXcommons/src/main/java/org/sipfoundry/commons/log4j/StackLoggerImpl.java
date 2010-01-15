package org.sipfoundry.commons.log4j;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Properties;

import org.apache.log4j.Appender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.Priority;

import gov.nist.core.StackLogger;

public class StackLoggerImpl implements StackLogger {
    
    private  Logger logger = Logger.getLogger(StackLoggerImpl.class) ;
    
    private static HashMap<String,Integer> levelMap = new HashMap<String,Integer>();
    
    boolean enabled = true;

    private Properties stackProperties;
 
    static {
        levelMap.put(Level.DEBUG.toString(), new Integer(TRACE_DEBUG));
        levelMap.put(Level.INFO.toString(), new Integer(TRACE_INFO));
        levelMap.put(Level.TRACE.toString(), new Integer(TRACE_TRACE));
        levelMap.put(Level.ERROR.toString(), new Integer(TRACE_ERROR));
        levelMap.put(Level.WARN.toString(), new Integer(TRACE_WARN));
        levelMap.put(Level.FATAL.toString(), new Integer(TRACE_FATAL));
        levelMap.put(Level.OFF.toString(), new Integer(TRACE_NONE));
    }
    
    
    public StackLoggerImpl( ) {
    }
    
    
    public void setLogger(Logger logger) {
       this.logger = logger;
    }

    @Override
    public void disableLogging() {
        enabled = false;
    }

    @Override
    public void enableLogging() {
        enabled = true;

    }

    @Override
    public int getLineCount() {
        return 0;
    }

    @Override
    public boolean isLoggingEnabled() {   
        return enabled;
    }

    @Override
    public boolean isLoggingEnabled(int sipLogLevel) {
       int levelSet = levelMap.get( logger.getLevel().toString());
       return sipLogLevel <= levelSet;
    }

    @Override
    public void logDebug(String string) {
       logger.debug(string);

    }

    @Override
    public void logError(String string) {
        logger.error(string);
    }

    @Override
    public void logError(String string, Exception exception) {
      logger.error(string,exception);

    }

    @Override
    public void logException(Throwable throwable) {
        logger.error("Exception occured",throwable);
    }

    @Override
    public void logFatalError(String string) {     
        logger.fatal("Fatal error " + string);
    }

    @Override
    public void logInfo(String string) { 
        logger.info(string);
    }

    @Override
    public void logStackTrace() {
        if (enabled) {
            StringWriter sw = new StringWriter();
            PrintWriter pw = new PrintWriter(sw);
            StackTraceElement[] ste = new Exception().getStackTrace();
            // Skip the log writer frame and log all the other stack frames.
            for (int i = 1; i < ste.length; i++) {
                String callFrame = "[" + ste[i].getFileName() + ":"
                        + ste[i].getLineNumber() + "]";
                pw.print(callFrame);
            }
            pw.close();
            String stackTrace = sw.getBuffer().toString();
            logInfo(stackTrace);

        }
        
    }

    @Override
    public void logStackTrace(int level) {
        if ( this.isLoggingEnabled(level)) {
            logStackTrace();
        }

    }

    @Override
    public void logWarning(String message) {
        logger.warn(message);
    }

    @Override
    public void setBuildTimeStamp(String timeStamp) {
       logger.info("BuildTimeStamp = " + timeStamp);
    }

    @Override
    public void setStackProperties(Properties properties) {
        logger.info("StackProperties " + properties);
        this.stackProperties = properties;
        Enumeration appenders = this.logger.getAllAppenders();
        Logger newLogger = Logger.getLogger(properties.getProperty("javax.sip.STACK_NAME"));
        if (appenders != null) {
            while (appenders.hasMoreElements() ) {
                newLogger.addAppender( (Appender)appenders.nextElement());
            }  
        }
        newLogger.setLevel(this.logger.getLevel());
        this.logger = newLogger;
    }

    
    @Override
    public String getLoggerName() {
        return this.stackProperties.getProperty("javax.sip.STACK_NAME");
    }

    @Override
    public void logTrace(String message) {
        if ( this.isLoggingEnabled(TRACE_TRACE)) {
             logger.debug(message);
        }
    }


}
