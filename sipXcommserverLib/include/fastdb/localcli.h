//-< LOCALCLI.H >----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Jun-2002  K.A. Knizhnik  * / [] \ *
//                          Last update: 20-Jun-2002  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Header file for local C interface to database
//-------------------------------------------------------------------*--------*

#include "fastdb.h"
#include "compiler.h"
#include "cli.h"
#include "cliproto.h"
#include "array.h"

inline int map_type(dbFieldDescriptor* fd) { 
    return (fd->type < dbField::tpArray) 
	? fd2cli_type_mapping[fd->type] 
	: (fd->type == dbField::tpArray && fd->components->type < dbField::tpArray)
	  ? cli_array_of_oid + fd2cli_type_mapping[fd->components->type] 
	  : cli_unknown;
}


struct parameter_binding {
    parameter_binding* next;
    char*   name;
    int     var_type;
    int     var_len;
    void*   var_ptr;
};

struct column_binding {
    column_binding*    next;
    dbFieldDescriptor* field;
    char*              name;
    int                var_type;
    int*               var_len;
    void*              var_ptr;
    cli_column_get_ex  get_fnc;
    cli_column_set_ex  set_fnc;
};

struct session_desc;

struct statement_desc {
    int                id;
    statement_desc*    next;
    dbQuery            query;
    dbAnyCursor        cursor;
    dbTableDescriptor* table;
    column_binding*    columns;
    parameter_binding* params;
    session_desc*      session;
    bool               first_fetch;
    bool               for_update;
    bool               prepared;
    bool               updated;
    cli_oid_t          oid;
    int                n_params;
    int                n_columns;
    int                n_autoincremented_columns;
    int                param_size;
    void*              record_struct;
    dbSmallBuffer      sql;

    statement_desc(int id, statement_desc* next) 
    {
	this->id = id;
	this->next = next;
    } 
    statement_desc() {}
};


class sql_scanner {
  private:
    char*  p;
    char*  ident;

  public:
    int     get();

    char* current_position() { 
	return p;
    }

    char* identifier() { 
	return ident;
    }

    sql_scanner(char* sql) {
	p = sql;
    }
};

struct session_desc {
    int              id;
    char*            name;
    session_desc*    next;
    statement_desc*  stmts;
    dbDatabase*      db;
    dbMutex          mutex;
    dbTableDescriptor* dropped_tables;
    dbTableDescriptor* existed_tables;
    
    session_desc(int id, session_desc* next) {
	this->id = id;
	this->next = next;
    }
    session_desc() {}
};

template<class T>
class fixed_size_object_allocator { 
  protected:
    T*          free_chain;
    dbMutex     mutex;

  public:
    T* allocate() {
	dbCriticalSection cs(mutex);
	T* obj = free_chain;
	if (obj == NULL) {
	    obj = new T();
	} else { 
	    free_chain = obj->next;
	}
	return obj;
    }

    void free(T* desc) {
	dbCriticalSection cs(mutex);
	desc->next = free_chain;
	free_chain = desc;
    }

    fixed_size_object_allocator() {
	free_chain = NULL;
    }

    ~fixed_size_object_allocator() { 
	T *obj, *next;
	for (obj = free_chain; obj != NULL; obj = next) { 
	    next = obj->next;
	    delete obj;
	}
    }
};

template<class T>
class descriptor_table : public fixed_size_object_allocator<T> {
  protected:
    T**         table;
    int         descriptor_table_size;

  public:
    descriptor_table() {
	int i;
	descriptor_table_size = 16;
	table = new T*[descriptor_table_size];
	T* next = NULL;
	for (i = 0; i < descriptor_table_size; i++) {
	    table[i] = next = new T(i, next);
	}
	this->free_chain = next;
    }

    ~descriptor_table() { 
	delete[] table;
    }

    T* get(int desc) {
	dbCriticalSection cs(this->mutex);
	return (desc >= descriptor_table_size) ? (T*)0 : table[desc];
    }

