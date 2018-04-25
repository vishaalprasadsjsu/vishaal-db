/********************************************************************
db.h - This file contains all the structures, defines, and function
  prototype for the db.exe program.
*********************************************************************/

#include <stdio.h>

#define MAX_IDENT_LEN   16
#define MAX_NUM_COL     16
#define MAX_TOK_LEN     32
#define KEYWORD_OFFSET  10
#define STRING_BREAK    " (),<>="
#define NUMBER_BREAK    " ),"

/* Column descriptor structure = 20+4+4+4+4 = 36 bytes */
typedef struct cd_entry_def
{
  char    col_name[MAX_IDENT_LEN+4];
  int     col_id;                   /* Start from 0 */
  int     col_type;
  int     col_len;
  int     not_null;
} cd_entry;

/* Table packed descriptor structure = 4+20+4+4+4 = 36 bytes
   Minimum of 1 column in a table - therefore minimum size of
   1 valid tpd_entry is 36+36 = 72 bytes. */
typedef struct tpd_entry_def
{
  int       tpd_size;
  char      table_name[MAX_IDENT_LEN+4];
  int       num_columns;
  int       cd_offset;
  int       tpd_flags;
} tpd_entry;

/* Table packed descriptor list = 4+4+4+36 = 48 bytes.  When no
   table is defined the tpd_list is 48 bytes.  When there is 
   at least 1 table, then the tpd_entry (36 bytes) will be
   overlapped by the first valid tpd_entry. */
typedef struct tpd_list_def
{
  int       list_size;
  int       num_tables;
  int       db_flags;
  tpd_entry tpd_start;
} tpd_list;

/* This token_list definition is used for breaking the command
   string into separate tokens in function get_tokens().  For
   each token, a new token_list will be allocated and linked 
   together. */
typedef struct t_list
{
  char  tok_string[MAX_TOK_LEN];
  int   tok_class;
  int   tok_value;
  struct t_list *next;
} token_list;

/* Table header, at start of a .tab file */
typedef struct table_file_header_def
{
  int    file_size;        // 4 bytes
  int    record_size;      // 4 bytes
  int    num_records;      // 4 bytes
  int    record_offset;    // 4 bytes
  int    file_header_flag; // 4 bytes
  tpd_entry  *tpd_ptr;         // 4 bytes [?]
} table_file_header;           // minimum size = 24

/* This enum defines the different classes of tokens for 
   semantic processing. */
typedef enum t_class
{
  keyword = 1,  // 1
  identifier,   // 2
  symbol,       // 3
  type_name,    // 4
  constant,     // 5
  function_name,// 6
  terminator,   // 7
  error         // 8
} token_class;

/* This enum defines the different values associated with
   a single valid token.  Use for semantic processing. */
typedef enum t_value
{
  T_INT = 10,   // 10 - new type should be added above this line
  T_CHAR,       // 11 
  T_VARCHAR,    // 12
  K_CREATE,     // 13
  K_TABLE,      // 14
  K_NOT,        // 15
  K_NULL,       // 16
  K_DROP,       // 17
  K_LIST,       // 18
  K_SCHEMA,     // 19
  K_FOR,        // 20
  K_TO,         // 21
  K_INSERT,     // 22
  K_INTO,       // 23
  K_VALUES,     // 24
  K_DELETE,     // 25
  K_FROM,       // 26
  K_WHERE,      // 27
  K_UPDATE,     // 28
  K_SET,        // 29
  K_SELECT,     // 30
  K_ORDER,      // 31
  K_BY,         // 32
  K_DESC,       // 33
  K_IS,         // 34
  K_AND,        // 35
  K_OR,         // 36 - new keyword should be added below this line
  F_SUM,        // 37
  F_AVG,        // 38
  F_COUNT,      // 39 - new function name should be added below this line
  S_LEFT_PAREN = 70,  // 70
  S_RIGHT_PAREN,      // 71
  S_COMMA,            // 72
  S_STAR,             // 73
  S_EQUAL,            // 74
  S_LESS,             // 75
  S_GREATER,          // 76
  IDENT = 85,         // 85
  INT_LITERAL = 90,   // 90
  STRING_LITERAL,     // 91
  EOC = 95,           // 95
  INVALID = 99        // 99
} token_value;

/* This constants must be updated when add new keywords */
#define TOTAL_KEYWORDS_PLUS_TYPE_NAMES 30

/* New keyword must be added in the same position/order as the enum
   definition above, otherwise the lookup will be wrong */
char *keyword_table[] =
{
  "int", "char", "varchar", "create", "table", "not", "null", "drop", "list", "schema",
  "for", "to", "insert", "into", "values", "delete", "from", "where",
  "update", "set", "select", "order", "by", "desc", "is", "and", "or",
  "sum", "avg", "count"
};

/* This enum defines a set of possible statements */
typedef enum s_statement
{
  INVALID_STATEMENT = -199, // -199
  CREATE_TABLE = 100,       // 100
  DROP_TABLE,               // 101
  LIST_TABLE,               // 102
  LIST_SCHEMA,              // 103
  INSERT,                   // 104
  DELETE,                   // 105
  UPDATE,                   // 106
  SELECT                    // 107
} semantic_statement;

/* This enum has a list of all the errors that should be detected
   by the program.  Can append to this if necessary. */
typedef enum error_return_codes
{
  INVALID_TABLE_NAME = -399,  // -399
  DUPLICATE_TABLE_NAME,       // -398
  TABLE_NOT_EXIST,            // -397
  INVALID_TABLE_DEFINITION,   // -396
  INVALID_COLUMN_NAME,        // -395
  DUPLICATE_COLUMN_NAME,      // -394
  COLUMN_NOT_EXIST,           // -393
  MAX_COLUMN_EXCEEDED,        // -392
  INVALID_TYPE_NAME,          // -391
  INVALID_COLUMN_DEFINITION,  // -390
  INVALID_COLUMN_LENGTH,      // -389
  INVALID_REPORT_FILE_NAME,   // -388
  INVALID_INSERT_STATEMENT,   // -397
  /* Must add all the possible errors from I/U/D + SELECT here */
  FILE_OPEN_ERROR = -299,     // -299
  DBFILE_CORRUPTION,          // -298
  MEMORY_ERROR                // -297
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

/*
  Keep a global list of tpd - in real life, this will be stored
  in shared memory.  Build a set of functions/methods around this.
*/
tpd_list  *g_tpd_list;
int initialize_tpd_list();
int add_tpd_to_list(tpd_entry *tpd);
int drop_tpd_from_list(char *tabname);
tpd_entry* get_tpd_from_list(char *tabname);

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
