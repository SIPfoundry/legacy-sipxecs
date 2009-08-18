//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////



// Includes
#ifdef WIN32
#include <sys/timeb.h>
#elif defined(__pingtel_on_posix__)
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#define ERROR (-1)
#endif
#include "rtcp/SenderReport.h"
#ifdef INCLUDE_RTCP /* [ */

// Constants
// Difference between LocalTime and Wall Time:
        const unsigned long WALLTIMEOFFSET  = 2208992400UL;

// 2**32 ("4 Gig"):
        const double        FOUR_GIGABYTES  = (65536.0*65536.0);

// Microsecond to second conversion
        const double        MICRO2SEC       = 1000000.0;

// Millisecond to MicroSecond Conversion
        const int           MILLI2MICRO     = 1000;

// Receiver Report Length:
        const int           RR_LENGTH       = 24;
/**
 *
 * Method Name:  CSenderReport() - Constructor
 *
 *
 * Inputs:   unsigned long           ulSSRC  - The Identifier for this source
 *           ISetReceiverStatistics *piSetStatistics
 *                                     - Interface for setting receiver stats
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description:  Performs routine CSenderReport object initialization.
 *
 * Usage Notes:  A CSenderReport object shall be created by the CRTCPRender
 *               with this constructor.  The Sender shall be responsible for
 *               maintain sender statistics related to an outbound RTP
 *               connection.  The constructor shall be pass the SSRC and an
 *               optional pointer to the Set Statistics interface of the
 *               receiver report.
 *
 */
CSenderReport::CSenderReport(unsigned long ulSSRC,
                             ISetReceiverStatistics *piSetStatistics) :
          CRTCPHeader(ulSSRC, etSenderReport),  // Base class construction
          m_ulPacketCount(0),
          m_ulOctetCount(0),
          m_bMediaSent(FALSE),
          m_ulRTPTimestamp(0),
          m_ulSamplesPerSecond(SAMPLES_PER_SEC),
          m_ulRandomOffset(0)
{


    // Store the Statistics interface as an attribute
    m_piSetReceiverStatistics = piSetStatistics;

    // Increment the interface's reference counter
    if(m_piSetReceiverStatistics)
        m_piSetReceiverStatistics->AddRef();

    // Initialize NTP Timestamps
    m_aulNTPTimestamp[0]      = 0;
    m_aulNTPTimestamp[1]      = 0;
    m_aulNTPStartTime[0]      = 0;
    m_aulNTPStartTime[1]      = 0;
}



/**
 *
 * Method Name: ~CSenderReport() - Destructor
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Shall deallocate and/or release all resources that were
 *              acquired over the course of runtime.
 *
 * Usage Notes:
 *
 *
 */
CSenderReport::~CSenderReport(void)
{

// Our reference count must have gone to 0 to get here.  We have not allocated
// any memory so we shall now go quietly into that good night!

    // Release the interface's reference counter
    if(m_piSetReceiverStatistics)
        m_piSetReceiverStatistics->Release();

}


/**
 *
 * Method Name:  IncrementCounts
 *
 *
 * Inputs:       unsigned long  ulOctetCount    -   RTP Octets Sent
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  The IncrementCounts method shall add the number of octets
 *               passed to the cumulative octet count stored as an attribute
 *               to this object. Each call to IncrementCounts() shall also
 *               increment the packet count by 1.
 *
 * Usage Notes:
 *
 */
void CSenderReport::IncrementCounts(unsigned long ulOctetCount)
{

    // We will increment the packet count by 1 and the Octet count by the
    //  number specified within the octet count
    m_ulPacketCount++;
    m_ulOctetCount += ulOctetCount;

    // Set the Media Sent flag to so that we know to transmit a Sender
    // Report in the next reporting period.
    m_bMediaSent = TRUE;

}

/**
 *
 * Method Name:  SetRTPTimestamp
 *
 *
 * Inputs:   unsigned long ulRandomOffset     - Random Offset for RTP Timestamp
 *           unsigned long ulSamplesPerSecond - Number of samples per second
 *
 * Outputs:  None
 *
 * Returns:  void
 *
 * Description:  The SetRTPTimestamp method shall initialized values that are
 *               used to determine the RTP Timestamp value to be sent out in
 *               an SR Report.
 *
 * Usage Notes:
 *
 */
