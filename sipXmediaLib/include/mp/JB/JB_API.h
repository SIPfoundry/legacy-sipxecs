#ifndef _JB_API_H_
#define _JB_API_H_

#ifdef HAVE_GIPS /* [ */

#include "mp/GIPS/GIPS_API.h"

#else /* HAVE_GIPS ] [ */

#define JB_API_VERSION "SIPfoundry 1.0"
#define JB_EXTERN extern

#include "mp/JB/jb_typedefs.h"
#include "mp/MpTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

JB_EXTERN JB_ret JB_initCodepoint(JB_inst *JB_inst,
                              const JB_char *codec,
                              JB_size sampleRate, 
                              JB_code codepoint);
 

JB_EXTERN JB_ret JB_RecIn(JB_inst *JB_inst, 
                      JB_uchar* RTPpacket, 
                      JB_size RTPlength, 
                      JB_ulong timeStamp);

JB_EXTERN JB_ret JB_RecOut(JB_inst *JB_inst,
                      Sample *voiceSamples,
                      JB_size *length);

JB_EXTERN JB_ret G711A_Encoder(JB_size noOfSamples,
                         Sample* inBuff,
                         JB_uchar* codBuff, 
                         JB_size *size_in_bytes);

JB_EXTERN JB_ret G711U_Encoder(JB_size noOfSamples,
                         Sample* inBuff,
                         JB_uchar* codBuff, 
                         JB_size *size_in_bytes);

JB_EXTERN JB_ret JB_create(JB_inst **JB_inst);
JB_EXTERN JB_ret JB_init(JB_inst *JB_inst, JB_size fs);
JB_EXTERN JB_ret JB_free(JB_inst *JB_inst);


#ifdef __cplusplus
}
#endif

#endif /* HAVE_GIPS ] */
#endif /* _JB_API_H_ */
