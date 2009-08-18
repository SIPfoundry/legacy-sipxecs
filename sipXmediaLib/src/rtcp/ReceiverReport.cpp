//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


/*
 * Issues:      1. RTP Timestamp via VxWorks.
 *              2. NTP Timestamp via VxWorks.
 *              3. Check calculations carefully during testing
 *              4. Network Byte Order
 ****************************************************************************/


    // Includes
#ifdef WIN32
#include <sys/timeb.h>
#elif defined(_VXWORKS)
#include <timers.h>
#elif defined(__pingtel_on_posix__)
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/time.h>
#define ERROR (-1)
#endif

#include "rtcp/ReceiverReport.h"
#ifdef INCLUDE_RTCP /* [ */

   // Constants
const int     FRACTIONAL_MASK   = 0xFF;
const int     CUMULATIVE_MASK   = 0xFFFFFF;
const int     SEQUENCE_MASK     = 0xFFFF;

const int     LOSS_SHIFT        = 0x8;
const int     SEQUENCE_SHIFT    = 0x10;

const int     DELAY_UNITS       = (1<<16);

const int     RTP_SEQ_MOD       = (1<<16);
const int     MAX_DROPOUT       = 3000;
const int     MAX_MISORDER      = 100;
const int     MIN_SEQUENTIAL    = 2;

const double  MICRO2SEC         = 1000000.0;    // Microsecond to second
const int     MILLI2MICRO       = 1000;         // Millisecond to MicroSecond
const double  SAMPLE_UNITS      = (1.0 / 8000.0);
const double  REDUX_RATIO       = 16.0;
const int     FIXED_CONVERSION  = 256;

/**
 *
 * Method Name:  CReceiverReport() - Constructor
 *
 *
 * Inputs:       unsigned long ulSSRC     - The Identifier for this source
 *               unsigned long ulVersion  - Version of the RFC Standard
 *
 * Outputs:      None
 *
 * Returns:      None
 *
 * Description:  Performs routine CReceiverReport object initialization.
 *
 * Usage Notes:  A CReceiverReport object shall be created by the CRTCPRender
 *               with this constructor.  The Sender shall be responsible for
 *               maintain receiver statistics related to an outbound RTP
 *               connection.  The CReceiverReport shall pass the SSRC and
 *               version number to the CRTCPHeader at object construction.
 *
 */
CReceiverReport::CReceiverReport(unsigned long ulSSRC, unsigned long ulVersion)
     :CRTCPHeader(ulSSRC, etReceiverReport, ulVersion),  // Base class constr
#ifndef WIN32
     m_csSynchronized(NULL),
#endif
     m_ulRemoteSSRC(0),
     m_dLastSRRcvdTimestamp(0),
     mTotalPackets(0),
     mTotalWarnings(0)
{

    // Initialize Critical Section
    InitializeCriticalSection (&m_csSynchronized);

    // Reset Statistics
    ResetStatistics();

}




/**
 *
 * Method Name: ~CReceiverReport() - Destructor
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Shall deallocate and/or release all resources which were
 *              acquired over the course of runtime.
 *
 * Usage Notes:
 *
 *
 */
CReceiverReport::~CReceiverReport(void)
{

    // Our reference count must have gone to 0 to get here.  We have not
    // allocated any memory so we shall now go quietly into that good night!

    // Delete the Critical Section
    DeleteCriticalSection (&m_csSynchronized);


}

/**
 *
 * Method Name:  SetRTPStatistics
 *
 *
 * Inputs:      IRTPHeader *piRTPHeader
 *                            - RTP Packet Header received from RTP Source
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Takes the RTPHeader interface passed by the RTP Source object
 *              and updates Receiver Report statistics based upon its contents.
 *
 * Usage Notes:
 *
 *
 *
 */
void CReceiverReport::SetRTPStatistics(IRTPHeader *piRTPHeader)
{
    // Enter Synchronized Area
    EnterCriticalSection (&m_csSynchronized);

    // Recalculate Receiver Report Statistics based upon the most recent RTP
    // Packet Header received
    piRTPHeader->AddRef();

    // Update the jitter statistics
    UpdateJitter(piRTPHeader);

    // Set the SSRC ID associated with this RTP Packet
    SetRemoteSSRC(piRTPHeader);

    // Position the packet within the sequence list
    UpdateSequence(piRTPHeader);

    piRTPHeader->Release();

    // Leave Synchronized Area
    LeaveCriticalSection (&m_csSynchronized);

}


/**
 *
 * Method Name: SetLastRcvdSRTime
 *
 *
 * Inputs:      unsigned long aulNTPTimestamp[]
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Store the RTP timestamp from the last Sender Report received
 *              and store the time that this report was received on the system.
 *
 * Usage Notes:
 *
 *
 *
 */
