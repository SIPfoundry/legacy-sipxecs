//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef INCLUDE_RESPARSE_BZERO_H
#define INCLUDE_RESPARSE_BZERO_H
#ifdef __cplusplus
extern "C" {
#endif

/* _local added to function name to avoid name conflict --GAT */
void    bzero_local (void *buf, size_t len);
void* res_memset(void* dst0, int c0, size_t length);

#ifdef __cplusplus
}
#endif
#endif /* INCLUDE_RESPARSE_BZERO_H */