    T* allocate() {
	dbCriticalSection cs(this->mutex);
	if (this->free_chain == NULL) {
	    int i, n;
	    T** desc = new T*[descriptor_table_size * 2];
	    memcpy(desc, table, descriptor_table_size*sizeof(T*));
	    delete[] table;
	    table = desc;
	    T* next = NULL;
	    for (i = descriptor_table_size, n = i*2; i < n; i++) {
		table[i] = next = new T(i, next);
	    }
	    this->free_chain = next;
	    descriptor_table_size = n;
	}
	T* desc = this->free_chain;
	this->free_chain = desc->next;
	return desc;
    }
};

class FASTDB_DLL_ENTRY dbCLI { 
  private:
    fixed_size_object_allocator<column_binding> column_allocator;
    fixed_size_object_allocator<parameter_binding> parameter_allocator;
    
    descriptor_table<session_desc>   sessions;
    descriptor_table<statement_desc> statements;

    session_desc* active_session_list;
    
    dbMutex sessionMutex;    
    
  public:
    static dbCLI instance;

    dbCLI() { 
	active_session_list = NULL; 
    }

    int create_session(char const* databasePath,
		       char const* filePath,
		       unsigned    transactionCommitDelay, 
		       int         openAttr, 
		       size_t      initDatabaseSize,
                       size_t      extensionQuantum,
                       size_t      initIndexSize,
                       size_t      fileSizeLimit);   


    int create_replication_node(int         nodeId,
                                int         nServers,
                                char*       nodeNames[],
                                char const* databaseName, 
                                char const* filePath, 
                                int         openAttr, 
                                size_t      initDatabaseSize,
                                size_t      extensionQuantum,
                                size_t      initIndexSize,
                                size_t      fileSizeLimit);

    int create_statement(int session, char const* sql);

    int bind_parameter(int         statement,
		       char const* param_name,
		       int         var_type,
		       void*       var_ptr);

    int bind_column(int         statement,
		    char const* column_name,
		    int         var_type,
		    int*        var_len,
		    void*       var_ptr);

    int bind_array_column(int               statement,
			  char const*       column_name,
			  int               var_type,
			  void*             var_ptr,
			  cli_column_set_ex set,
			  cli_column_get_ex get);
	
    int fetch(int statement, int for_update);

    int fetch_columns(statement_desc* stmt);
    int store_columns(char* buf, statement_desc* stmt);

    int insert(int statement, cli_oid_t* oid);
    int update(int statement);

    int freeze(int statement);
    int unfreeze(int statement);

    int get_first(int statement);
    int get_last(int statement);
    int get_next(int statement);
    int get_prev(int statement);
    int skip(int statement, int n);
    int seek(int statement, cli_oid_t oid);

    cli_oid_t get_current_oid(int statement);
    int free_statement(int statement);
    int free_statement(statement_desc* stmt);
    
    int commit(int session);
    int precommit(int session);
    int abort(int session);

    int remove(int statement);

    int describe(int session, char const* table, cli_field_descriptor** fields);
    int show_tables(int session, cli_table_descriptor** tables);

    int match_columns(char const* table_name, statement_desc* stmt);

    int create_table(int session, char const* tableName, int nColumns, 
		     cli_field_descriptor* columns);

    int drop_table(int session, char const* tableName);

    int alter_index(int session, char const* tableName, char const* fieldName, int newFlags);

    cli_error_handler set_error_handler(int session, cli_error_handler new_handler);

    int attach(int session);
    int detach(int session, int detach_mode);

    int get_database_state(int session, cli_database_monitor* monitor);

    int close(int session);

    int prepare_query(int session, char const* query);
    int execute_query(int statement, int for_update, void* record_struct, va_list params);
    int insert_struct(int session, char const* table_name, void* record_struct, cli_oid_t* oid);

    static int create_table(dbDatabase* db, char const* tableName, int nColumns, 
                            cli_field_descriptor* columns);

    static int alter_index(dbDatabase* db, char const* tableName, char const* fieldName, int newFlags);
};