void CSenderReport::SetRTPTimestamp(unsigned long ulRandomOffset,
                                    unsigned long ulSamplesPerSecond)
{

    // Set Timestamp Information
    m_ulRandomOffset     = ulRandomOffset;
    m_ulSamplesPerSecond = ulSamplesPerSecond;

    // Let's check whether an initial NTP timestamp has been established.
    //  If so, ignore.
    if(!m_aulNTPStartTime[0] && !m_aulNTPStartTime[1])
    {
        double dTimestamp;

#ifdef WIN32
        struct _timeb stLocalTime;

        // Get the LocalTime expressed as seconds since 1/1/70 (UTC)
        _ftime(&stLocalTime);

        // Load Most Significant word with Wall time seconds
        m_aulNTPStartTime[0] = stLocalTime.time + WALLTIMEOFFSET;

        // Load Least Significant word with Wall time microseconds
        dTimestamp = stLocalTime.millitm * MILLI2MICRO;
        dTimestamp *= (double)(FOUR_GIGABYTES/MICRO2SEC);

#elif defined(__pingtel_on_posix__)
        struct timeval tv;

        gettimeofday(&tv, NULL);
        // Load Most Significant word with Wall time seconds
        m_aulNTPStartTime[0] = tv.tv_sec + WALLTIMEOFFSET;

        // Load Least Significant word with Wall time microseconds
        dTimestamp = (double) tv.tv_usec / MILLI2MICRO;
        dTimestamp *= (double) (FOUR_GIGABYTES / MICRO2SEC);

#else
        struct timespec stLocalTime;

        // Make a call to VxWorks to get this timestamp
        if (clock_gettime(CLOCK_REALTIME, &stLocalTime) == ERROR)
        {
            osPrintf("**** FAILURE **** SetRTPTimestamp() - clock_gettime failure\n");
            stLocalTime.tv_sec = 0;
            stLocalTime.tv_nsec = 0;
        }

        // Load Most Significant word with Wall time seconds
        m_aulNTPStartTime[0] = stLocalTime.tv_sec + WALLTIMEOFFSET;

        // Load Least Significant word with Wall time microseconds
        dTimestamp = (double)stLocalTime.tv_nsec / MILLI2MICRO;
        dTimestamp *= (double)(FOUR_GIGABYTES / MICRO2SEC);

#endif

        // Store microsecond portion of NTP
        m_aulNTPStartTime[1] = (unsigned long)dTimestamp;

    }
}

/**
 *
 * Method Name:  GetSenderStatistics
 *
 *
 * Inputs:      None
 *
 * Outputs:     unsigned long   *pulPacketCount   - Sender Packet Count
 *              unsigned long   *pulOctetCount    - Sender Octet Count
 *
 * Returns:     void
 *
 * Description: Returns the packet and octet counts values stored as members.
 *
 * Usage Notes:
 *
 *
 *
 */
void CSenderReport::GetSenderStatistics(unsigned long *pulPacketCount,
                                        unsigned long *pulOctetCount)
{

    // Pass back the current packet and octet counts
    *pulPacketCount = m_ulPacketCount;
    *pulOctetCount  = m_ulOctetCount;

}


/**
 *
 * Method Name:  SetSSRC
 *
 *
 * Inputs:      unsigned long   ulSSRC   - Source ID
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Stores the Source Identifier associated with an RTP connection.
 *
 * Usage Notes: This is an override of the base class method defined in
 *              CRTCPHeader.  This method shall additionally reset the octet
 *              and packet count accumulators as mandated by standard.
 *
 *
 *
 */
void CSenderReport::SetSSRC(unsigned long ulSSRC)
{

    // An SSRC collision must have been detected for this to occur.
    // Let's reset our statistics.
    ResetStatistics();

    // Let's delegate to the base class method to set the new SSRC ID
    CRTCPHeader::SetSSRC(ulSSRC);

}



/**
 *
 * Method Name:  FormatSenderReport
 *
 *
 * Inputs:   unsigned long  ulBufferSize     - length allocated for the buffer
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
 *
 * Outputs:  unsigned char *puchReportBuffer
 *                                    - Buffer used to store the Sender Report
 *
 * Returns:  unsigned long - number of octets written into the buffer.
 *
 * Description: Constructs a Sender report using the buffer passed in by the
 *              caller.  The Sender Report object shall keep track of the
 *              reporting periods that have passed an which information should
 *              be used to populate the report.
 *
 * Usage Notes: The header of the RTCP Report shall be formatted by delegating
 *              to the base class.
 *
 *
 */
