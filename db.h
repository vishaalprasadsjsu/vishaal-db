/********************************************************************
db.h - This file contains all the structures, defines, and function
  prototype for the db.exe program.
*********************************************************************/

#include <stdio.h>
#include <string>
#include <locale>

#define MAX_IDENT_LEN   16
#define MAX_NUM_COL     16
#define MAX_TOK_LEN     32
#define KEYWORD_OFFSET  10
#define STRING_BREAK    " (),<>="
#define NUMBER_BREAK    " ),"

/* Column descriptor structure = 20+4+4+4+4 = 36 bytes */
typedef struct cd_entry_def {
  char col_name[MAX_IDENT_LEN + 4];
  int col_id;                   /* Start from 0 */
  int col_type;
  int col_len;
  int not_null;
} cd_entry;

/* Table packed descriptor structure = 4+20+4+4+4 = 36 bytes
   Minimum of 1 column in a table - therefore minimum size of
   1 valid tpd_entry is 36+36 = 72 bytes. */
typedef struct tpd_entry_def {
  int tpd_size;
  char table_name[MAX_IDENT_LEN + 4];
  int num_columns;
  int cd_offset;
  int tpd_flags;
} tpd_entry;

/* Table packed descriptor list = 4+4+4+36 = 48 bytes.  When no
   table is defined the tpd_list is 48 bytes.  When there is 
   at least 1 table, then the tpd_entry (36 bytes) will be
   overlapped by the first valid tpd_entry. */
typedef struct tpd_list_def {
  int list_size;
  int num_tables;
  int db_flags;
  tpd_entry tpd_start;
} tpd_list;

/* This token_list definition is used for breaking the command
   string into separate tokens in function get_tokens().  For
   each token, a new token_list will be allocated and linked 
   together. */
typedef struct t_list {
  char tok_string[MAX_TOK_LEN];
  int tok_class;
  int tok_value;
  struct t_list *next;
} token_list;

/* Table header, at start of a .tab file */
typedef struct table_file_header_def {
  int file_size;        // 4 bytes
  int record_size;      // 4 bytes
  int num_records;      // 4 bytes
  int record_offset;    // 4 bytes
  int file_header_flag; // 4 bytes
  tpd_entry *tpd_ptr;         // 4 bytes [?]
} table_file_header;           // minimum size = 24

/* This enum defines the different classes of tokens for 
   semantic processing. */
typedef enum t_class {
  keyword = 1,
  identifier,
  symbol,
  type_name,
  constant,
  function_name,
  terminator,
  error
} token_class;

/* This enum defines the different values associated with
   a single valid token.  Use for semantic processing. */
typedef enum t_value {
  // new type should be added above this line
  T_INT = 10,
  T_CHAR,
  T_VARCHAR,
  K_CREATE,
  K_TABLE,
  K_NOT,
  K_NULL,
  K_DROP,
  K_LIST,
  K_SCHEMA,
  K_FOR,
  K_TO,
  K_INSERT,
  K_INTO,
  K_VALUES,
  K_DELETE,
  K_FROM,
  K_WHERE,
  K_UPDATE,
  K_SET,
  K_SELECT,
  K_ORDER,
  K_BY,
  K_DESC,
  K_IS,
  K_AND,
  K_OR,
  K_BACKUP,
  K_RESTORE,
  K_ROLLFORWARD,
  K_RF,
  K_WITHOUT,
  F_SUM,
  F_AVG,
  F_COUNT,
  // new function name should be added below this line

  S_LEFT_PAREN = 70,
  S_RIGHT_PAREN,
  S_COMMA,
  S_STAR,
  S_EQUAL,
  S_LESS,
  S_GREATER,

  IDENT = 85,
  INT_LITERAL = 90,
  STRING_LITERAL,
  EOC = 95,
  INVALID = 99
} token_value;

/* This constants must be updated when add new keywords */
#define TOTAL_KEYWORDS_PLUS_TYPE_NAMES 35

