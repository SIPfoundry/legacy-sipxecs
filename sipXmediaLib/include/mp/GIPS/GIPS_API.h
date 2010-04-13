#ifdef HAVE_GIPS /* [ */
#include "GIPS/Vendor_GIPS_API.h"
#define GIPS_API_VERSION "gips_2.0"

#define        JB_create NETEQ_GIPS_10MS16B_create
#define          JB_init NETEQ_GIPS_10MS16B_init
#define          JB_free NETEQ_GIPS_10MS16B_free
#define JB_initCodepoint NETEQ_GIPS_10MS16B_initCodepoint
#define         JB_RecIn NETEQ_GIPS_10MS16B_RecIn
#define        JB_RecOut NETEQ_GIPS_10MS16B_RecOut

#define        JB_EXTERN GIPS_DLLEXPORT
#endif /* HAVE_GIPS ] */