unsigned long CSenderReport::FormatSenderReport(
                  unsigned char *puchReportBuffer, unsigned long ulBufferSize)
{

    unsigned long ulReportLength=0;

    // Let's offset into the Formatting buffer enough to
    //  start depositing payload
    unsigned char *puchPayloadBuffer = puchReportBuffer + GetHeaderLength();

    // Let's load the NTP and RTP timestamps into the Sender Report
    puchPayloadBuffer += LoadTimestamps((unsigned long *)puchPayloadBuffer);

    // Let's load the Sender Statistics
    puchPayloadBuffer += LoadSenderStats((unsigned long *)puchPayloadBuffer);

    // Set the sender report length
    ulReportLength = puchPayloadBuffer - puchReportBuffer;

    // Let's slap a header on this report
    FormatRTCPHeader( puchReportBuffer,  // RTCP Report Buffer
          FALSE,                         // No Padding
          1,                             // Receiver Count set to 1 for now
          ulReportLength + RR_LENGTH);   // Report Length

    return(ulReportLength);

}


/**
 *
 * Method Name:  ParseSenderReport
 *
 *
 * Inputs:   unsigned char *puchReportBuffer
 *                               - Buffer containing the Sender Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: Extracts the contents of an Sender report using the buffer
 *              passed in by the caller.  The Sender Report object shall store
 *              the content and length of data fields extracted from the Sender
 *              Report.  The timestamps identifying the time of SR report
 *              reception shall obtained and sent with the SR Send timestamp to
 *              the associated Receiver Report through the SetLastRcvdSRTime()
 *              method of the ISetReceiverStatistics interface.
 *
 * Usage Notes: The header of the RTCP Report shall be parsed by delegating to
 *              the base class.
 *
 *
 */
unsigned long CSenderReport::ParseSenderReport(unsigned char *puchReportBuffer)
{

    unsigned char   *puchPayloadBuffer = puchReportBuffer;

    // Check whether the RTCP Header has been correctly
    //  formed (Version, etc...).
    if(!ParseRTCPHeader(puchReportBuffer))
        return(GetReportLength());

    // Good header.  Let's bump the payload pointer and continue.
    puchPayloadBuffer += GetHeaderLength();

    // Let's extract the NTP and RTP timestamps from the Sender Report
    puchPayloadBuffer += ExtractTimestamps((unsigned long *)puchPayloadBuffer);

    // Let's extract the Sender Statistics
    puchPayloadBuffer += ExtractSenderStats((unsigned long*)puchPayloadBuffer);

    return(puchPayloadBuffer - puchReportBuffer);

}




/**
 *
 * Method Name:  ResetStatistics
 *
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  This method shall reset all sedner report statistics.
 *               A reset shall occur when the SSRC ID is reset due to a
 *               collision with a participating source.
 *
 * Usage Notes:
 *
 */
void CSenderReport::ResetStatistics(void)
{

    // We must set both the packet and octet counts to 0
    m_ulPacketCount = 0;
    m_ulOctetCount = 0;

}


/**
 *
 * Method Name:  LoadTimestamps
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned long *aulTimestamps - Long word array in which to load
 *                                          the NTP and RTP timestamps
 *                                          (WHAT FORMAT???)
 *
 * Returns:  unsigned long - Size of the data loaded (WHAT UNITS???)
 *
 * Description:  This method shall use the VxWorks Network time protocol
 *               service to get a 64 bit representation of the current Network
 *               time and 32 bit for RTP time.
 *
 * Usage Notes:
 *
 */
