//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _SenderReport_h
#define _SenderReport_h

#include "rtcp/RtcpConfig.h"

//  Includes
#ifndef WIN32
#include <time.h>
#endif

#include "RTCPHeader.h"
#include "ISenderReport.h"
#include "IGetSenderStatistics.h"
#include "ISetSenderStatistics.h"
#include "ISetReceiverStatistics.h"



/**
 *
 * Class Name:  CSenderReport
 *
 * Inheritance: CRTCPHeader          - RTCP Report Header Base Class
 *
 *
 * Interfaces:  ISenderReport        - RTCP Sender Report Interface
 *
 * Description: The CSenderReport Class coordinates the processing and
 *              generation of RTCP sender reports associated with either an
 *              inbound or outbound RTP connection.
 *
 * Notes:       CSenderReport is derived from CBaseClass which provides basic
 *              Initialization and reference counting support.
 *
 */
class CSenderReport:
       public CBaseClass,           // Inherits the CBaseClass implementation
       public CRTCPHeader,          // Inherits the CRTCPHeader implementation
       public ISenderReport,        // Sender Report Control Interface
       public IGetSenderStatistics, // Get Sender Statistics Interface
       public ISetSenderStatistics  // Set Sender Statistics Interface
{

//  Public Methods
public:

/**
 *
 * Method Name:  CSenderReport() - Constructor
 *
 *
 * Inputs:   unsigned long          ulSSRC    - The Identifier for this source
 *           ISetReceiverStatistics *piSetStatistics
 *                                      - Interface for setting receiver stats
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
    CSenderReport(unsigned long ulSSRC,
                  ISetReceiverStatistics *piSetStatistics=NULL);



/**
 *
 * Method Name: ~CSenderReport() - Destructor
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: Shall deallocated and/or release all resources which was
 *              acquired over the course of runtime.
 *
 * Usage Notes:
 *
 *
 */
    ~CSenderReport(void);


/**
 *
 * Method Name:  IncrementCounts
 *
 *
 * Inputs:   unsigned long  ulOctetCount    -   RTP Octets Sent
 *
 * Outputs:  None
 *
 * Returns:  void
 *
 * Description:  The IncrementCounts method shall add the number of octets
 *               passed to the cumulative octet count stored as an attribute to
 *               this object.  Each call to IncrementCounts() shall also
 *               increment the packet count by 1.
 *
 * Usage Notes:
 *
 */
    void IncrementCounts(unsigned long ulOctetCount);

/**
 *
 * Method Name:  SetRTPTimestamp
 *
 *
 * Inputs:   unsigned long  ulRandomOffset  -  Random Offset for RTP Timestamp
 *           unsigned long  ulSamplesPerSecond  -  Number of sample per second
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
    void SetRTPTimestamp(unsigned long ulRandomOffset,
                         unsigned long ulSamplesPerSecond = SAMPLES_PER_SEC);



/**
 *
 * Method Name:  GetSenderStatistics
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned long   *ulPacketCount   - Sender Packet Count
 *           unsigned long   *ulOctetCount    - Sender Octet Count
 *
 * Returns:  void
 *
 * Description: Returns the packet and octet counts values stored as attributes.
 *
 * Usage Notes:
 *
 *
 *
 */
    void GetSenderStatistics(unsigned long *ulPacketCount,
                             unsigned long *ulOctetCount);

/**
 *
 * Method Name:  GetSSRC
 *
 *
 * Inputs:   None
 *
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - The SSRC of the Bye Report
 *
 * Description: Returns the SSRC Associated with the Bye Report.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long GetSSRC(void);


/**
 *
 * Method Name:  SetSSRC
 *
 *
 * Inputs:   unsigned long   ulSSRC   - Source ID
 *
 * Outputs:  None
 *
 * Returns:  void
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
    virtual void SetSSRC(unsigned long ulSSRC);

/**
 *
 * Method Name:  WasMediaSent
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  bool
 *
 * Description:  A method to determine whether media has been sent out since
 *               the last reporting period.  This will determine whether a
 *               Sender Report or Receiver Report is in order.
 *
 * Usage Notes:
 *
 */
    bool WasMediaSent(void);


/**
 *
 * Method Name:  FormatSenderReport
 *
 *
 * Inputs:   unsigned long  ulBufferSize
 *                                - Optional length allocated for the buffer
 *
 * Outputs:  unsigned char *puchReportBuffer
 *                                - Buffer to hold the Sender Report
 *
 * Returns:  unsigned long  - number of octets written into the buffer.
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
    unsigned long FormatSenderReport(unsigned char *puchReportBuffer,
                                     unsigned long ulBufferSize=0);


/**
 *
 * Method Name:  ParseSenderReport
 *
 *
 * Inputs:   unsigned char *puchReportBuffer
 *                                  - Buffer containing the Sender Report
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
unsigned long ParseSenderReport(unsigned char *puchReportBuffer);

/**
 *
 * Macro Name:  DECLARE_IBASE_M
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: This implements the IBaseClass functions used and exposed by
 *              derived classes.
 *
 * Usage Notes:
 *
 *
 */
DECLARE_IBASE_M

private:        // Private Methods


/**
 *
 * Method Name:  ResetStatistics
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  void
 *
 * Description:  This method shall reset all sedner report statistics.  A reset
 *              shall occur when the SSRC ID is reset due to a collision with
 *              a participating source.
 *
 * Usage Notes:
 *
 */
    void ResetStatistics(void);

/**
 *
 * Method Name:  LoadTimestamps
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned long *aulTimeStamps - Long word array in which to load
 *              the NTP/RTP timestamp
 *
 * Returns:  unsigned long                - Size of the data loaded
 *
 * Description: This method shall load the 64 bit NTP timestamp and the 32-bit
 *              RTP timestamp into the long word array passed as an argument
 *              to this call.
 *
 * Usage Notes:
 *
 */
    unsigned long LoadTimestamps(unsigned long *paulTimeStamps);



/**
 *
 * Method Name:  LoadSenderStats
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned long *aulSenderStats
 *                          - Long word array in which to load the statistics
 *
 * Returns:  unsigned long                 - Amount of data loaded
 *
 * Description:  This method shall retrieve the packet and octet counts.
 *
 * Usage Notes:
 *
 */
    unsigned long  LoadSenderStats(unsigned long *paulSenderStats);


/**
 *
 * Method Name:  ExtractTimestamps
 *
 *
 * Inputs:   unsigned long *paulTimestamps
 *                             - Array containing the NTP/RTP Timestamps
 *
 * Outputs:  None
 *
 * Returns:  unsigned long                 - Size of the data extracted
 *
 * Description:  This method shall extract the 64 bits of NTP time information
 *               and the 32-bits of RTP timestamp and store them both in
 *               respective report attributes.
 *
 * Usage Notes:
 *
 */
    unsigned long ExtractTimestamps(unsigned long *paulTimestamps);


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
 * Returns:  unsigned long                 - Amount of data extracted
 *
 * Description:  This method shall extract the packet and octet counts from the
 *               Sender Report.
 *
 * Usage Notes:
 *
 */
    unsigned long  ExtractSenderStats(unsigned long *aulSenderStats);



private:        // Private Data Members


/**
 *
 * Attribute Name:  m_piSetReceiverStatistics
 *
 * Type:            ISetReceiverStatistics
 *
 * Description:  Interface used by the Sender Report of an associated inbound
 *               RTP connection to update SR timestamp information within the
 *               corresponding Receiver Report.
 *
 */
      ISetReceiverStatistics *m_piSetReceiverStatistics;

/**
 *
 * Attribute Name:  m_ulPacketCount
 *
 * Type:            unsigned long
 *
 * Description:     This member shall store the cumlative sender packet count.
 *
 */
      unsigned long m_ulPacketCount;

/**
 *
 * Attribute Name:  m_ulOctetCount
 *
 * Type:            unsigned long
 *
 * Description:     This member shall store the cumlative sender octet count.
 *
 */
      unsigned long m_ulOctetCount;

/**
 *
 * Attribute Name:  m_bMediaSent
 *
 * Type:            bool
 *
 * Description:  This member shall identify whether media was sent over a
 *               particular reporting period.
 *
 */
      bool          m_bMediaSent;

/**
 *
 * Attribute Name:  m_aulNTPTimestamp
 *
 * Type:            unsigned long [2]
 *
 * Description:  This member shall store the 64 bit Network TimeStamp.
 *
 */
      unsigned long m_aulNTPTimestamp[2];


/**
 *
 * Attribute Name:  m_aulNTPStartTime
 *
 * Type:            unsigned long [2]
 *
 * Description:  This member shall store the 64 bit Network Starting TimeStamp
 *               for the RTP Stream.
 *
 */
      unsigned long m_aulNTPStartTime[2];

/**
 *
 * Attribute Name:  m_ulRTPTimestamp
 *
 * Type:            unsigned long
 *
 * Description:  This member shall store the 32 bit RTP TimeStamp identifying
 *               when a Sender Report was sent.
 *
 */
      unsigned long m_ulRTPTimestamp;

/**
 *
 * Attribute Name:  m_ulSamplesPerSecond
 *
 * Type:            unsigned long
 *
 * Description:  This member shall store the number of samples of media output
 *               per second.
 *
 */
      unsigned long m_ulSamplesPerSecond;


/**
 *
 * Attribute Name:  m_ulRandomOffset
 *
 * Type:            unsigned long
 *
 * Description:  This member shall store the random timestamp offset
 *               established for the RTP Stream.
 *
 */
    double          m_ulRandomOffset;

};


/**
 *
 * Method Name:  WasMediaSent
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  bool
 *
 * Description:  A method to determine whether media has been sent out since
 *               the last reporting period.  This will determine whether a
 *               Sender Report or Receiver Report is in order.
 *
 * Usage Notes:
 *
 */
inline bool CSenderReport::WasMediaSent(void)
{

    return(m_bMediaSent);

}

/**
 *
 * Method Name:  GetSSRC
 *
 *
 * Inputs:   None
 *
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - The SSRC of the Bye Report
 *
 * Description: Returns the SSRC Associated with the Bye Report.
 *
 * Usage Notes:
 *
 *
 */
inline unsigned long CSenderReport::GetSSRC(void)
{

    return(CRTCPHeader::GetSSRC());

}

#endif
