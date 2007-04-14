//-< EXCEPTION.H >---------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     3-Oct-99 Sebastiano Suraci  * / [] \ *
//                          Last update: 5-Oct-99 K.A. Knizhnik      * GARRET *
//-------------------------------------------------------------------*--------*
// Database exception 
//-------------------------------------------------------------------*--------*

#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

class FASTDB_DLL_ENTRY dbException  
{
   protected:
     int   err_code;
     char* msg;	
     int   arg;

   public:
     dbException(int p_err_code, char const* p_msg = NULL, int p_arg = 0)
     : err_code (p_err_code),
       msg (NULL),
       arg (p_arg)
     { 
         if (p_msg != NULL) { 
	     msg = new char[strlen(p_msg)+1]; 
	     strcpy(msg, p_msg);
	 }
     }

     dbException(dbException const& ex) { 
	 err_code = ex.err_code;
	 arg = ex.arg;
         if (ex.msg != NULL) { 
	     msg = new char[strlen(ex.msg)+1]; 
	     strcpy(msg, ex.msg);
	 } else { 
	     msg = NULL;
	 }
     }	

     ~dbException() { 
         delete[] msg;
     }

     int   getErrCode() const { return err_code; }
     char* getMsg() const     { return msg; }
     long  getArg() const     { return arg; }
};

#endif
