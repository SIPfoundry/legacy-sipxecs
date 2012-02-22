/*
 *  Process watching mib group
 */
#ifndef _MIBGROUP_PROC_H
#define _MIBGROUP_PROC_H

config_require(util_funcs)

     void            init_regexp_proc(void);

     extern FindVarMethod var_extensible_regexp_proc;
     extern WriteMethod fixRegexpProcError;
     int             sh_count_regexp_procs(char *, pcre *);

/*
 * config file parsing routines 
 */
     void            regexp_proc_free_config(void);
     void            regexp_proc_parse_config(const char *, char *);
     void            regexp_procfix_parse_config(const char *, char *);

 struct myregexp_proc {
     char            name[STRMAX];
     pcre            *regexp;
     char            fixcmd[STRMAX];
     int             min;
     int             max;
     struct myregexp_proc  *next;
 };

#include "mibdefs.h"

#define PROCMIN 3
#define PROCMAX 4
#define PROCCOUNT 5

#endif                          /* _MIBGROUP_PROC_H */
