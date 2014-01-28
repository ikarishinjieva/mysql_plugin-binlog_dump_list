#define MYSQL_SERVER 1
#include <sql_class.h>

#define LIST_PROCESS_HOST_LEN 64 //sql_show.cc

#define MAX_BINLOG_DUMP_STATUS_LEN 70

static ST_FIELD_INFO is_table_fields[] = {
	{"ID", MY_INT64_NUM_DECIMAL_DIGITS, MYSQL_TYPE_LONG, 0, 0, 0, 0},
	{"HOST", LIST_PROCESS_HOST_LEN, MYSQL_TYPE_STRING, 0, 0, 0, 0},
	{"PORT", 7, MYSQL_TYPE_LONG, 0, 0, 0, 0},
	{"USER", 16, MYSQL_TYPE_STRING, 0, 0, 0, 0},
	{"STATUS", MAX_BINLOG_DUMP_STATUS_LEN, MYSQL_TYPE_STRING, 0, 0, 0, 0}, 
	{0, 0, MYSQL_TYPE_NULL, 0, 0, 0, 0}
};

static int init_plugin(void *p);
static int deinit_plugin(void *p);

struct st_mysql_information_schema my_plugin = {
	MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION
};

mysql_declare_plugin(binlog_dump_list) 
{
	MYSQL_INFORMATION_SCHEMA_PLUGIN,
	&my_plugin,
	"binlog_dump_list",
	"Tac Huang",
	"List Binlog Dump threads, providing more informations than SHOW PROCESSLIST",
	PLUGIN_LICENSE_GPL,
	init_plugin,
	deinit_plugin,
	0x0100,
	NULL,
	NULL,
	NULL
}
mysql_declare_plugin_end;