void CReceiverReport::SetLastRcvdSRTime(unsigned long aulNTPTimestamp[])
{

    double  dCurrentSeconds, dCurrentUSeconds;

    // Extract and store the timestamp of the Last SR Received.  This is
    // derived by taking the lower 16 bits of the 4 byte representation of
    // seconds and the high 16 bits of the 4 byte representation of
    // microseconds.
    m_ulLastSRTimestamp = ((aulNTPTimestamp[0] & 0xFFFF) << 16);
    m_ulLastSRTimestamp |= ((aulNTPTimestamp[1] >> 16) & 0xFFFF);

    // Let's perform some calculations that will be useful in
    // determining SR Delay
#ifdef WIN32
    struct _timeb stLocalTime;

    // Get the LocalTime expressed as seconds since 1/1/70 (UTC)
    _ftime(&stLocalTime);

    // Load Most Significant word
    dCurrentSeconds = stLocalTime.time;

    // Load Least Significant word with microseconds
    dCurrentUSeconds = stLocalTime.millitm * MILLI2MICRO;

#elif defined(__pingtel_on_posix__)
    struct timeval tv;

    gettimeofday(&tv, NULL);
    dCurrentSeconds = tv.tv_sec;
    dCurrentUSeconds = tv.tv_usec;

#else
    struct timespec stLocalTime;

    // Make a call to VxWorks to get this timestamp
    if (clock_gettime(CLOCK_REALTIME, &stLocalTime) == ERROR)
    {
        osPrintf("**** FAILURE **** LoadTimestamps() - clock_gettime failure\n");
        stLocalTime.tv_sec = 0;
        stLocalTime.tv_nsec = 0;
    }

    // Load Most Significant word with time seconds
    dCurrentSeconds = stLocalTime.tv_sec;

    // Load Least Significant word with time microseconds
    dCurrentUSeconds = (double)stLocalTime.tv_nsec / MILLI2MICRO;

#endif

    // Store time associated with Last SR Received
    m_dLastSRRcvdTimestamp = dCurrentSeconds + (dCurrentUSeconds / MICRO2SEC);

}



/**
 *
 * Method Name:  GetReceiverStatistics
 *
 *
 * Inputs:      None
 *
 * Outputs:
 *     unsigned long  *pulFractionalLoss      - Fractional Packet Loss
 *     unsigned long  *pulCumulativeLoss      - Cumulative Packet Loss
 *     unsigned long  *pulHighestSequenceNo
 *                                 - Highest Sequence Number Received
 *     unsigned long  *pulInterarrivalJitter
 *                                 - Interarrival Packet Variance
 *     unsigned long  *pulSRTimestamp
 *                                 - Timestamp of last Sender Report received
 *     unsigned long  *pulPacketDelay
 *                                 - Delay between last Sender Report Received
 *                                   and sending this report
 *
 * Returns:     void
 *
 * Description: Returns a number of receiver report statistics associated with
 *              an inbound or outbound RTP connection.
 *
 * Usage Notes:
 *
 *
 *
 */