/* New keyword must be added in the same position/order as the enum
   definition above, otherwise the lookup will be wrong */
char *keyword_table[] =
    {"int", "char", "varchar", "create", "table", "not", "null", "drop", "list", "schema", "for",
     "to", "insert", "into", "values", "delete", "from", "where", "update", "set", "select",
     "order", "by", "desc", "is", "and", "or", "backup", "restore", "rollforward", "rf", "without",
     "sum", "avg", "count"};

/* This enum defines a set of possible statements */
typedef enum s_statement {
  INVALID_STATEMENT = -199,

  CREATE_TABLE = 100,
  DROP_TABLE,
  LIST_TABLE,
  LIST_SCHEMA,
  INSERT,
  DELETE,
  UPDATE,
  SELECT,
  BACKUP,
  RESTORE,
  ROLLFORWARD
} semantic_statement;

/* This enum has a list of all the errors that should be detected
   by the program.  Can append to this if necessary. */
typedef enum error_return_codes {
  INVALID_TABLE_NAME = -399,
  DUPLICATE_TABLE_NAME,
  TABLE_NOT_EXIST,
  INVALID_TABLE_DEFINITION,
  INVALID_COLUMN_NAME,
  DUPLICATE_COLUMN_NAME,
  COLUMN_NOT_EXIST,
  MAX_COLUMN_EXCEEDED,
  INVALID_TYPE_NAME,
  INVALID_COLUMN_DEFINITION,
  INVALID_COLUMN_LENGTH,
  INVALID_REPORT_FILE_NAME,
  INVALID_INSERT_STATEMENT,

  // add all the possible errors from I/U/D + SELECT here */

  FILE_OPEN_ERROR = -299,
  DBFILE_CORRUPTION,
  MEMORY_ERROR
} return_codes;

/* Set of function prototypes */
int get_token(char *command, token_list **tok_list);
void add_to_list(token_list **tok_list, char *tmp, int t_class, int t_value);
int do_semantic(token_list *tok_list);
int sem_create_table(token_list *t_list);
int sem_drop_table(token_list *t_list);
int sem_list_tables();
int sem_list_schema(token_list *t_list);
int sem_select(token_list *cur_token);
int sem_select_agg(token_list *t_list);
int sem_insert_value(token_list *cur_token);
int sem_update_value(token_list *cur_token);
int sem_delete_value(token_list *cur_token);
int sem_backup(token_list *cur_token);
int sem_restore(token_list *cur_token);
int sem_rollforward(token_list *cur_token);

/*
  Keep a global list of tpd - in real life, this will be stored
  in shared memory.  Build a set of functions/methods around this.
*/
tpd_list *g_tpd_list;
bool is_rollforwarding = false;

int initialize_tpd_list();
int add_tpd_to_list(tpd_entry *tpd);
int drop_tpd_from_list(char *tabname);
tpd_entry *get_tpd_from_list(char *tabname);

int create_table_data_file(char *tab_name, table_file_header *table_file_header, size_t size);
table_file_header *get_file_header(char *tab_name);
void append_field_to_tab(char *tab_name, token_list *token, int col_len);
void append_zeros_to_tab(char *tab_name, int how_many_bytes);
int get_print_size(cd_entry *cd);
int delete_tab_file(char *tab_name);
bool satisfies_condition(char *field, int operator_type, token_list *comp_value_token, int col_len);
cd_entry *get_cd(char *table_name, char *col_name);
int get_compare_vals(token_list *cur_token, char *table_name, cd_entry *first_cd, int *comp_type,
                     token_list **comp_value_token, cd_entry **compare_cd, int *comp_field_offset);
bool compare_records_by_val(const char *record_a, const char *record_b,
                            cd_entry *order_cd, int field_offset, bool desc);
int add_to_log_end(token_list *first_token);
std::string get_timestamp();
bool is_timestamp_before(std::string first, std::string second);