unsigned long CSenderReport::LoadTimestamps(unsigned long *aulTimestamps)
{
    double dTimestamp;

#ifdef WIN32
    struct _timeb stLocalTime;

    // Get the LocalTime expressed as seconds since 1/1/70 (UTC)
    _ftime(&stLocalTime);

    // Load Most Significant word with Wall time seconds
    m_aulNTPTimestamp[0] = stLocalTime.time + WALLTIMEOFFSET;

    // Load Least Significant word with Wall time microseconds
    dTimestamp = stLocalTime.millitm * MILLI2MICRO;
    dTimestamp *= (double)(FOUR_GIGABYTES/MICRO2SEC);

#elif defined(__pingtel_on_posix__)
        struct timeval tv;

        gettimeofday(&tv, NULL);
        // Load Most Significant word with Wall time seconds
        m_aulNTPStartTime[0] = tv.tv_sec + WALLTIMEOFFSET;

        // Load Least Significant word with Wall time microseconds
        dTimestamp = (double) tv.tv_usec / MILLI2MICRO;
        dTimestamp *= (double) (FOUR_GIGABYTES / MICRO2SEC);

#else
    struct timespec stLocalTime;

    // Make a call to VxWorks to get this timestamp
    if (clock_gettime(CLOCK_REALTIME, &stLocalTime) == ERROR)
    {
        osPrintf("**** FAILURE **** LoadTimestamps() - clock_gettime failure\n");
        stLocalTime.tv_sec = 0;
        stLocalTime.tv_nsec = 0;
    }

    // Load Most Significant word with Wall time seconds
    m_aulNTPTimestamp[0] = stLocalTime.tv_sec + WALLTIMEOFFSET;

    // Load Least Significant word with Wall time microseconds
    dTimestamp = (double)stLocalTime.tv_nsec / MILLI2MICRO;
    dTimestamp *= (double)(FOUR_GIGABYTES / MICRO2SEC);


#endif

    // Store microsecond portion of NTP
    m_aulNTPTimestamp[1] = (unsigned long)dTimestamp;

    // Assign NTP Time to Sender Report Buffer
    *aulTimestamps = htonl(m_aulNTPTimestamp[0]);
    aulTimestamps++;
    *aulTimestamps = htonl(m_aulNTPTimestamp[1]);
    aulTimestamps++;

    // Calculate RTP Timestamp by taking the difference between the current
    //  and starting NTP timestamps
    double dSecondsElapsed  =
                        (double)(m_aulNTPTimestamp[0] - m_aulNTPStartTime[0]);
    double dUSecondsElapsed =
                        (double)(m_aulNTPTimestamp[1] - m_aulNTPStartTime[1]);

    // Round Seconds down if Microsecond difference is less that 0.
    while (dUSecondsElapsed < 0)
    {
        dSecondsElapsed--;
        dUSecondsElapsed += MICRO2SEC;
    }

    // Express in fractions of seconds
    dUSecondsElapsed /= MICRO2SEC;

    // Express total elapsed time in sample Units per second
    double dElapsedUnits = (dSecondsElapsed + dUSecondsElapsed) /
                                     ((1.0) / ((double)m_ulSamplesPerSecond));

    // Adjust by random offset and format in Network Byte Order
    m_ulRTPTimestamp = (unsigned long) (dElapsedUnits + m_ulRandomOffset);
    *aulTimestamps =  htonl(m_ulRTPTimestamp);

    return(sizeof(m_aulNTPTimestamp) + sizeof(m_ulRTPTimestamp));

}



/**
 *
 * Method Name:  LoadSenderStats
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned long *aulSenderStats - Long word array in which
 *                                           to load the statistics
 *
 * Returns:  unsigned long - Amount of data loaded
 *
 * Description:  This method shall retrieve the packet and octet counts.
 *
 * Usage Notes:
 *
 */
unsigned long  CSenderReport::LoadSenderStats(unsigned long *aulSenderStats)
{

    // The RTP timestamp shall be based on the NTP timestamp
    *aulSenderStats     = htonl(m_ulPacketCount);
    *(aulSenderStats+1) = htonl(m_ulOctetCount);

    // Reset the Media Sent flag to so that we can determine whether a Sender
    // Report is necessary.
    m_bMediaSent = FALSE;

    return(sizeof(m_ulPacketCount) + sizeof(m_ulOctetCount));

}


/**
 *
 * Method Name:  ExtractTimestamps
 *
 *
 * Inputs:   unsigned long *paulTimestamps
 *                                   - Array containing the NTP/RTP Timestamps
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Size of the data extracted
 *
 * Description:  This method shall extract the 64 bits of NTP time information
 *               and the 32-bits of RTP timestamp and store them both in
 *               respective report attributes.
 *
 * Usage Notes:
 *
 */
