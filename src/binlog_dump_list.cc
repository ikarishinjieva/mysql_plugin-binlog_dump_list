#include <sql_show.h>
#include "binlog_dump_list.h"
#include <sql_repl.h>

extern I_List<THD> threads;
extern HASH slave_list;
extern const LEX_STRING command_name[];

class thread_info :public ilink
{
public:
  static void *operator new(size_t size)
  {
    return (void*) sql_alloc((uint) size);
  }
  static void operator delete(void *ptr __attribute__((unused)),
                              size_t size __attribute__((unused)))
  { TRASH(ptr, size); }

  THD *thd;
  ulong thread_id;
  const char *host, *status, *user;
  uint32 port;
};

//Copy from sql_show.cc
static const char *thread_state_info(THD *tmp)
{
	if (tmp->net.reading_or_writing) {
		if (tmp->net.reading_or_writing == 2)
			return "Writing to net";
		else if (tmp->command == COM_SLEEP)
			return "";
		else
			return "Reading from net";
	}
	else {
		if (tmp->proc_info)
			return tmp->proc_info;
		else if (tmp->mysys_var && tmp->mysys_var->current_cond)
			return "Waiting on cond";
		else
			return NULL;
	}
}

static int fill_is_table(THD *thd, TABLE_LIST *tables, COND *cond) {
	I_List<thread_info> thread_infos;

	//collect thread infos
	mysql_mutex_lock(&LOCK_thread_count);
    I_List_iterator<THD> iter(threads);
    THD *tmp;
    #ifdef DEBUG_PLUGIN
    	fprintf(stderr, "[binlog_dump_list] start fill IS table\n");
    #endif
    while (tmp=iter++) {
    	if (tmp->system_thread || tmp->vio_ok()) {
   			#ifdef DEBUG_PLUGIN
    			fprintf(stderr, "[binlog_dump_list] found one thread\n");
    		#endif
    		Security_context *tmp_sctx = tmp->security_ctx;
    		if (NULL == tmp_sctx) {
                fprintf(stderr, "[binlog_dump_list] got THD->security_ctx == NULL, maybe the plugin is not compatible with the mysqld, make sure the version is match, so is --with-debug\n");
				mysql_mutex_unlock(&LOCK_thread_count);
                return 1;
    		}

		    #ifdef DEBUG_PLUGIN
	    		fprintf(stderr, "[binlog_dump_list] thread command is %s\n", command_name[tmp->command].str);
		    #endif
    		if (NULL == tmp->command || 0 != strcmp("Binlog Dump", command_name[tmp->command].str)) {
    			continue;
    		}

    		//thread_id
    		thread_info *thd_info = new thread_info;
    		thd_info->thd = tmp;
    		thd_info->thread_id = tmp->thread_id;

			//host
			if (thd_info->host= (char*) thd->alloc(LIST_PROCESS_HOST_LEN+1)) {
	    		my_snprintf((char *) thd_info->host, LIST_PROCESS_HOST_LEN, "%s", tmp_sctx->host_or_ip);
			} else {
				fprintf(stderr, "[binlog_dump_list] cannot allocate thd_info->host\n");
				mysql_mutex_unlock(&LOCK_thread_count);
                return 1;
			}

			//user
        	thd_info->user= thd->strdup(tmp_sctx->user ? tmp_sctx->user :
                                    (tmp->system_thread ? "system user" : "unauthenticated user"));

			//status
        	mysql_mutex_lock(&tmp->LOCK_thd_data);
      		struct st_my_thread_var *mysys_var;
	        if (mysys_var= tmp->mysys_var)
	        	mysql_mutex_lock(&mysys_var->mutex); //thread_state_info will use mysys_var lock
        	thd_info->status= thread_state_info(tmp);
	        if (mysys_var)
	        	mysql_mutex_unlock(&mysys_var->mutex);
        	mysql_mutex_unlock(&tmp->LOCK_thd_data);

    		thread_infos.append(thd_info);
    	}
	}
	mysql_mutex_unlock(&LOCK_thread_count);

	//collect slave info

	mysql_mutex_lock(&LOCK_slave_list);
	for (uint i = 0; i < slave_list.records; ++i)
	{
	    #ifdef DEBUG_PLUGIN
			fprintf(stderr, "[binlog_dump_list] found one slave info\n");
		#endif
		SLAVE_INFO* si = (SLAVE_INFO*) my_hash_element(&slave_list, i);	
    	I_List_iterator<thread_info> iter(thread_infos);
    	thread_info *info;
    	while ((info = iter++) && (info->thd != si->thd));
    	if (NULL != info) {
    		info->port = (uint32) si->port;
    	}
	}
	mysql_mutex_unlock(&LOCK_slave_list);

	//fill IS table
	CHARSET_INFO *cs = system_charset_info;
	TABLE *table = tables->table;
	thread_info *thd_info;
	while (thd_info = thread_infos.get()) {
		table->field[0]->store(thd_info->thread_id);
		table->field[1]->store(thd_info->host, strlen(thd_info->host), cs);
		table->field[2]->store(thd_info->port);
		table->field[3]->store(thd_info->user, strlen(thd_info->user), cs);
		table->field[4]->store(thd_info->status, strlen(thd_info->status), cs);
		if (schema_table_store_record(thd, table))
			return 1;
	}

	return 0;
}

static int init_plugin(void *p) {
	ST_SCHEMA_TABLE *schema = (ST_SCHEMA_TABLE*) p;
	schema->fields_info = is_table_fields;
	schema->fill_table = fill_is_table;
	fprintf(stderr, "binlog_dump_list plugin started\n");

	//print sizeof(some struct), in case that the plugin is not compiled compatible with mysqld
    #ifdef DEBUG_PLUGIN
		fprintf(stderr, "[binlog_dump_list] sizeof(Statement)=%d\n", sizeof(Statement));
		fprintf(stderr, "[binlog_dump_list] sizeof(Open_tables_state)=%d\n", sizeof(Open_tables_state));
		fprintf(stderr, "[binlog_dump_list] sizeof(THD)=%d\n", sizeof(THD));
		fprintf(stderr, "[binlog_dump_list] sizeof(NET)=%d\n", sizeof(NET));
		fprintf(stderr, "[binlog_dump_list] sizeof(Protocol_text)=%d\n", sizeof(Protocol_text));
		fprintf(stderr, "[binlog_dump_list] sizeof(Protocol_binary)=%d\n", sizeof(Protocol_binary));
		fprintf(stderr, "[binlog_dump_list] sizeof(Protocol)=%d\n", sizeof(Protocol));
		fprintf(stderr, "[binlog_dump_list] sizeof(rand_struct)=%d\n", sizeof(rand_struct));
		fprintf(stderr, "[binlog_dump_list] sizeof(system_variables)=%d\n", sizeof(system_variables));
		fprintf(stderr, "[binlog_dump_list] sizeof(system_status_var)=%d\n", sizeof(system_status_var));
		fprintf(stderr, "[binlog_dump_list] sizeof(THR_LOCK_INFO)=%d\n", sizeof(THR_LOCK_INFO));
		fprintf(stderr, "[binlog_dump_list] sizeof(mysql_mutex_t)=%d\n", sizeof(mysql_mutex_t));
		fprintf(stderr, "[binlog_dump_list] sizeof(Statement_map)=%d\n", sizeof(Statement_map));
		fprintf(stderr, "[binlog_dump_list] sizeof(Security_context)=%d\n", sizeof(Security_context));
    #endif
	return 0;
}

static int deinit_plugin(void *p) {
	fprintf(stderr, "binlog_dump_list plugin stopped\n");
	return 0;
}
