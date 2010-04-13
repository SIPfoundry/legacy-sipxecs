//
//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

using System;
using System.Diagnostics;
using System.Reflection;
using System.Windows.Forms;

namespace AddinForOutlook
{
    public enum LogLevel
    {
        Information,
        Warning,
        Error,
        /// <summary>
        /// Level None, logging is turn off
        /// </summary>
        None
    }

    /// <summary>
    /// Delegate definition for logging.
    /// </summary>
    /// <param name="message"></param>
    /// <param name="level"></param>
    public delegate void LoggerDelegate(String message);

    ///<summary>
    ///Logging singleton with configurable output delegate.
    ///</summary>
    ///<remarks>
    ///This singleton provides a centralized log. The actual WriteEntry calls are passed
    ///off to a delegate however. Having a delegate do the actual logginh allows you to
    ///implement different logging mechanism and have them take effect throughout the system.
    ///</remarks>
    public class Logger
    {
        public static LogLevel currentLogLevel = getLogLevelFromReg();
        static public LoggerDelegate Delegate = null;
        static public void WriteEntry(LogLevel level, String message)
        {
            try
            {

                if (Delegate != null && level >= currentLogLevel)
                {
                    StackTrace stackTrace = new StackTrace();
                    StackFrame stackFrame = stackTrace.GetFrame(1); //parent frame
                    MethodBase methodBase = stackFrame.GetMethod(); // who calls me.

                    Delegate("[ "+ level + " ]" + methodBase.Name + ":" + message);
                }
            }

            catch (System.Exception ex)
            {
                FileLogging.WriteToEventLog("Application", ex.Message, System.Diagnostics.EventLogEntryType.Error);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public static LogLevel getLogLevelFromReg()
        {
            try
            {
                Microsoft.Win32.RegistryKey key;
                key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(Constants.REG_KEY_NAME_ADDIN_SETTING);
                int logLevel = (int)key.GetValue(Constants.REG_ATTR_NAME_LOG_LEVEL);
                key.Close();

                return intToLogLevel(logLevel);
            }

            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, "Exception : " + ex.Message + ex.StackTrace);
                return 0;
            }
        }


        /// <summary>
        /// 
        /// </summary>
        /// <param name="value"></param>
        /// <returns></returns>
        public static LogLevel intToLogLevel(int value)
        {
            switch (value)
            {
                case 0:
                    {
                        return LogLevel.Information;
                    }
                case 1:
                    {
                        return LogLevel.Warning;
                    }
                case 2:
                    {
                        return LogLevel.Error;
                    }
                default:
                    {
                        return LogLevel.None;
                    }
            }
        }

    }
}