unsigned long CSenderReport::ExtractTimestamps(unsigned long *paulTimestamps)
{
    unsigned long aulCurrentNTPTime[2];
    double        dTimestamp;

    // Load the two long word into NTP timestamp array
    m_aulNTPTimestamp[0] = ntohl(*paulTimestamps);
    paulTimestamps++;
    m_aulNTPTimestamp[1] = ntohl(*paulTimestamps);
    paulTimestamps++;

    // Store the RTP timestamp associated with this report
    m_ulRTPTimestamp = ntohl(*paulTimestamps);


    // Let's perform some calculations that will be useful in
    //  determining SR Delay
#ifdef WIN32
    struct _timeb stLocalTime;

    // Get the LocalTime expressed as seconds since 1/1/70 (UTC)
    _ftime(&stLocalTime);

    // Load Most Significant word with Wall time seconds
    aulCurrentNTPTime[0] = stLocalTime.time + WALLTIMEOFFSET;

    // Load Least Significant word with Wall time microseconds
    dTimestamp = stLocalTime.millitm * MILLI2MICRO;
    dTimestamp *= (double)(FOUR_GIGABYTES/MICRO2SEC);

#elif defined(__pingtel_on_posix__)
    struct timeval tv;

    gettimeofday(&tv, NULL);
    // Load Most Significant word with Wall time seconds
    aulCurrentNTPTime[0] = tv.tv_sec + WALLTIMEOFFSET;

    // Load Least Significant word with Wall time microseconds
    dTimestamp = (double) tv.tv_usec / MILLI2MICRO;
    dTimestamp *= (double) (FOUR_GIGABYTES / MICRO2SEC);

#else
    struct timespec stLocalTime;

    // Make a call to VxWorks to get this timestamp
    if (clock_gettime(CLOCK_REALTIME, &stLocalTime) == ERROR)
    {
        osPrintf("**** FAILURE **** LoadTimestamps() - clock_gettime failure\n");
        stLocalTime.tv_sec = 0;
        stLocalTime.tv_nsec = 0;
    }

    // Load Most Significant word with Wall time seconds
    aulCurrentNTPTime[0] = stLocalTime.tv_sec + WALLTIMEOFFSET;

    // Load Least Significant word with Wall time microseconds
    dTimestamp = (double)stLocalTime.tv_nsec / MILLI2MICRO;
    dTimestamp *= (double)(FOUR_GIGABYTES / MICRO2SEC);


#endif

    // Store microsecond portion of NTP
    aulCurrentNTPTime[1] = (unsigned long)dTimestamp;


    // Calculate Current RTP Timestamp by taking the difference
    //  between the current and starting NTP timestamps
    double dSecondsElapsed  =
                        (double)(aulCurrentNTPTime[0] - m_aulNTPStartTime[0]);
    double dUSecondsElapsed =
                        (double)(aulCurrentNTPTime[1] - m_aulNTPStartTime[1]);

    // Round Seconds down if Microsecond difference is less that 0.
    while (dUSecondsElapsed < 0)
    {
        dSecondsElapsed--;
        dUSecondsElapsed += MICRO2SEC;
    }

    // Express in fractions of seconds
    dUSecondsElapsed /= MICRO2SEC;

    // Express total elapsed time in sample Units per seond
    m_ulRTPTimestamp = (unsigned long)(dSecondsElapsed + dUSecondsElapsed);


    // Use the CReceiverReport's ISetReceiverStatistics Interface to timestamp
    //  when the last Sender Report was Received.
    m_piSetReceiverStatistics->SetLastRcvdSRTime(m_aulNTPTimestamp);

    return(sizeof(m_aulNTPTimestamp) + sizeof(m_ulRTPTimestamp));

}

/**
 *
 * Method Name:  ExtractSenderStats
 *
 *
 * Inputs:   unsigned long *aulSenderStats
 *                           - Long word array in which to load the statistics
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Amount of data extracted
 *
 * Description:  This method shall extract the packet and octet counts from
 *               the Sender Report.
 *
 * Usage Notes:
 *
 */
unsigned long  CSenderReport::ExtractSenderStats(unsigned long *aulSenderStats)
{

    // The RTP timestamp shall be based on the NTP timestamp
    m_ulPacketCount = ntohl(*aulSenderStats);
    m_ulOctetCount =  ntohl(*(aulSenderStats+1));

    return(sizeof(m_ulPacketCount) + sizeof(m_ulOctetCount));

}


#endif /* INCLUDE_RTCP ] */