void CReceiverReport::GetReceiverStatistics(unsigned long  *pulFractionalLoss,
                         unsigned long  *pulCumulativeLoss,
                         unsigned long  *pulHighestSequenceNo,
                         unsigned long  *pulInterarrivalJitter,
                         unsigned long  *pulSRTimestamp,
                         unsigned long  *pulPacketDelay)
{

    // Load the output argument passed with the current Receiver
    // Report statistics values.
    *pulFractionalLoss      = m_ulCachedFractionalLoss;
    *pulCumulativeLoss      = m_ulCachedCumulativeLoss;
    *pulHighestSequenceNo   = m_ulCachedHighestSequenceNo;
    *pulInterarrivalJitter  = m_ulCachedMeanJitter;
    *pulSRTimestamp         = m_ulCachedLastSRTimestamp;
    *pulPacketDelay         = m_ulCachedSRDelay;

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
 * Usage Notes:
 *
 *
 *
 */
void CReceiverReport::SetSSRC(unsigned long ulSSRC)
{

    // Store the modified SSRC as an internal attribute
    CRTCPHeader::SetSSRC(ulSSRC);

}


/**
 *
 * Method Name: SetRemoteSSRC
 *
 *
 * Inputs:   IRTPHeader *piRTPHeader - Interface pointer to RTP Header services
 *
 * Outputs:  None
 *
 * Returns:  void
 *
 * Description: Stores the Source Identifier associated with an RTP connection.
 *
 * Usage Notes: Stores the Source Identifier associated with an RTP source as
 *              an internal attribute.
 *
 */

void CReceiverReport::SetRemoteSSRC(IRTPHeader *piRTPHeader)
{

    piRTPHeader->AddRef();

    // Let's determine whether the SSRC, either local or remote depending on
    // context of ReceiverReport, has changed and what to do about it.
    unsigned long ulSSRC = piRTPHeader->GetSSRC();

    // An SSRC collision must have been detected for this to occur.
    // Let's reset our statistics and set the new SSRC.
    if(m_ulRemoteSSRC && ulSSRC != m_ulRemoteSSRC)
    {
        // Reset Stats
        if (mTotalWarnings++ < 20)
            osPrintf(" CReceiverReport::SetRemoteSSRC ==> New SSRC"
                             " Detected - Resetting Receiver Stats\n");
        ResetStatistics();
    }

    // Let's set the new SSRC ID
    m_ulRemoteSSRC = ulSSRC;

    piRTPHeader->Release();
}


/**
 *
 * Method Name:  FormatReceiverReport
 *
 *
 * Inputs:   bool           bSRPresent    - TRUE => Sender Report is prepended
 *           unsigned long  ulBufferSize  - length allocated for the buffer
 *
 * Outputs:  unsigned char *puchReportBuffer
 *                                  - Buffer used to store the Receiver Report
 *
 * Returns:  unsigned long  - number of octets written into the buffer.
 *
 * Description: Constructs a Receiver report using the buffer passed in by
 *              the caller.  A Receiver Report may be appended to the contents
 *              of a Sender Report or sent along in the case where no data was
 *              transmitted during the reporting period.  The header flag set
 *              to True shall cause the Receiver Report to be appended while
 *              False will cause a header to be prepended to the Report
 *              information.
 *
 *              A call to this method shall cause all period counts to be reset.
 *
 * Usage Notes: The header of the RTCP Report shall be formatted by delegating
 *              to the base class.
 *
 *
 *
 */
unsigned long CReceiverReport::FormatReceiverReport(bool bSRPresent,
                                           unsigned char *puchReportBuffer,
                                           unsigned long  ulBufferSize)
{
    unsigned char   *puchPayloadBuffer;
    unsigned long    ulReportLength;

    // Check to see whether the Sender Report has been prepended
    if(bSRPresent)
    {
        // Sender Report present.
        // Deposit payload at the beginning of the buffer
        puchPayloadBuffer = puchReportBuffer;
    }
    else
    {
        // Let's offset into the Formatting buffer enough
        // to start depositing payload
        puchPayloadBuffer = puchReportBuffer + GetHeaderLength();
    }

    // Enter Synchronized Area
    EnterCriticalSection (&m_csSynchronized);

    // Let's load the Remote SSRC into the Receiver Report
    puchPayloadBuffer += LoadRemoteSSRC((unsigned long *)puchPayloadBuffer);

    // Let's load the Loss Statistic
    puchPayloadBuffer +=
             LoadLossStatistics((unsigned long *)puchPayloadBuffer);

    // Let's load the Extended Highest Sequence
    puchPayloadBuffer +=
             LoadExtendedSequence((unsigned long *)puchPayloadBuffer);

    // Let's load the Jitter Statistics
    puchPayloadBuffer += LoadJitter((unsigned long *)puchPayloadBuffer);

    // Let's load the Report Timestamps
    puchPayloadBuffer += LoadReportTimes((unsigned long *)puchPayloadBuffer);

    // Leave Synchronized Area
    LeaveCriticalSection (&m_csSynchronized);

    // Set the report length
    ulReportLength = puchPayloadBuffer - puchReportBuffer;

    // Let's check to see whether we need to prepend a header to this Receiver
    // Report.  If so, let's call the RTCP Header base class's formatter.
    if(!bSRPresent)
    {
        // Let's slap a header on this report
        FormatRTCPHeader(puchReportBuffer,  // RTCP Report Buffer
                         FALSE,             // No Padding
                         1,                 // Receiver Count set to 0 for now
                         ulReportLength);   // Payload Length
    }

    // This packet is about to be sent out.  Let's clear the period
    // statistics before returning control to the caller.
    ResetPeriodCounts();

    return(ulReportLength);

}

/**
 *
 * Method Name:  ParseReceiverReport
 *
 *
 * Inputs:   boolean         bHeader          - TRUE => a header is included
 *           unsigned char  *puchReportBuffer
 *                                  - Buffer used to store the Receiver Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Number of octets processed
 *
 * Description: Processes a Receiver report using the buffer passed in by the
 *              caller.  The header flag shall identify whether the report is
 *              prepended with a header.
 *
 * Usage Notes: The header of the RTCP Report, if provided, shall be parsed
 *              by delegating to the base class.
 *
 *
 */
unsigned long
   CReceiverReport::ParseReceiverReport(bool bHeader,
                                        unsigned char *puchReportBuffer)
{

    unsigned char   *puchPayloadBuffer = puchReportBuffer;

    // Determine whether the header has been included.  If so, let's
    // extract the header information
    if(bHeader)
    {
        // Check whether the RTCP Header has been
        // correctly formed (Version, etc...).
        if(!ParseRTCPHeader(puchReportBuffer))
            return(GetReportLength());

        // Good header.  Let's bump the payload pointer and continue.
        puchPayloadBuffer += GetHeaderLength();
    }

    // Let's extract the  Remote SSRC from the Receiver Report
    puchPayloadBuffer +=
            ExtractRemoteSSRC((unsigned long *)puchPayloadBuffer);

    // Let's extract the Loss Statistic
    puchPayloadBuffer +=
            ExtractLossStatistics((unsigned long *)puchPayloadBuffer);

    // Let's extract the Extended Highest Sequence
    puchPayloadBuffer +=
            ExtractExtendedSequence((unsigned long *)puchPayloadBuffer);

    // Let's extract the Jitter Statistics
    puchPayloadBuffer += ExtractJitter((unsigned long *)puchPayloadBuffer);

    // Let's extract the Report Timestamps
    puchPayloadBuffer +=
            ExtractReportTimes((unsigned long *)puchPayloadBuffer);

    return(puchPayloadBuffer - puchReportBuffer);


}


/**
 *
 * Method Name:  UpdateJitter
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  This method shall use the current and previous packet send
 *               and receive time to calculate interarrival jitter.
 *               The jitter calculated shall be used to update cumulative
 *               jitter and average jitter statistics.
 *
 * Usage Notes:
 *
 */
void CReceiverReport::UpdateJitter(IRTPHeader *piRTPHeader)
{

    piRTPHeader->AddRef();

    // We will determine jitter by calculating the difference in time between
    // the current and last RTP packets send and receive times.
    unsigned long ulPreviousPacketSendTime    = m_ulLastPacketSendTime;
    unsigned long ulPreviousPacketReceiveTime = m_ulLastPacketReceiveTime;
    double dJitter;

    // Let's get the current RTP packet Receive Timestamp
    piRTPHeader->GetRTPTimestamp(&m_ulLastPacketSendTime);
    piRTPHeader->GetRecvTimestamp(&m_ulLastPacketReceiveTime);

    // Do not make the calculation if this is the first run through
    if(ulPreviousPacketSendTime == 0 && ulPreviousPacketReceiveTime == 0)
    {
        piRTPHeader->Release();
        return;
    }

    // Let's calculate packet jitter
    dJitter  = ((double)m_ulLastPacketReceiveTime -
                                   (double)ulPreviousPacketReceiveTime);
    dJitter -= ((double)m_ulLastPacketSendTime -
                                      (double)ulPreviousPacketSendTime);

    // Let's check for a wrapping condition
    if(dJitter < 0)
        dJitter = -dJitter;

    // Let's update the mean interarrival jitter
    dJitter -= (double)m_ulMeanJitter;
    dJitter /= REDUX_RATIO;
    m_ulMeanJitter += (unsigned long)dJitter;

    piRTPHeader->Release();
}

/**
 *
 * Method Name:  UpdateSequence
 *
 * Inputs:    IRTPHeader *piRTPHeader
 *                          - RTP Header of a recently received RTP packet
 *
 * Outputs:   None
 *
 * Returns:   None
 *
 * Description:  This method shall queue the RTP Header to a packet sequencing
 *               list and update the Highest Sequence Number statistic.  It
 *               shall then evaluate the sequence list to determine whether
 *               any packets have been lost.  Detection of one or more lost
 *               packets shall cause the UpdatePacketLoss() to be called with
 *               the number of packets lost.
 *
 * Usage Notes:
 *
 */
void CReceiverReport::UpdateSequence(IRTPHeader *piRTPHeader)
{
    unsigned long ulSequenceNo, ulSeqNoDelta;
    static unsigned long ulBadSequenceNo = RTP_SEQ_MOD + 1;
    static unsigned long ulPacketProbation = MIN_SEQUENTIAL;

    piRTPHeader->AddRef();

    // Get the Sequence Number from the header
    ulSequenceNo = piRTPHeader->GetSequenceNo();

    // Calculate difference in sequence numbers
    ulSeqNoDelta = ulSequenceNo - m_ulLastSequenceNo;

    // Source is not valid until an acceptable number of packets
    // with sequential sequence numbers have been received
    if (ulPacketProbation != 0)
    {
        // Is the packet received in sequence?
        if (ulSequenceNo == m_ulLastSequenceNo + 1)
        {
            // Let's decrement the probationary count
            ulPacketProbation--;
        }
        else
        {
            // Reset the probationary period
            ulPacketProbation = MIN_SEQUENTIAL;
            ResetPeriodCounts();
        }

    }

    // This RTP session has survived probation and
    // is being tracked statistically.
    else if (ulSeqNoDelta < ((unsigned long) (MAX_DROPOUT)));

    else if (ulSeqNoDelta <= RTP_SEQ_MOD - MAX_MISORDER)
    {
/*
        if (mTotalWarnings++ < 60)
            osPrintf(" UpdateSequence() ==> ulSeqNoDelta"
                             " <= RTP_SEQ_MOD - MAX_MISORDER\n"
                             "     // %lu <= (%d - %d)\n",
                             ulSeqNoDelta, RTP_SEQ_MOD, MAX_MISORDER);
*/
        // The sequence number made a very large jump
        if (ulSequenceNo == ulBadSequenceNo)
        {

             // Two sequential packets -- assume that the other side
             // restarted without telling us so just re-sync
             // (i.e., pretend this was the first packet).
             ResetPeriodCounts();
             ulBadSequenceNo = RTP_SEQ_MOD + 1;

        }
        else
        {
            ulBadSequenceNo = (ulSequenceNo + 1) & (RTP_SEQ_MOD-1);
            piRTPHeader->Release();
            return;
        }
    }
    else
    {
        // Duplicate or reordered packet
    }

    // Update Sequence Number attributes
    UpdateSequenceNumbers(ulSequenceNo);

    // Increment Packet Counts
    IncrementPacketCounters();

    piRTPHeader->Release();

    return;
}

/**
 *
 * Method Name:  UpdateSequenceNumbers
 *
 * Inputs:       unsigned long ulSequenceNo
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  This method shall calculate the highest extended sequence
 *               number.  This value shall be a combination of the highest
 *               sequence number received (low 16 bits) and the number of
 *               sequence number cycles (high 16 bits).
 *
 * Usage Notes:
 *
 */
void CReceiverReport::UpdateSequenceNumbers(unsigned long ulSequenceNo)
{

    // Check the sequence number saved to determine whether we are
    // about ready to roll over.
    if(ulSequenceNo < m_ulLastSequenceNo)
    {
        m_ulSequenceWraps++;
    }

    // Store the new sequence number so that it may be used to check for
    // continuity between packets.
    m_ulLastSequenceNo = ulSequenceNo;

 // Create the new highest sequence number
    unsigned long ulHighestSequenceNo =
                         (m_ulSequenceWraps << SEQUENCE_SHIFT) + ulSequenceNo;
    if(ulHighestSequenceNo > m_ulHighestSequenceNo)
        m_ulHighestSequenceNo = ulHighestSequenceNo;

    // Store the first highest packet sequence number received for the
    // reporting period so that we might arrive at some approximation
    // of expect and actual packets received.
    if(m_ulPeriodPacketCount == 0)
    {
        m_ulFirstSequenceNo = m_ulHighestSequenceNo;
    }
}

/**
 *
 * Method Name:  IncrementPacketCounters
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  This method shall increment both the period and cumulative
 *               packet counters.
 *
 * Usage Notes:
 *
 */
void CReceiverReport::IncrementPacketCounters(void)
{

    // Let's increment both the period and total packet counters
    m_ulTotalPacketCount++;
    m_ulPeriodPacketCount++;
    mTotalPackets++;
    if (0 == (mTotalPackets & ((1<<11)-1))) mTotalWarnings = 0;
}


/**
 *
 * Method Name:  UpdateLostPackets
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  This method shall update the packet loss counts for the
 *               period and the session as well as calculate the fractional
 *               packet loss for the period.
 *
 * Usage Notes:
 *
 */
void CReceiverReport::UpdateLostPackets(void)
{

    // Calculate the fractional packet loss over two reporting periods
    long lExpectedCount    = m_ulHighestSequenceNo - m_ulFirstSequenceNo;
    long lExpectedInterval = lExpectedCount + m_ulLastPeriodExpectedCount;
    long lReceivedInterval = m_ulPeriodPacketCount + m_ulLastPeriodPacketCount;

    double dLostInterval =
                         (double)lExpectedInterval - (double)lReceivedInterval;
    if (lExpectedInterval == 0 || dLostInterval <= 0)
        m_ulFractionalLoss = 0;
    else
    {
        dLostInterval /= (double)lExpectedInterval;
        dLostInterval *= (double)FIXED_CONVERSION;
        m_ulFractionalLoss = (unsigned long) dLostInterval;
    }


    // Add the period loss statistics to the cumulative session totals
    long lPacketDiff = lExpectedCount - m_ulPeriodPacketCount;
    m_ulCumulativeLoss = ((long)m_ulCumulativeLoss + lPacketDiff < 0) ?
                                       0 : (m_ulCumulativeLoss + lPacketDiff);

    // Save last period expected and actual packet counts
    m_ulLastPeriodExpectedCount = (unsigned long)lExpectedCount;
    m_ulLastPeriodPacketCount   = m_ulPeriodPacketCount;
    m_ulFirstSequenceNo         = m_ulHighestSequenceNo;

}


/**
 *
 * Method Name:  LoadRemoteSSRC
 *
 * Inputs:    unsigned long *pulPayloadBuffer - Payload buffer for loading data
 *
 * Outputs:   None
 *
 * Returns:   unsigned long - the number of octets written into the buffer
 *
 * Description:  This method shall load the SSRC ID of the reportee into
 *               the payload buffer.
 *
 * Usage Notes:
 *
 */
unsigned long CReceiverReport::LoadRemoteSSRC(unsigned long *pulPayloadBuffer)
{

    // Load the Remote SSRC
    *pulPayloadBuffer = htonl(m_ulRemoteSSRC);

    return(sizeof(m_ulRemoteSSRC));

}

/**
 *
 * Method Name:  LoadLossStatistics
 *
 * Inputs:    unsigned long *pulPayloadBuffer - Payload buffer for loading data
 *
 * Outputs:   None
 *
 * Returns:   unsigned long - the number of octets written into the buffer
 *
 * Description:  This method shall load the fractional and cumulative loss
 *               statistics into the payload buffer.
 *
 * Usage Notes:
 *
 */
unsigned long CReceiverReport::LoadLossStatistics(
                                           unsigned long *pulPayloadBuffer)
{

    // These statistics need to be calculated at period end.
    // So let's calculate them first.
    UpdateLostPackets();

    // Load the fractional and cumulative loss statistics
    *pulPayloadBuffer = htonl(
                    (((unsigned long)m_ulFractionalLoss) & FRACTIONAL_MASK) +
                    ((m_ulCumulativeLoss & CUMULATIVE_MASK) << LOSS_SHIFT));

    // Load the values into cached copies so that they will not change
    m_ulCachedFractionalLoss = m_ulFractionalLoss;
    m_ulCachedCumulativeLoss = m_ulCumulativeLoss;

    return(sizeof(m_ulCumulativeLoss));

}

/**
 *
 * Method Name:  LoadExtendedSequence
 *
 * Inputs:    unsigned long *pulPayloadBuffer - Payload buffer for loading data
 *
 * Outputs:   None
 *
 * Returns:   unsigned long - the number of octets written into the buffer
 *
 * Description:  This method shall load the extended highest sequence number.
 *               This is the number of times the 16 bit sequence number has
 *               wrapped shifted into the high 16 bits with the actual
 *               sequence number in the low 16 bits.
 *
 * Usage Notes:
 *
 */
unsigned long CReceiverReport::LoadExtendedSequence(
                                          unsigned long *pulPayloadBuffer)
{

    // Load the extended sequence number statistics
    *pulPayloadBuffer = htonl(m_ulHighestSequenceNo);

    // Load the statistic into a cached copy
    m_ulCachedHighestSequenceNo = m_ulHighestSequenceNo;

    return(sizeof(m_ulHighestSequenceNo));

}

/**
 *
 * Method Name:  LoadJitter
 *
 * Inputs:    unsigned long *pulPayloadBuffer - Payload buffer for loading data
 *
 * Outputs:   None
 *
 * Returns:   unsigned long - the number of octets written into the buffer
 *
 * Description:  This method shall load the RTP packet interarrival jitter
 *               statistic.  This is an estimate of the statistical variance
 *               of the RTP data packet interarrival time, measured in
 *               timestamp units and expressed as an unsigned integer.
 *
 * Usage Notes:
 *
 */
unsigned long CReceiverReport::LoadJitter(unsigned long *pulPayloadBuffer)
{

    // Load the interarrival jitter calculation
    *pulPayloadBuffer = htonl(m_ulMeanJitter);

    // Load the statistic into a cached copy
    m_ulCachedMeanJitter = m_ulMeanJitter;

    return(sizeof(unsigned long));

}

/**
 *
 * Method Name:  LoadReportTimes
 *
 * Inputs:    unsigned long *pulPayloadBuffer - Payload buffer for loading data
 *
 * Outputs:   None
 *
 * Returns:   unsigned long - the number of octets written into the buffer
 *
 * Description:  This method shall load the timestamp of the last SR Report
 *               received from an SSRC in addition to loading the delay between
 *               our receiving this SR packet and responding with an RR packet.
 *
 * Usage Notes:
 *
 */
unsigned long CReceiverReport::LoadReportTimes(
                                              unsigned long *pulPayloadBuffer)
{
    double dRRTransmitTime;

    // Load the timestamp extracted from the last SR packet received
    // from a source
    *pulPayloadBuffer = htonl(m_ulLastSRTimestamp);
     pulPayloadBuffer++;

    // Let's check whether we have received our first SR from the FE.
    // We will forgo the delay calculation until the first SR is received.
    if(m_ulLastSRTimestamp)
    {

#ifdef WIN32
        struct _timeb stLocalTime;

        // Get the LocalTime expressed as seconds since 1/1/70 (UTC)
        _ftime(&stLocalTime);

        // Load second granularity of system time
        dRRTransmitTime = (double)stLocalTime.time;

        // Add microsecond granularity of system time
        dRRTransmitTime +=
            (double)(((double)stLocalTime.millitm * MILLI2MICRO) / MICRO2SEC);

#elif defined(__pingtel_on_posix__)
    struct timeval tv;

    gettimeofday(&tv, NULL);
    dRRTransmitTime = (double) tv.tv_sec;
    dRRTransmitTime += tv.tv_usec / MICRO2SEC;

#else
        struct timespec stLocalTime;

        // Make a call to VxWorks to get this timestamp
        if (clock_gettime (CLOCK_REALTIME, &stLocalTime) == ERROR)
        {
            osPrintf("**** FAILURE **** LoadTimestamps() - clock_gettime failure\n");
            stLocalTime.tv_sec = 0;
            stLocalTime.tv_nsec = 0;
        }

        // Load second granularity of system time
        dRRTransmitTime = (double)stLocalTime.tv_sec;

        // Add microsecond granularity of system time
        dRRTransmitTime +=
            (double)(((double)stLocalTime.tv_nsec / MILLI2MICRO) / MICRO2SEC);

#endif

        // The delay is calculated as the difference beteen the RR Transmit
        // time and the SR receive time expressed in delay units.
        dRRTransmitTime -= m_dLastSRRcvdTimestamp;
        dRRTransmitTime *= DELAY_UNITS;
        m_ulSRDelay = (unsigned long)dRRTransmitTime;
    }

    // Load the delay calculation into the payload buffer. We are expressing
    // our results in units of 1 / 65536.  Why, I don't know....
    // It probably seemed like the thing to do at the time.
    *pulPayloadBuffer = htonl(m_ulSRDelay);

    // Load the statistic into a cached copy
    m_ulCachedSRDelay         = m_ulSRDelay;
    m_ulCachedLastSRTimestamp = m_ulLastSRTimestamp;

    return(sizeof(m_ulLastSRTimestamp) + sizeof(m_ulSRDelay));

}


/**
 *
 * Method Name:  ExtractRemoteSSRC
 *
 * Inputs:    unsigned long *pulPayloadBuffer - Payload buffer for loading data
 *
 * Outputs:   None
 *
 * Returns:   unsigned long - the number of octets read from the buffer
 *
 * Description:  This method shall store the SSRC ID of the reportee found
 *               in the payload buffer.
 *
 * Usage Notes:
 *
 */
unsigned long CReceiverReport::ExtractRemoteSSRC(
                                              unsigned long *pulPayloadBuffer)
{

    // Store the Remote SSRC from the payload buffer as an attribute
    m_ulRemoteSSRC = ntohl(*pulPayloadBuffer);

    return(sizeof(m_ulRemoteSSRC));

}

/**
 *
 * Method Name:  ExtractLossStatistics
 *
 * Inputs:    unsigned long *pulPayloadBuffer - Payload buffer for loading data
 *
 * Outputs:   None
 *
 * Returns:   unsigned long - the number of octets read from the buffer
 *
 * Description:  This method shall store the fractional and cumulative loss
 *               statistics from the payload buffer.
 *
 * Usage Notes:
 *
 */
unsigned long CReceiverReport::ExtractLossStatistics(
                                             unsigned long *pulPayloadBuffer)
{

    unsigned long ulLossStatistics = ntohl(*pulPayloadBuffer);

    // Extract the fractional loss and cumulative loss statistics
    m_ulFractionalLoss = ulLossStatistics & FRACTIONAL_MASK;
    m_ulCachedFractionalLoss =  m_ulFractionalLoss;
    m_ulCumulativeLoss = (ulLossStatistics >> LOSS_SHIFT) & CUMULATIVE_MASK;
    m_ulCachedCumulativeLoss = m_ulCumulativeLoss;

    return(sizeof(m_ulCumulativeLoss));

}

/**
 *
 * Method Name:  ExtractExtendedSequence
 *
 * Inputs:    unsigned long *pulPayloadBuffer - Payload buffer for loading data
 *
 * Outputs:   None
 *
 * Returns:   unsigned long - the number of octets read from the buffer
 *
 * Description:  This method shall extract the extended highest sequence
 *               number.  This is the number of times the 16 bit sequence
 *               number has wrapped shifted into the high 16 bits with the
 *               actual sequence number in the low 16 bits.
 *
 * Usage Notes:
 *
 */
unsigned long CReceiverReport::ExtractExtendedSequence(
                                              unsigned long *pulPayloadBuffer)
{

    // Extract the highest sequence number statistics
    m_ulHighestSequenceNo = ntohl(*pulPayloadBuffer);
    m_ulCachedHighestSequenceNo = m_ulHighestSequenceNo;

    return(sizeof(m_ulHighestSequenceNo));

}

/**
 *
 * Method Name:  ExtractJitter
 *
 * Inputs:    unsigned long *pulPayloadBuffer - Payload buffer for loading data
 *
 * Outputs:   None
 *
 * Returns:   unsigned long - the number of octets read from the buffer
 *
 * Description:  This method shall extract the RTP packet interarrival jitter
 *               statistic.  This is an estimate of the statistical variance
 *               of the RTP data packet interarrival time, measured in
 *               timestamp units and expressed as an unsigned integer.
 *
 * Usage Notes:
 *
 */
unsigned long CReceiverReport::ExtractJitter(unsigned long *pulPayloadBuffer)
{

    // Load the interarrival jitter calculation
    m_ulMeanJitter = ntohl(*pulPayloadBuffer);
    m_ulCachedMeanJitter = m_ulMeanJitter;

    return(sizeof(m_ulMeanJitter));

}

/**
 *
 * Method Name:  ExtractReportTimes
 *
 * Inputs:    unsigned long *pulPayloadBuffer - Payload buffer for loading data
 *
 * Outputs:   None
 *
 * Returns:   unsigned long - the number of octets read from the buffer
 *
 * Description:  This method shall extract the timestamp of the last SR Report
 *               received from an SSRC in addition to loading the delay between
 *               our receiving this SR packet and responding with an RR packet.
 *
 * Usage Notes:
 *
 */
unsigned long CReceiverReport::ExtractReportTimes(
                                               unsigned long *pulPayloadBuffer)
{

    // Extracted the last SR packet timestamp received from the source
    m_ulLastSRTimestamp = ntohl(*pulPayloadBuffer);
    m_ulCachedLastSRTimestamp = m_ulLastSRTimestamp;

    // Extract the delay calculation from the payload buffer.
    pulPayloadBuffer++;
    m_ulSRDelay = ntohl(*pulPayloadBuffer);
    m_ulCachedSRDelay = m_ulSRDelay;

    return(sizeof(m_ulLastSRTimestamp) + sizeof(m_ulSRDelay));

}


/**
 *
 * Method Name:  ResetPeriodCounts
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  This method shall reset all period counters and accumulators.
 *
 * Usage Notes:
 *
 */
void CReceiverReport::ResetPeriodCounts(void)
{

    m_ulPeriodPacketCount       = 0;
    m_ulPeriodPacketLoss        = 0;
    m_ulFractionalLoss          = 0;

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
 * Description:  This method shall reset all receiver report statistics.
 *               A reset shall occur when the SSRC ID is reset due to a
 *               collision with a participating source.
 *
 * Usage Notes:
 *
 */
void CReceiverReport::ResetStatistics(void)
{

    // Reset all report statistics back to their initial state
    m_ulTotalPacketCount        = 0;
    m_ulPeriodPacketCount       = 0;
    m_ulPeriodPacketLoss        = 0;
    m_ulFractionalLoss          = 0;
    m_ulCumulativeLoss          = 0;
    m_ulFirstSequenceNo         = 0;
    m_ulLastSequenceNo          = 0;
    m_ulSequenceWraps           = 0;
    m_ulHighestSequenceNo       = 0;
    m_ulLastPacketSendTime      = 0;
    m_ulLastPacketReceiveTime   = 0;
    m_ulMeanJitter              = 0;
    m_ulSRDelay                 = 0;
    m_ulLastSRTimestamp         = 0;
    m_ulLastPeriodExpectedCount = 0;
    m_ulLastPeriodPacketCount   = 0;

    // Temporary Data Members
    m_ulCachedFractionalLoss    = 0;
    m_ulCachedCumulativeLoss    = 0;
    m_ulCachedHighestSequenceNo = 0;
    m_ulCachedMeanJitter        = 0;
    m_ulCachedLastSRTimestamp   = 0;
    m_ulCachedSRDelay           = 0;

}
#endif /* INCLUDE_RTCP ] */
