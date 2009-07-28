//
//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Diagnostics;
using System.Windows.Forms;

namespace AddinForOutlook
{

    public class FileLogging
    {


        public static string GenerateDefaultLogFileName(string BaseFileName)
        {
            return AppDomain.CurrentDomain.BaseDirectory + "\\" + BaseFileName + "_" + DateTime.Now.Month + "_" + DateTime.Now.Day + "_" + DateTime.Now.Year + ".log";
        }

        /// <summary>
        /// Pass in the fully qualified name of the log file you want to write to
        /// and the message to write
        /// </summary>
        /// <param name="LogPath"></param>
        /// <param name="Message"></param>
        public static void WriteToLog(string LogPath, string Message)
        {
            try
            {
                using (StreamWriter s = File.AppendText(LogPath))
                {
                    s.WriteLine(DateTime.Now + "\t" + Message);
                }
            }

            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine(ex.Message);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="level"></param>
        /// <param name="message"></param>
        public static void WriteEntry(string message)
        {
            WriteToLog(GenerateDefaultLogFileName(Constants.PRODUCT_NAME), message);
        }


        /// <summary>
        /// 
        /// </summary>
        /// <param name="Source"></param>
        /// <param name="Message"></param>
        /// <param name="EntryType"></param>
        public static void WriteToEventLog(string Source, string Message, System.Diagnostics.EventLogEntryType EntryType)
        {
            try
            {
                if (!EventLog.SourceExists(Source))
                {
                    EventLog.CreateEventSource(Source, "Application");
                }
                EventLog.WriteEntry(Source, Message, EntryType);
            }

            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine(ex.Message);
            }

        }
    }
}
