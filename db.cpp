/************************************************************
  Project#1:  CLP & DDL
 ************************************************************/

#include "db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <zconf.h>
#include <string>
#include <cmath>
#include <vector>
#include <functional>
using namespace std::placeholders;

#if defined(_WIN32) || defined(_WIN64)
#define strcasecmp _stricmp
#endif

int main(int argc, char **argv) {
  int rc = 0;
  token_list *tok_list = NULL, *tok_ptr = NULL, *tmp_tok_ptr = NULL;

  if ((argc != 2) || (strlen(argv[1]) == 0)) {
    printf("Usage: db \"command statement\"");
    return 1;
  }

  rc = initialize_tpd_list();

  if (rc) {
    printf("\nError in initialize_tpd_list().\nrc = %d\n", rc);

  } else {
    rc = get_token(argv[1], &tok_list);

//    /* Test code */
//    tok_ptr = tok_list;
//    while (tok_ptr != NULL) {
//      printf("%16s \t%d \t %d\n", tok_ptr->tok_string, tok_ptr->tok_class,
//             tok_ptr->tok_value);
//      tok_ptr = tok_ptr->next;
//    }

    if (!rc) {
      rc = do_semantic(tok_list);
    }

    if (rc) {
      tok_ptr = tok_list;
      while (tok_ptr != NULL) {
        if ((tok_ptr->tok_class == error)
            || (tok_ptr->tok_value == INVALID)) {
          printf("\nError in the string: %s\n", tok_ptr->tok_string);
          printf("rc=%d\n", rc);
          break;
        }
        tok_ptr = tok_ptr->next;
      }
    }

    /* Whether the token list is valid or not, we need to free the memory */
    tok_ptr = tok_list;
    while (tok_ptr != NULL) {
      tmp_tok_ptr = tok_ptr->next;
      free(tok_ptr);
      tok_ptr = tmp_tok_ptr;
    }
  }

  return rc;
}

/************************************************************* 
  This is a lexical analyzer for simple SQL statements
 *************************************************************/
int get_token(char *command, token_list **tok_list) {

  int rc = 0, i, j;
  char *start, *cur, temp_string[MAX_TOK_LEN];
  bool done = false;

  start = cur = command;
  while (!done) {
    bool found_keyword = false;

    /* This is the TOP Level for each token */
    memset((void *) temp_string, '\0', MAX_TOK_LEN);
    i = 0;

    /* Get rid of all the leading blanks */
    while (*cur == ' ')
      cur++;

    if (cur && isalpha(*cur)) {
      // find valid identifier
      int t_class;
      do {
        temp_string[i++] = *cur++;
      } while ((isalnum(*cur)) || (*cur == '_'));

      if (!(strchr(STRING_BREAK, *cur))) {
        /* If the next char following the keyword or identifier
           is not a blank, (, ), or a comma, then append this
           character to temp_string, and flag this as an error */
        temp_string[i++] = *cur++;
        add_to_list(tok_list, temp_string, error, INVALID);
        rc = INVALID;
        done = true;
      } else {

        // We have an identifier with at least 1 character
        // Now check if this ident is a keyword
        for (j = 0, found_keyword = false; j < TOTAL_KEYWORDS_PLUS_TYPE_NAMES; j++) {
          if ((strcasecmp(keyword_table[j], temp_string) == 0)) {
            found_keyword = true;
            break;
          }
        }

        if (found_keyword) {
          if (KEYWORD_OFFSET + j < K_CREATE)
            t_class = type_name;
          else if (KEYWORD_OFFSET + j >= F_SUM)
            t_class = function_name;
          else
            t_class = keyword;

          add_to_list(tok_list, temp_string, t_class, KEYWORD_OFFSET + j);
        } else {
          if (strlen(temp_string) <= MAX_IDENT_LEN)
            add_to_list(tok_list, temp_string, identifier, IDENT);
          else {
            add_to_list(tok_list, temp_string, error, INVALID);
            rc = INVALID;
            done = true;
          }
        }

        if (!*cur) {
          add_to_list(tok_list, "", terminator, EOC);
          done = true;
        }
      }
    } else if (isdigit(*cur)) {
      // find valid number
      do {
        temp_string[i++] = *cur++;
      } while (isdigit(*cur));

      if (!(strchr(NUMBER_BREAK, *cur))) {
        /* If the next char following the keyword or identifier
           is not a blank or a ), then append this
           character to temp_string, and flag this as an error */
        temp_string[i++] = *cur++;
        add_to_list(tok_list, temp_string, error, INVALID);
        rc = INVALID;
        done = true;
      } else {
        add_to_list(tok_list, temp_string, constant, INT_LITERAL);

        if (!*cur) {
          add_to_list(tok_list, "", terminator, EOC);
          done = true;
        }
      }
    } else if ((*cur == '(') || (*cur == ')') || (*cur == ',') || (*cur == '*')
               || (*cur == '=') || (*cur == '<') || (*cur == '>')) {
      /* Catch all the symbols here. Note: no look ahead here. */
      int t_value;
      switch (*cur) {
        case '(' :
          t_value = S_LEFT_PAREN;
          break;
        case ')' :
          t_value = S_RIGHT_PAREN;
          break;
        case ',' :
          t_value = S_COMMA;
          break;
        case '*' :
          t_value = S_STAR;
          break;
        case '=' :
          t_value = S_EQUAL;
          break;
        case '<' :
          t_value = S_LESS;
          break;
        case '>' :
          t_value = S_GREATER;
          break;
      }

      temp_string[i++] = *cur++;

      add_to_list(tok_list, temp_string, symbol, t_value);

      if (!*cur) {
        add_to_list(tok_list, "", terminator, EOC);
        done = true;
      }

    } else if (*cur == '\'') {
      /* Find STRING_LITERAL */
      int t_class;
      cur++;
      do {
        temp_string[i++] = *cur++;
      } while ((*cur) && (*cur != '\''));

      temp_string[i] = '\0';

      if (!*cur) {
        /* If we reach the end of line */
        add_to_list(tok_list, temp_string, error, INVALID);
        rc = INVALID;
        done = true;
      } else {
        /* must be a ' */
        add_to_list(tok_list, temp_string, constant, STRING_LITERAL);
        cur++;
        if (!*cur) {
          add_to_list(tok_list, "", terminator, EOC);
          done = true;
        }
      }
    } else {
      if (!*cur) {
        add_to_list(tok_list, "", terminator, EOC);
        done = true;
      } else {
        /* not a ident, number, or valid symbol */
        temp_string[i++] = *cur++;
        add_to_list(tok_list, temp_string, error, INVALID);
        rc = INVALID;
        done = true;
      }
    }
  }

  return rc;
}

void add_to_list(token_list **tok_list, char *tmp, int t_class, int t_value) {
  token_list *cur = *tok_list;
  token_list *ptr = NULL;

  // printf("%16s \t%d \t %d\n",tmp, t_class, t_value);

  ptr = (token_list *) calloc(1, sizeof(token_list));
  strcpy(ptr->tok_string, tmp);
  ptr->tok_class = t_class;
  ptr->tok_value = t_value;
  ptr->next = NULL;

  if (cur == NULL)
    *tok_list = ptr;
  else {
    while (cur->next != NULL)
      cur = cur->next;

    cur->next = ptr;
  }
  return;
}

int do_semantic(token_list *tok_list) {
  int rc = 0, cur_cmd = INVALID_STATEMENT;
  bool unique = false;
  token_list *cur = tok_list;

  if ((cur->tok_value == K_CREATE) &&
      ((cur->next != NULL) && (cur->next->tok_value == K_TABLE))) {
    printf("CREATE TABLE statement\n");
    cur_cmd = CREATE_TABLE;
    cur = cur->next->next;

  } else if ((cur->tok_value == K_DROP) &&
             ((cur->next != NULL) && (cur->next->tok_value == K_TABLE))) {
    printf("DROP TABLE statement\n");
    cur_cmd = DROP_TABLE;
    cur = cur->next->next;

  } else if ((cur->tok_value == K_LIST) &&
             ((cur->next != NULL) && (cur->next->tok_value == K_TABLE))) {
    printf("LIST TABLE statement\n");
    cur_cmd = LIST_TABLE;
    cur = cur->next->next;

  } else if ((cur->tok_value == K_LIST) &&
             ((cur->next != NULL) && (cur->next->tok_value == K_SCHEMA))) {
    printf("LIST SCHEMA statement\n");
    cur_cmd = LIST_SCHEMA;
    cur = cur->next->next;

  } else if ((cur->tok_value == K_INSERT) &&
             ((cur->next != NULL) && (cur->next->tok_value == K_INTO))) {
    printf("INSERT statement\n");
    cur_cmd = INSERT;
    cur = cur->next->next;

  } else if ((cur->tok_value == K_SELECT) && ((cur->next != NULL))) {
    printf("SELECT statement\n");
    cur_cmd = SELECT;
    cur = cur->next;

  } else if ((cur->tok_value == K_DELETE) &&
             ((cur->next != NULL) && (cur->next->tok_value == K_FROM))) {
    printf("DELETE statement\n");
    cur_cmd = DELETE;
    cur = cur->next->next;

  } else if (cur->tok_value == K_UPDATE && cur->next != nullptr
             && cur->next->next != nullptr && cur->next->next->tok_value == K_SET) {

    printf("UPDATE statement\n");
    cur_cmd = UPDATE;
    cur = cur->next;

  } else {
    printf("Invalid statement\n");
    rc = cur_cmd;
  }

  if (cur_cmd != INVALID_STATEMENT) {
    switch (cur_cmd) {
      case CREATE_TABLE:
        rc = sem_create_table(cur);
        break;
      case DROP_TABLE:
        rc = sem_drop_table(cur);
        break;
      case LIST_TABLE:
        rc = sem_list_tables();
        break;
      case LIST_SCHEMA:
        rc = sem_list_schema(cur);
        break;
      case INSERT:
        rc = sem_insert_value(cur);
        break;
      case SELECT:
        rc = sem_select(cur);
        break;
      case DELETE:
        rc = sem_delete_value(cur);
        break;
      case UPDATE:
        rc = sem_update_value(cur);
        break;
      default:
        break;
    }
  }

  return rc;
}

int sem_create_table(token_list *t_list) {
  int rc = 0;
  token_list *cur;
  tpd_entry tab_entry;
  tpd_entry *new_entry = NULL;
  table_file_header tab_file_header;
  int record_size = 0;

  bool column_done = false;
  int cur_id = 0;
  cd_entry col_entry[MAX_NUM_COL];

  memset(&tab_entry, '\0', sizeof(tpd_entry));
  memset(&tab_file_header, '\0', sizeof(table_file_header));
  cur = t_list;

  if ((cur->tok_class != keyword)
      && (cur->tok_class != identifier)
      && (cur->tok_class != type_name)) {

    // Error
    rc = INVALID_TABLE_NAME;
    cur->tok_value = INVALID;

  } else {

    if ((new_entry = get_tpd_from_list(cur->tok_string)) != NULL) {
      rc = DUPLICATE_TABLE_NAME;
      cur->tok_value = INVALID;

    } else {
      strcpy(tab_entry.table_name, cur->tok_string);
      cur = cur->next;

      if (cur->tok_value != S_LEFT_PAREN) {
        //Error
        rc = INVALID_TABLE_DEFINITION;
        cur->tok_value = INVALID;

      } else {
        memset(&col_entry, '\0', (MAX_NUM_COL * sizeof(cd_entry)));

        /* Now build a set of column entries */
        cur = cur->next;
        do {
          if ((cur->tok_class != keyword) &&
              (cur->tok_class != identifier) &&
              (cur->tok_class != type_name)) {
            // Error
            rc = INVALID_COLUMN_NAME;
            cur->tok_value = INVALID;

          } else {
            int i;
            for (i = 0; i < cur_id; i++) {
              /* make column name case sensitive */
              if (strcmp(col_entry[i].col_name, cur->tok_string) == 0) {
                rc = DUPLICATE_COLUMN_NAME;
                cur->tok_value = INVALID;
                break;
              }
            }

            if (!rc) {
              strcpy(col_entry[cur_id].col_name, cur->tok_string);
              col_entry[cur_id].col_id = cur_id;
              col_entry[cur_id].not_null = false;    /* set default */

              cur = cur->next;
              if (cur->tok_class != type_name) {
                // Error
                rc = INVALID_TYPE_NAME;
                cur->tok_value = INVALID;
              } else {
                /* Set the column type here, int or char */
                col_entry[cur_id].col_type = cur->tok_value;
                cur = cur->next;

                if (col_entry[cur_id].col_type == T_INT) {
                  if ((cur->tok_value != S_COMMA) &&
                      (cur->tok_value != K_NOT) &&
                      (cur->tok_value != S_RIGHT_PAREN)) {
                    rc = INVALID_COLUMN_DEFINITION;
                    cur->tok_value = INVALID;
                  } else {
                    int attr_size = sizeof(int);
                    col_entry[cur_id].col_len = attr_size;
                    record_size += attr_size + 1;

                    if ((cur->tok_value == K_NOT) &&
                        (cur->next->tok_value != K_NULL)) {
                      rc = INVALID_COLUMN_DEFINITION;
                      cur->tok_value = INVALID;
                    } else if ((cur->tok_value == K_NOT) &&
                               (cur->next->tok_value == K_NULL)) {
                      col_entry[cur_id].not_null = true;
                      cur = cur->next->next;
                    }

                    if (!rc) {
                      /* I must have either a comma or right paren */
                      if ((cur->tok_value != S_RIGHT_PAREN) &&
                          (cur->tok_value != S_COMMA)) {
                        rc = INVALID_COLUMN_DEFINITION;
                        cur->tok_value = INVALID;
                      } else {
                        if (cur->tok_value == S_RIGHT_PAREN) {
                          column_done = true;
                        }
                        cur = cur->next;
                      }
                    }
                  }
                }   // end of T_INT processing
                else {
                  // It must be char() or varchar() 
                  if (cur->tok_value != S_LEFT_PAREN) {
                    rc = INVALID_COLUMN_DEFINITION;
                    cur->tok_value = INVALID;
                  } else {
                    /* Enter char(n) processing */
                    cur = cur->next;

                    if (cur->tok_value != INT_LITERAL) {
                      rc = INVALID_COLUMN_LENGTH;
                      cur->tok_value = INVALID;
                    } else {
                      /* Got a valid integer - convert */
                      int attr_size = atoi(cur->tok_string);
                      col_entry[cur_id].col_len = attr_size;
                      record_size += attr_size + 1;

                      cur = cur->next;

                      if (cur->tok_value != S_RIGHT_PAREN) {
                        rc = INVALID_COLUMN_DEFINITION;
                        cur->tok_value = INVALID;
                      } else {
                        cur = cur->next;

                        if ((cur->tok_value != S_COMMA) &&
                            (cur->tok_value != K_NOT) &&
                            (cur->tok_value != S_RIGHT_PAREN)) {
                          rc = INVALID_COLUMN_DEFINITION;
                          cur->tok_value = INVALID;
                        } else {
                          if ((cur->tok_value == K_NOT) &&
                              (cur->next->tok_value != K_NULL)) {
                            rc = INVALID_COLUMN_DEFINITION;
                            cur->tok_value = INVALID;
                          } else if ((cur->tok_value == K_NOT) &&
                                     (cur->next->tok_value == K_NULL)) {
                            col_entry[cur_id].not_null = true;
                            cur = cur->next->next;
                          }

                          if (!rc) {
                            /* I must have either a comma or right paren */
                            if ((cur->tok_value != S_RIGHT_PAREN) && (cur->tok_value != S_COMMA)) {
                              rc = INVALID_COLUMN_DEFINITION;
                              cur->tok_value = INVALID;
                            } else {
                              if (cur->tok_value == S_RIGHT_PAREN) {
                                column_done = true;
                              }
                              cur = cur->next;
                            }
                          }
                        }
                      }
                    } /* end char(n) processing */
                  }
                } /* end char processing */
              }
            }  // duplicate column name
          } // invalid column name

          /* If rc=0, then get ready for the next column */
          if (!rc) {
            cur_id++;
          }

        } while ((rc == 0) && (!column_done));

        if ((column_done) && (cur->tok_value != EOC)) {
          rc = INVALID_TABLE_DEFINITION;
          cur->tok_value = INVALID;
        }

        if (!rc) {
          /* Now finished building tpd and add it to the tpd list */
          tab_entry.num_columns = cur_id;
          tab_entry.tpd_size = sizeof(tpd_entry) +
                               sizeof(cd_entry) * tab_entry.num_columns;
          tab_entry.cd_offset = sizeof(tpd_entry);
          new_entry = (tpd_entry *) calloc(1, tab_entry.tpd_size);

          if (new_entry == NULL) {
            rc = MEMORY_ERROR;

          } else {
            memcpy((void *) new_entry,
                   (void *) &tab_entry,
                   sizeof(tpd_entry));

            memcpy((void *) ((char *) new_entry + sizeof(tpd_entry)),
                   (void *) col_entry,
                   sizeof(cd_entry) * tab_entry.num_columns);

            // round up to 4
            record_size += (record_size % 4) ? (4 - (record_size % 4)) : 0;

            tab_file_header.file_size = sizeof(table_file_header_def);
            tab_file_header.record_size = record_size;
            tab_file_header.num_records = 0;
            tab_file_header.record_offset = 0;
            tab_file_header.file_header_flag = 0;
            tab_file_header.tpd_ptr = NULL;

            rc = add_tpd_to_list(new_entry);

            create_table_data_file(new_entry->table_name, &tab_file_header, sizeof(table_file_header));

            free(new_entry);
          }
        }
      }
    }
  }

  return rc;
}

int sem_drop_table(token_list *t_list) {
  int rc = 0;
  token_list *cur;
  tpd_entry *tab_entry = NULL;

  cur = t_list;
  if ((cur->tok_class != keyword) &&
      (cur->tok_class != identifier) &&
      (cur->tok_class != type_name)) {
    // Error
    rc = INVALID_TABLE_NAME;
    cur->tok_value = INVALID;
  } else {
    if (cur->next->tok_value != EOC) {
      rc = INVALID_STATEMENT;
      cur->next->tok_value = INVALID;
    } else {
      if ((tab_entry = get_tpd_from_list(cur->tok_string)) == NULL) {
        rc = TABLE_NOT_EXIST;
        cur->tok_value = INVALID;
      } else {
        /* Found a valid tpd, drop it from tpd list */
        delete_tab_file(cur->tok_string);
        rc = drop_tpd_from_list(cur->tok_string);
      }
    }
  }

  return rc;
}

int sem_list_tables() {
  int rc = 0;
  int num_tables = g_tpd_list->num_tables;
  tpd_entry *cur = &(g_tpd_list->tpd_start);

  if (num_tables == 0) {
    printf("\nThere are currently no tables defined\n");
  } else {
    printf("\nTable List\n");
    printf("*****************\n");
    while (num_tables-- > 0) {
      printf("%s\n", cur->table_name);
      if (num_tables > 0) {
        cur = (tpd_entry *) ((char *) cur + cur->tpd_size);
      }
    }
    printf("****** End ******\n");
  }

  return rc;
}

int sem_list_schema(token_list *t_list) {
  int rc = 0;
  token_list *cur;
  tpd_entry *tab_entry = NULL;
  cd_entry *col_entry = NULL;
  char tab_name[MAX_IDENT_LEN + 1];
  char filename[MAX_IDENT_LEN + 1];
  bool report = false;
  FILE *fhandle = NULL;
  int i = 0;

  cur = t_list;

  if (cur->tok_value != K_FOR) {
    rc = INVALID_STATEMENT;
    cur->tok_value = INVALID;
  } else {
    cur = cur->next;

    if ((cur->tok_class != keyword) &&
        (cur->tok_class != identifier) &&
        (cur->tok_class != type_name)) {
      // Error
      rc = INVALID_TABLE_NAME;
      cur->tok_value = INVALID;
    } else {
      memset(filename, '\0', MAX_IDENT_LEN + 1);
      strcpy(tab_name, cur->tok_string);
      cur = cur->next;

      if (cur->tok_value != EOC) {
        if (cur->tok_value == K_TO) {
          cur = cur->next;

          if ((cur->tok_class != keyword) &&
              (cur->tok_class != identifier) &&
              (cur->tok_class != type_name)) {
            // Error
            rc = INVALID_REPORT_FILE_NAME;
            cur->tok_value = INVALID;
          } else {
            if (cur->next->tok_value != EOC) {
              rc = INVALID_STATEMENT;
              cur->next->tok_value = INVALID;
            } else {
              /* We have a valid file name */
              strcpy(filename, cur->tok_string);
              report = true;
            }
          }
        } else {
          /* Missing the TO keyword */
          rc = INVALID_STATEMENT;
          cur->tok_value = INVALID;
        }
      }

      if (!rc) {
        if ((tab_entry = get_tpd_from_list(tab_name)) == NULL) {
          rc = TABLE_NOT_EXIST;
          cur->tok_value = INVALID;
        } else {
          if (report) {
            if ((fhandle = fopen(filename, "a+tc")) == NULL) {
              rc = FILE_OPEN_ERROR;
            }
          }

          if (!rc) {
            /* Find correct tpd, need to parse column and index information */

            /* First, write the tpd_entry information */
            printf("Table PD size            (tpd_size)    = %d\n", tab_entry->tpd_size);
            printf("Table Name               (table_name)  = %s\n", tab_entry->table_name);
            printf("Number of Columns        (num_columns) = %d\n", tab_entry->num_columns);
            printf("Column Descriptor Offset (cd_offset)   = %d\n", tab_entry->cd_offset);
            printf("Table PD Flags           (tpd_flags)   = %d\n\n", tab_entry->tpd_flags);

            if (report) {
              fprintf(fhandle, "Table PD size            (tpd_size)    = %d\n", tab_entry->tpd_size);
              fprintf(fhandle, "Table Name               (table_name)  = %s\n", tab_entry->table_name);
              fprintf(fhandle, "Number of Columns        (num_columns) = %d\n", tab_entry->num_columns);
              fprintf(fhandle, "Column Descriptor Offset (cd_offset)   = %d\n", tab_entry->cd_offset);
              fprintf(fhandle, "Table PD Flags           (tpd_flags)   = %d\n\n", tab_entry->tpd_flags);
            }

            /* Next, write the cd_entry information */
            for (i = 0, col_entry = (cd_entry *) ((char *) tab_entry + tab_entry->cd_offset);
                 i < tab_entry->num_columns; i++, col_entry++) {
              printf("Column Name   (col_name) = %s\n", col_entry->col_name);
              printf("Column Id     (col_id)   = %d\n", col_entry->col_id);
              printf("Column Type   (col_type) = %d\n", col_entry->col_type);
              printf("Column Length (col_len)  = %d\n", col_entry->col_len);
              printf("Not Null flag (not_null) = %d\n\n", col_entry->not_null);

              if (report) {
                fprintf(fhandle, "Column Name   (col_name) = %s\n", col_entry->col_name);
                fprintf(fhandle, "Column Id     (col_id)   = %d\n", col_entry->col_id);
                fprintf(fhandle, "Column Type   (col_type) = %d\n", col_entry->col_type);
                fprintf(fhandle, "Column Length (col_len)  = %d\n", col_entry->col_len);
                fprintf(fhandle, "Not Null Flag (not_null) = %d\n\n", col_entry->not_null);
              }
            }

            if (report) {
              fflush(fhandle);
              fclose(fhandle);
            }
          } // File open error              
        } // Table not exist
      } // no semantic errors
    } // Invalid table name
  } // Invalid statement

  return rc;
}

int sem_insert_value(token_list *t_list) {

  int rc = 0;
  int num_tables = g_tpd_list->num_tables;
  token_list *cur_token = t_list;
  char *table_name = cur_token->tok_string;
  tpd_entry *curr_table_tpd = get_tpd_from_list(table_name);

  if (curr_table_tpd == nullptr) {
    return TABLE_NOT_EXIST;
  } else {
    cur_token = cur_token->next;
  }

  // make sure next token is keyword "values"
  if (cur_token->tok_class != 1 || cur_token->tok_value != 24) {
    printf("values keyword missing\n");
    return INVALID_STATEMENT;
  } else {
    cur_token = cur_token->next;
  }

  token_list *open_paren_token = nullptr; //hold a reference to the open parenthesis

  // make sure next token is open parenthesis
  if (cur_token->tok_class != 3 || cur_token->tok_value != 70) {
    printf("missing open parenthesis\n");
    return INVALID_STATEMENT;
  } else {
    open_paren_token = cur_token;
    cur_token = cur_token->next;
  }

  // get cd (tpd_start is a pointer to the table)
  cd_entry *curr_cd = (cd_entry *) ((char *) curr_table_tpd + curr_table_tpd->cd_offset);
  cd_entry *cd_start = curr_cd;

  table_file_header *file_header = get_file_header(curr_table_tpd->table_name);

  // check validity of input
  for (int i = 0; i < curr_table_tpd->num_columns; ++i, cur_token = cur_token->next->next, curr_cd++) {

    // invalid null column
    if (curr_cd->not_null && cur_token->tok_value == K_NULL) {
      printf("error: [%s] marked not null\n", curr_cd->col_name);
      return INVALID_INSERT_STATEMENT;
    }

    if (cur_token->tok_value == K_NULL) {
      continue;
    }

    if (curr_cd->col_type == T_CHAR) {
      if (cur_token->tok_value != STRING_LITERAL) {
        printf("error: expected string for [%s]\n", curr_cd->col_name);
        return INVALID_INSERT_STATEMENT;
      }

      // check size
      if (curr_cd->col_len < strlen(cur_token->tok_string)) {
        printf("error: value too big for [%s]: expected length [%d] but got [%d]\n",
               curr_cd->col_name, curr_cd->col_len, (int) strlen(cur_token->tok_string));

        return INVALID_INSERT_STATEMENT;
      }

    } else if (curr_cd->col_type == T_INT) {

      if (cur_token->tok_value != INT_LITERAL) {
        printf("error: expected int for [%s]\n", curr_cd->col_name);
        return INVALID_INSERT_STATEMENT;
      }
    }
  }

  if (file_header->num_records == 0) {
    file_header->record_offset = file_header->file_size;
  }

  file_header->file_size += file_header->record_size;
  file_header->num_records++;
  create_table_data_file(table_name, file_header, sizeof(table_file_header)); // overwrites at the top

  curr_cd = cd_start;
  cur_token = open_paren_token->next;

  int bytes_written = 0;

  for (int i = 0; i < curr_table_tpd->num_columns; ++i, cur_token = cur_token->next->next, curr_cd++) {
    int size = curr_cd->col_len;
    bytes_written += size + 1; // +1 for prefix size
    append_field_to_tab(table_name, cur_token, size);
  }

  if (file_header->record_size > bytes_written) {
    append_zeros_to_tab(table_name, file_header->record_size - bytes_written);
  }

  printf("%s.tab size: %d. Number of records: %d\n", table_name, file_header->file_size, file_header->num_records);

  return rc;
}

void append_zeros_to_tab(char *tab_name, int how_many_bytes) {
  FILE *fhandle = nullptr;

  char *file_name = (char *) calloc(1, (int) strlen(tab_name) + 4);
  strcpy(file_name, tab_name);
  strcat(file_name, ".tab");
  char *zeroed_out = (char *) calloc(1, (size_t) how_many_bytes);

  fhandle = fopen(file_name, "a+b");
  fwrite(zeroed_out, (size_t) how_many_bytes, 1, fhandle);
  fflush(fhandle);
  fclose(fhandle);

  free(file_name);
  free(zeroed_out);
}

void append_field_to_tab(char *tab_name, token_list *token, int col_len) {

  FILE *fhandle = NULL;

  char *file_name = (char *) calloc(1, (int) strlen(tab_name) + 4);
  strcpy(file_name, tab_name);
  strcat(file_name, ".tab");

  fhandle = fopen(file_name, "a+b");

  // now write the actual data
  if (token->tok_value == INT_LITERAL) {
    int prefix_size = sizeof(int);
    int data = atoi(token->tok_string);
    fwrite(&prefix_size, 1, 1, fhandle); // each field prefixed with length
    fwrite(&data, (size_t) col_len, 1, fhandle);

  } else if (token->tok_value == STRING_LITERAL) {
    int prefix_size = (int) strlen(token->tok_string);
    fwrite(&prefix_size, 1, 1, fhandle); // each field prefixed with length
    fwrite(token->tok_string, (size_t) col_len, 1, fhandle);

  } else if (token->tok_value == K_NULL) {
    char *data = (char *) calloc(1, (size_t) col_len);
    fwrite(data, (size_t) col_len + 1, 1, fhandle);
    free(data);

  } else {
    printf("not appending field, unexpected type\n");
  }

  fflush(fhandle);
  fclose(fhandle);
  free(file_name);
}

int initialize_tpd_list() {
  int rc = 0;
  FILE *fhandle = NULL;
//  struct _stat file_stat;
  struct stat file_stat;

  /* Open for read */
  if ((fhandle = fopen("dbfile.bin", "rbc")) == NULL) {
    if ((fhandle = fopen("dbfile.bin", "wbc")) == NULL) {
      rc = FILE_OPEN_ERROR;
    } else {
      g_tpd_list = NULL;
      g_tpd_list = (tpd_list *) calloc(1, sizeof(tpd_list));

      if (!g_tpd_list) {
        rc = MEMORY_ERROR;
      } else {
        g_tpd_list->list_size = sizeof(tpd_list);
        fwrite(g_tpd_list, sizeof(tpd_list), 1, fhandle);
        fflush(fhandle);
        fclose(fhandle);
      }
    }
  } else {
    /* There is a valid dbfile.bin file - get file size */
//    _fstat(_fileno(fhandle), &file_stat);
    fstat(fileno(fhandle), &file_stat);
    printf("dbfile.bin size = %d\n", (int) file_stat.st_size);

    g_tpd_list = (tpd_list *) calloc(1, (size_t) file_stat.st_size);

    if (!g_tpd_list) {
      rc = MEMORY_ERROR;
    } else {
      fread(g_tpd_list, (size_t) file_stat.st_size, 1, fhandle);
      fflush(fhandle);
      fclose(fhandle);

      if (g_tpd_list->list_size != file_stat.st_size) {
        rc = DBFILE_CORRUPTION;
      }

    }
  }

  return rc;
}

int add_tpd_to_list(tpd_entry *tpd) {
  int rc = 0;
  int old_size = 0;
  FILE *fhandle = NULL;

  if ((fhandle = fopen("dbfile.bin", "wbc")) == NULL) {
    rc = FILE_OPEN_ERROR;
  } else {
    old_size = g_tpd_list->list_size;

    if (g_tpd_list->num_tables == 0) {
      /* If this is an empty list, overlap the dummy header */
      g_tpd_list->num_tables++;
      g_tpd_list->list_size += (tpd->tpd_size - sizeof(tpd_entry));
      fwrite(g_tpd_list, old_size - sizeof(tpd_entry), 1, fhandle);
    } else {
      /* There is at least 1, just append at the end */
      g_tpd_list->num_tables++;
      g_tpd_list->list_size += tpd->tpd_size;
      fwrite(g_tpd_list, old_size, 1, fhandle);
    }

    fwrite(tpd, tpd->tpd_size, 1, fhandle);
    fflush(fhandle);
    fclose(fhandle);
  }

  return rc;
}

int create_table_data_file(char *tab_name, table_file_header *table_file_header, size_t size) {
  FILE *fhandle = nullptr;

  int rc = 0;

  char *file_name = (char *) calloc(1, (int) strlen(tab_name) + 4);
  strcpy(file_name, tab_name);
  strcat(file_name, ".tab");

  char *file_mode = nullptr;

  if (access(file_name, F_OK)) {
    file_mode = (char *) "wbc";
  } else {
    file_mode = (char *) "r+";
  }

  if ((fhandle = fopen(file_name, file_mode)) == NULL) {
    rc = FILE_OPEN_ERROR;

  } else {
    rewind(fhandle);

    fwrite(table_file_header, (size_t) size, 1, fhandle);

    fflush(fhandle);
    fclose(fhandle);
  }

  free(file_name);

  return rc;
}

int drop_tpd_from_list(char *tabname) {
  int rc = 0;
  tpd_entry *cur = &(g_tpd_list->tpd_start);
  int num_tables = g_tpd_list->num_tables;
  bool found = false;
  int count = 0;

  if (num_tables > 0) {
    while ((!found) && (num_tables-- > 0)) {
      if (strcasecmp(cur->table_name, tabname) == 0) {
        /* found it */
        found = true;
        int old_size = 0;
        FILE *fhandle = nullptr;

        if ((fhandle = fopen("dbfile.bin", "wbc")) == nullptr) {
          rc = FILE_OPEN_ERROR;

        } else {
          old_size = g_tpd_list->list_size;

          if (count == 0) {
            /* If this is the first entry */
            g_tpd_list->num_tables--;

            if (g_tpd_list->num_tables == 0) {
              /* This is the last table, null out dummy header */
              memset((void *) g_tpd_list, '\0', sizeof(tpd_list));
              g_tpd_list->list_size = sizeof(tpd_list);
              fwrite(g_tpd_list, sizeof(tpd_list), 1, fhandle);
            } else {
              /* First in list, but not the last one */
              g_tpd_list->list_size -= cur->tpd_size;

              /* First, write the 8 byte header */
              fwrite(g_tpd_list, sizeof(tpd_list) - sizeof(tpd_entry),
                     1, fhandle);

              /* Now write everything starting after the cur entry */
              fwrite((char *) cur + cur->tpd_size,
                     old_size - cur->tpd_size -
                     (sizeof(tpd_list) - sizeof(tpd_entry)),
                     1, fhandle);
            }
          } else {
            /* This is NOT the first entry - count > 0 */
            g_tpd_list->num_tables--;
            g_tpd_list->list_size -= cur->tpd_size;

            /* First, write everything from beginning to cur */
            fwrite(g_tpd_list, ((char *) cur - (char *) g_tpd_list),
                   1, fhandle);

            /* Check if cur is the last entry. Note that g_tdp_list->list_size
               has already subtracted the cur->tpd_size, therefore it will
               point to the start of cur if cur was the last entry */
            if ((char *) g_tpd_list + g_tpd_list->list_size == (char *) cur) {
              /* If true, nothing else to write */
            } else {
              /* NOT the last entry, copy everything from the beginning of the
                 next entry which is (cur + cur->tpd_size) and the remaining size */
              fwrite((char *) cur + cur->tpd_size,
                     old_size - cur->tpd_size -
                     ((char *) cur - (char *) g_tpd_list),
                     1, fhandle);
            }
          }

          fflush(fhandle);
          fclose(fhandle);
        }


      } else {
        if (num_tables > 0) {
          cur = (tpd_entry *) ((char *) cur + cur->tpd_size);
          count++;
        }
      }
    }
  }

  if (!found) {
    rc = INVALID_TABLE_NAME;
  }

  return rc;
}

tpd_entry *get_tpd_from_list(char *tabname) {
  tpd_entry *tpd = nullptr;
  tpd_entry *cur = &(g_tpd_list->tpd_start);
  int num_tables = g_tpd_list->num_tables;
  bool found = false;

  if (num_tables > 0) {
    while ((!found) && (num_tables-- > 0)) {
      if (strcasecmp(cur->table_name, tabname) == 0) {
        /* found it */
        found = true;
        tpd = cur;
      } else {
        if (num_tables > 0) {
          cur = (tpd_entry *) ((char *) cur + cur->tpd_size);
        }
      }
    }
  }

  return tpd;
}

table_file_header *get_file_header(char *tab_name) {

  FILE *fhandle = nullptr;
  struct stat file_stat;
  table_file_header *table_header = nullptr;
  char *file_name = (char *) calloc(1, (int) strlen(tab_name) + 4);
  strcpy(file_name, tab_name);
  strcat(file_name, ".tab");

  if ((fhandle = fopen(file_name, "rbc")) != nullptr) {
    fstat(fileno(fhandle), &file_stat);
    table_header = (table_file_header *) calloc(1, (size_t) file_stat.st_size);
    fread(table_header, (size_t) file_stat.st_size, 1, fhandle);
  }

  free(file_name);
  return table_header;
}

int sem_select(token_list *t_list) {
  int rc = 0;
  token_list *cur_token = t_list;

  if (cur_token->tok_value == F_SUM || cur_token->tok_value == F_COUNT
      || cur_token->tok_value == F_AVG) {

    return sem_select_agg(cur_token);
  }

  token_list *first_col_token = cur_token;

  int cols_print_count = 0;
  token_list *table_name_token = nullptr;

  if (first_col_token->tok_value == S_STAR) {
    // make sure next is FROM
    if (first_col_token->next->tok_value != K_FROM) {
      return INVALID_STATEMENT;
    }
    table_name_token = first_col_token->next->next;
    cur_token = first_col_token->next;

  } else {
    while (cur_token->tok_value != K_FROM) {
      cols_print_count++;
      if (cur_token->next->tok_value == S_COMMA) {
        cur_token = cur_token->next->next;
      } else if (cur_token->next->tok_value == K_FROM) {
        cur_token = cur_token->next;
      } else {
        return INVALID_STATEMENT;
      }
    }
    if (!cols_print_count) {
      return INVALID_STATEMENT;
    }
    table_name_token = cur_token->next;
  }

  if (table_name_token == nullptr) {
    return INVALID_STATEMENT;
  }

  // cur_token points to FROM
  token_list *from_token = cur_token;

  tpd_entry *tpd = get_tpd_from_list(table_name_token->tok_string);

  if (tpd == nullptr) {
    printf("table not found\n");
    return INVALID_TABLE_NAME;
  }

  cd_entry *curr_cd = (cd_entry *) ((char *) tpd + tpd->cd_offset);
  cd_entry *first_cd = curr_cd;

  if (first_col_token->tok_value == S_STAR) {
    cols_print_count = tpd->num_columns;
    cur_token = cur_token->next;
  } else {
    cur_token = first_col_token;
  }

  int* cols_to_print = (int *) calloc((size_t) cols_print_count, sizeof(int)); // indices of which cols to print

  // populate cols_to_print
  for (int i = 0; i < cols_print_count; ++i) {

    if (first_col_token->tok_value == S_STAR) {
      cols_to_print[i] = i;

    } else {
      curr_cd = get_cd(table_name_token->tok_string, cur_token->tok_string);
      if (curr_cd == nullptr) return INVALID_STATEMENT;
      cols_to_print[i] = curr_cd->col_id;
      cur_token = cur_token->next->next;
    }
  }

  // header line 1/3
  printf("+");
  for (int i = 0; i < cols_print_count; ++i) {
    printf("%s+", std::string((unsigned long) get_print_size(
        (first_cd + cols_to_print[i])), '-').c_str());
  }
  printf("\n|");

  // header line 2/3
  for (int i = 0; i < cols_print_count; ++i) {
    printf("%-*s|", get_print_size(first_cd + cols_to_print[i]),
           (first_cd + cols_to_print[i])->col_name);
  }
  printf("\n+");

  // header line 3/3
  for (int i = 0; i < cols_print_count; ++i) {
    printf("%s+", std::string((unsigned long) get_print_size(
        (first_cd + cols_to_print[i])), '-').c_str());
  }
  printf("\n");

  table_file_header *file_header = get_file_header(table_name_token->tok_string);

  if (file_header->num_records == 0) {
    printf("there are no records in this table\n");
    return 0;
  }

  cur_token = from_token->next->next;

  token_list *where_token = nullptr;
  token_list *comp_cond_token = nullptr;
  token_list *order_col_token = nullptr;
  cd_entry *order_col_cd = nullptr;
  int order_by_col_offset = 0;
  bool order_desc = false;

  int first_comp_type = 0;
  token_list *first_comp_val_token = nullptr;
  cd_entry *first_comp_cd = nullptr;
  int first_comp_field_offset = 0;

  int second_comp_type = 0;
  token_list *second_comp_val_token = nullptr;
  cd_entry *second_comp_cd = nullptr;
  int second_comp_field_offset = 0;

  if (cur_token->tok_value == K_WHERE) {

    where_token = cur_token;
    rc = get_compare_vals(where_token, table_name_token->tok_string, first_cd, &first_comp_type,
                          &first_comp_val_token, &first_comp_cd, &first_comp_field_offset);

    if (rc) return rc;

    cur_token = first_comp_val_token->next;

    if (cur_token->tok_value == K_AND || cur_token->tok_value == K_OR) {
      comp_cond_token = cur_token;

      rc = get_compare_vals(comp_cond_token, table_name_token->tok_string, first_cd, &second_comp_type,
                            &second_comp_val_token, &second_comp_cd, &second_comp_field_offset);

      if (rc) return rc;

      cur_token = second_comp_val_token->next;
    }
  }

  if (cur_token->tok_value == K_ORDER) {

    cur_token = cur_token->next;
    if (cur_token->tok_value != K_BY) {
      return INVALID_STATEMENT;
    }

    cur_token = cur_token->next;
    order_col_token = cur_token;
    order_col_cd = get_cd(table_name_token->tok_string, order_col_token->tok_string);

    cd_entry *cd_iter = first_cd;
    while (cd_iter != order_col_cd) order_by_col_offset += (cd_iter++)->col_len + 1;

    if (order_col_cd == nullptr) {
      printf("could not find order by col [%s]\n", order_col_token->tok_string);
    }

    cur_token = cur_token->next;

    order_desc = (cur_token->tok_value == K_DESC);


  } else if (cur_token->tok_class != terminator) {
    printf("unexpected token [%s]", cur_token->tok_string);
    return INVALID_STATEMENT;
  }

  char *first_record = (char *) (file_header + 1);
  char *curr_record = first_record;

  std::vector<char *> record_heads; // push_back(char *) to add values

  for (int i = 0; i < file_header->num_records; ++i, curr_record += file_header->record_size) {

    // no conditional
    if (where_token == nullptr) {
      record_heads.push_back(curr_record);

    } else if (comp_cond_token == nullptr) {

      // where only
      if (satisfies_condition(curr_record + first_comp_field_offset, first_comp_type,
                          first_comp_val_token, first_comp_cd->col_len)) {

        record_heads.push_back(curr_record);
      }

    } else if (comp_cond_token->tok_value == K_AND) {

      // where cond1 AND cond2
      if (satisfies_condition(curr_record + first_comp_field_offset, first_comp_type,
                              first_comp_val_token, first_comp_cd->col_len)
          && satisfies_condition(curr_record + second_comp_field_offset, second_comp_type,
                                 second_comp_val_token, second_comp_cd->col_len)) {

        record_heads.push_back(curr_record);

      }

    } else if (comp_cond_token->tok_value == K_OR) {

      if (satisfies_condition(curr_record + first_comp_field_offset, first_comp_type,
                              first_comp_val_token, first_comp_cd->col_len)
          || satisfies_condition(curr_record + second_comp_field_offset, second_comp_type,
                                 second_comp_val_token, second_comp_cd->col_len)) {

        record_heads.push_back(curr_record);

      }
    }
  }

  if (order_col_token != nullptr) {
    std::sort(record_heads.begin(), record_heads.end(), std::bind(compare_records_by_val, _1, _2,
                        order_col_cd, order_by_col_offset, order_desc));
  }

  for (int i = 0; i < record_heads.size(); ++i) {

    printf("|");

    for (int j = 0; j < cols_print_count; ++j) {

      cd_entry *print_cd = first_cd + cols_to_print[j];
      char *print_field = record_heads.at((unsigned long) i);

      cd_entry *cd_iter = first_cd;
      while (cd_iter != print_cd) print_field += (cd_iter++)->col_len + 1;

      int col_print_size = get_print_size(print_cd);

      if ((int) print_field[0] == 0) {
        // todo :: left align String and right align ints (even for null values)
        printf("%sNULL|", std::string(col_print_size - 4, ' ').c_str());

      } else if (print_cd->col_type == T_CHAR) {

        char *data = (char *) calloc(1, (size_t) print_cd->col_len);
        memcpy(data, (print_field + 1), (size_t) print_cd->col_len);
        printf("%-*s|", col_print_size, data);
        free(data);

      } else if (print_cd->col_type == T_INT) {

        int *data = (int *) calloc(1, sizeof(int));
        memcpy(data, &(print_field + 1)[0], sizeof(int));
        printf("%*d|", col_print_size, *data);
        free(data);

      } else {
        printf("unexpected type\n");
      }
    }

    printf("\n");
  }

  // bottom of table
  printf("+");
  for (int i = 0; i < cols_print_count; ++i) {
    printf("%s+", std::string((unsigned long) get_print_size(
        (first_cd + cols_to_print[i])), '-').c_str());
  }
  printf("\n");

  printf("%d rows selected.\n", (int) record_heads.size());

  free(file_header);
  free(cols_to_print);
  return 0;
}

int sem_select_agg(token_list *t_list) {

  int rc = 0;

  token_list *cur_token = t_list;
  token_list *agg_token = cur_token;
  cur_token = cur_token->next;

  if (cur_token->tok_value != S_LEFT_PAREN) {
    return INVALID_STATEMENT;
  }

  cur_token = cur_token->next;
  token_list *sel_col_token = cur_token;
  cur_token = cur_token->next;

  if (cur_token->tok_value != S_RIGHT_PAREN) {
    return INVALID_STATEMENT;
  }

  cur_token = cur_token->next;

  if (cur_token->tok_value != K_FROM) {
    return INVALID_STATEMENT; 
  }
  
  cur_token = cur_token->next;

  tpd_entry *curr_table_tpd = get_tpd_from_list(cur_token->tok_string);
  if (curr_table_tpd == nullptr) { 
    printf("table not found\n");
    return TABLE_NOT_EXIST;
  }

  cd_entry *sel_col_cd = nullptr;
  if (sel_col_token->tok_value == S_STAR) {
    if (agg_token->tok_value != F_COUNT) {
      printf("%s cannot be used with *\n", agg_token->tok_value == F_AVG ? "AVG" : "SUM");
      return INVALID_STATEMENT;
    }

  } else {
    sel_col_cd = get_cd(cur_token->tok_string, sel_col_token->tok_string);
    if (sel_col_cd == nullptr) {
      printf("could not find column [%s]\n", sel_col_token->tok_string);
      return COLUMN_NOT_EXIST;
    }
  }

  token_list *table_name_token = cur_token;
  cur_token = cur_token->next;
  cd_entry *first_cd = (cd_entry *) (((char *) curr_table_tpd) + curr_table_tpd->cd_offset);

  token_list *where_token = nullptr;
  token_list *comp_cond_token = nullptr;

  int first_comp_type = 0;
  token_list *first_comp_val_token = nullptr;
  cd_entry *first_comp_cd = nullptr;
  int first_comp_field_offset = 0;

  int second_comp_type = 0;
  token_list *second_comp_val_token = nullptr;
  cd_entry *second_comp_cd = nullptr;
  int second_comp_field_offset = 0;

  if (cur_token->tok_value == K_WHERE) {
    where_token = cur_token;
    rc = get_compare_vals(where_token, table_name_token->tok_string, first_cd, &first_comp_type,
                          &first_comp_val_token, &first_comp_cd, &first_comp_field_offset);

    if (rc) return rc;

    cur_token = first_comp_val_token->next;

    if (cur_token->tok_value == K_AND || cur_token->tok_value == K_OR) {
      comp_cond_token = cur_token;

      rc = get_compare_vals(comp_cond_token, table_name_token->tok_string, first_cd, &second_comp_type,
                            &second_comp_val_token, &second_comp_cd, &second_comp_field_offset);

      if (rc) return rc;

      cur_token = second_comp_val_token->next;
    }

  } else if (cur_token->tok_class != terminator && cur_token->tok_value != K_ORDER) {
    return INVALID_STATEMENT;
  }


  if ((agg_token->tok_value == F_AVG || agg_token->tok_value == F_SUM)
      && (sel_col_cd->col_type == T_CHAR)) {

    printf("%s must be used with an INT column type\n",
           agg_token->tok_value == F_AVG ? "AVG" : "SUM");

    return INVALID_STATEMENT;
  }

  table_file_header *file_header = get_file_header(table_name_token->tok_string);

  char *record_head = ((char *) file_header) + file_header->record_offset;
  char *curr_field = ((char *) file_header) + file_header->record_offset;

  cd_entry *cd_iter = (cd_entry *) (((char *) curr_table_tpd) + curr_table_tpd->cd_offset);
  while (cd_iter != sel_col_cd && (sel_col_cd != nullptr)) curr_field += (cd_iter++)->col_len + 1;

  long sum = 0;
  int count = 0;

  for (int i = 0; i < file_header->num_records; ++i,
      curr_field += file_header->record_size, record_head += file_header->record_size) {

    bool is_cond_satisfied = false;

    // no conditional
    if (where_token == nullptr) {
      is_cond_satisfied = true;

    } else if (comp_cond_token == nullptr) {

      // where only
      if (satisfies_condition(record_head + first_comp_field_offset, first_comp_type,
                              first_comp_val_token, first_comp_cd->col_len)) {

        is_cond_satisfied = true;
      }

    } else if (comp_cond_token->tok_value == K_AND) {

      // where cond1 AND cond2
      if (satisfies_condition(record_head + first_comp_field_offset, first_comp_type,
                              first_comp_val_token, first_comp_cd->col_len)
          && satisfies_condition(record_head + second_comp_field_offset, second_comp_type,
                                 second_comp_val_token, second_comp_cd->col_len)) {

        is_cond_satisfied = true;

      }

    } else if (comp_cond_token->tok_value == K_OR) {

      if (satisfies_condition(record_head + first_comp_field_offset, first_comp_type,
                              first_comp_val_token, first_comp_cd->col_len)
          || satisfies_condition(record_head + second_comp_field_offset, second_comp_type,
                                 second_comp_val_token, second_comp_cd->col_len)) {

        is_cond_satisfied = true;

      }
    }

    if (is_cond_satisfied) {
      if (sel_col_token->tok_value == S_STAR) {
        count++;

      } else {

        if (((int) curr_field[0]) != 0) count++;

        if (sel_col_cd->col_type == T_INT) {
          int *data = (int *) calloc(1, sizeof(int));
          memcpy(data, &(curr_field + 1)[0], sizeof(int));
          sum += *data;
          free(data);
        }
      }
    }
  }

  float avg = ((float) sum) / count;

  std::string result_str;

  if (agg_token->tok_value == F_SUM) result_str = std::to_string(sum);
  else if (agg_token->tok_value == F_AVG) result_str = std::to_string(avg);
  else result_str = std::to_string(count);

  std::string title_str = std::string(agg_token->tok_string);
  title_str.append("(");
  title_str.append(sel_col_token->tok_string);
  title_str.append(")");

  int print_width = (int) fmax(result_str.length(), title_str.length());

  printf("+%s+\n", std::string((unsigned long) print_width, '-').c_str());
  printf("|%-*s|\n", print_width, title_str.c_str());
  printf("+%s+\n", std::string((unsigned long) print_width, '-').c_str());
  printf("|%+*s|\n", print_width, result_str.c_str());
  printf("+%s+\n", std::string((unsigned long) print_width, '-').c_str());

  free(file_header);
  return rc;
}

int sem_update_value(token_list *cur_token) {
  int rc = 0;
  char *table_name = cur_token->tok_string;
  tpd_entry *curr_table_tpd = get_tpd_from_list(table_name);

  if (curr_table_tpd == nullptr) {
    return TABLE_NOT_EXIST;
  }

  cur_token = cur_token->next->next; // do_semantic already checked for "SET"

  char *set_col_name = cur_token->tok_string;

  cd_entry *first_cd = (cd_entry *) ((char *) curr_table_tpd + curr_table_tpd->cd_offset);
  cd_entry *set_cd = get_cd(table_name, set_col_name);

  cur_token = cur_token->next;

  if (cur_token->tok_value != S_EQUAL || cur_token->next == nullptr) {
    return INVALID_STATEMENT; // todo :: invalid insert statement
  }

  cur_token = cur_token->next;

  token_list *set_data_token = cur_token;
  int set_value_size;

  switch (cur_token->tok_value) {
    case INT_LITERAL:
      set_value_size = sizeof(int);
      break;
    case STRING_LITERAL:
      set_value_size = (int) strlen(cur_token->tok_string);
      break;
    default:
      set_value_size = 0;
      break;
  }

  // check against cd col length, type, and not null
  if ((set_data_token->tok_value == INT_LITERAL && set_cd->col_type != T_INT)
      || (set_data_token->tok_value == STRING_LITERAL && set_cd->col_type != T_CHAR)) {
    printf("Wrong col type (int vs char)\n");
    rc = INVALID_STATEMENT;
  }

  if ((set_cd->col_type == T_CHAR) && ((set_cd->col_len < strlen(set_data_token->tok_string)
                                        && set_data_token->tok_value != K_NULL))) {
    printf("String input too big\n");
    rc = INVALID_STATEMENT;
  }

  if (set_cd->not_null && set_data_token->tok_value == K_NULL) {
    printf("NULL not allowed\n");
    rc = INVALID_STATEMENT;
  }

  if (rc) {
    printf("0 rows updated.\n");
    return rc;
  }

  table_file_header *table_header = get_file_header(table_name);

  char *first_record = ((char *) table_header) + table_header->record_offset;

  int field_offset = 0;
  cd_entry *cd_iter = first_cd;
  for (int i = 0; i < set_cd->col_id; ++i, ++cd_iter) {
    field_offset += cd_iter->col_len + 1;
  }

  char *curr_set_field = first_record + field_offset; // points to size [SIZE][DATA]

  if (cur_token->next->tok_class == terminator) {

    for (int i = 0; i < table_header->num_records; ++i) {
      memcpy(curr_set_field, &set_value_size, 1); // write new size
      if (set_data_token->tok_value == INT_LITERAL) {
        int val = (set_data_token->tok_value == K_NULL) ? 0 : atoi(set_data_token->tok_string);
        memcpy(curr_set_field + 1, &val, sizeof(int));
      } else {
        memset(curr_set_field + 1, '\0', (size_t) set_cd->col_len);
        if (set_data_token->tok_value != K_NULL) {
          memcpy(curr_set_field + 1, set_data_token->tok_string, strlen(set_data_token->tok_string));
        }
      }
      curr_set_field += table_header->record_size;
    }

    printf("%d rows updated.\n", table_header->num_records);

  } else {

    int comp_type = 0;
    token_list *compare_value_token = nullptr;
    cd_entry *compare_cd = nullptr;
    int comp_field_offset = 0;

    rc = get_compare_vals(cur_token->next, table_name, first_cd, &comp_type,
                          &compare_value_token, &compare_cd, &comp_field_offset);

    if (rc) return rc;

    int num_rows_updated = 0;
    char *curr_compare_field = first_record + comp_field_offset; // points to DATA (not size prefix)

    for (int i = 0; i < table_header->num_records; ++i) {
      if (satisfies_condition(curr_compare_field, comp_type,
                              compare_value_token, compare_cd->col_len)) {

        num_rows_updated++;
        memcpy(curr_set_field, &set_value_size, 1); // write new size
        if (set_data_token->tok_value == INT_LITERAL) {
          int val = (set_data_token->tok_value == K_NULL) ? 0 : atoi(set_data_token->tok_string);
          memcpy(curr_set_field + 1, &val, sizeof(int));
        } else {
          memset(curr_set_field + 1, '\0', (size_t) set_cd->col_len);
          if (set_data_token->tok_value != K_NULL) {
            memcpy(curr_set_field + 1, set_data_token->tok_string, strlen(set_data_token->tok_string));
          }
        }

      }
      curr_set_field += table_header->record_size;
      curr_compare_field += table_header->record_size;
    }

    printf("%d rows updated.\n", num_rows_updated);
  }

  // copy data back
  delete_tab_file(table_name);
  create_table_data_file(table_name, table_header, (size_t) table_header->file_size);

  free(table_header);
  return 0;
}

int sem_delete_value(token_list *cur_token) {

  int rc = 0;
  char *table_name = cur_token->tok_string;
  tpd_entry *curr_table_tpd = get_tpd_from_list(table_name);

  if (curr_table_tpd == nullptr) {
    printf("table not found\n");
    return TABLE_NOT_EXIST;
  }

  cd_entry *first_cd = (cd_entry *) ((char *) curr_table_tpd + curr_table_tpd->cd_offset);

  cur_token = cur_token->next;
  table_file_header *file_header = get_file_header(table_name);

  // "delete from table" --> delete all rows
  if (cur_token->tok_class == terminator) {

    int num_rows_deleted = file_header->num_records;

    file_header->file_size = sizeof(table_file_header_def);
    file_header->num_records = 0;
    file_header->record_offset = 0;

    delete_tab_file(table_name);
    create_table_data_file(table_name, file_header, sizeof(table_file_header));

    printf("%d rows deleted.\n", num_rows_deleted);

  } else {

    int comp_type = 0;
    token_list *compare_value_token = nullptr;
    cd_entry *compare_cd = nullptr;
    int comp_field_offset = 0;

    rc = get_compare_vals(cur_token, table_name, first_cd, &comp_type,
                          &compare_value_token, &compare_cd, &comp_field_offset);

    if (rc) return rc;

    int num_rows_deleted = 0;

    char *first_record = (char *) file_header + file_header->record_offset;
    char *cur_record = first_record;

    int i = 0;
    while (i < file_header->num_records) {

      if (satisfies_condition(
          (cur_record + comp_field_offset), comp_type, compare_value_token, compare_cd->col_len)) {

        // only copy if not last row
        if (i != file_header->num_records - 1) {
          char *last_row = first_record + (file_header->record_size * (file_header->num_records - 1));
          memcpy(cur_record, last_row, (size_t) file_header->record_size);
        }

        file_header->num_records--;
        num_rows_deleted++;

      } else {
        i++;
        cur_record += file_header->record_size;
      }

    }

    file_header->file_size =
        sizeof(table_file_header_def) + (file_header->num_records * file_header->record_size);

    printf("%d rows deleted.\n", num_rows_deleted);

    delete_tab_file(table_name);
    create_table_data_file(table_name, file_header, (size_t) file_header->file_size);
  }

  free(file_header);
  return rc;
}

int get_compare_vals(token_list *cur_token, char *table_name, cd_entry *first_cd, int *comp_type,
                     token_list **comp_value_token, cd_entry **compare_cd, int *comp_field_offset) {

  if ((cur_token->tok_value != K_WHERE && cur_token->tok_value != K_OR
       && cur_token->tok_value != K_AND)
      || cur_token->next->tok_class == terminator) {

    return INVALID_STATEMENT;
  }

  cur_token = cur_token->next;
  char *col_name = cur_token->tok_string;
  cur_token = cur_token->next;

  if (cur_token == nullptr || cur_token->next == nullptr
      || (cur_token->tok_value != S_EQUAL
          && cur_token->tok_value != S_LESS
          && cur_token->tok_value != S_GREATER
          && cur_token->tok_value != K_IS))  {

    return INVALID_STATEMENT;
  }

  *comp_type = cur_token->tok_value;
  *comp_value_token = cur_token->next;

  if (cur_token->tok_value == K_IS) {
    cur_token = cur_token->next;

    if (cur_token->tok_value == K_NOT) {
      cur_token = cur_token->next;

      if (cur_token->tok_value == K_NULL) {
        // IS NOT NULL
        *comp_type = K_NOT;
        *comp_value_token = cur_token;

      } else {
        return INVALID_STATEMENT;
      }

    } else if (cur_token->tok_value == K_NULL) {
      // IS NULL
      *comp_type = K_IS;
      *comp_value_token = cur_token;

    } else {
      return INVALID_STATEMENT;
    }

  }

  *compare_cd = get_cd(table_name, col_name);

  if (*compare_cd == nullptr) {
    return COLUMN_NOT_EXIST;
  }

  if (((*comp_value_token)->tok_value != K_NULL)
      && (((*compare_cd)->col_type == T_INT && (*comp_value_token)->tok_value != INT_LITERAL)
      || ((*compare_cd)->col_type == T_CHAR && (*comp_value_token)->tok_value != STRING_LITERAL))) {

    return INVALID_STATEMENT; // todo :: invalid compare value type
  }

  *comp_field_offset = 1;
  cd_entry *cd_iter = first_cd;
  for (int i = 0; i < (*compare_cd)->col_id; ++i, ++cd_iter) {
    *comp_field_offset += cd_iter->col_len + 1;
  }

  return 0;
}

bool satisfies_condition(char *field, int operator_type, token_list *comp_value_token, int col_len) {

  if (operator_type == K_IS && comp_value_token->tok_value == K_NULL) {
    return *(field - 1) == '\0';
  }

  if (operator_type == K_NOT && comp_value_token->tok_value == K_NULL) {
    return *(field - 1) != '\0';
  }

  if (*(field - 1) == '\0') {
    return false;
  }

  int cmp_val;

  if (comp_value_token->tok_value == INT_LITERAL) {
    int *data = (int *) calloc(1, sizeof(int));
    memcpy(data, field, sizeof(int));
    cmp_val = *data - atoi(comp_value_token->tok_string);
    free(data);
  } else {
    char *data = (char *) calloc(1, (size_t) col_len);
    memcpy(data, field, (size_t) col_len);
    cmp_val = strcmp(data, comp_value_token->tok_string);
    free(data);
  }

  switch (operator_type) {
    case S_EQUAL:
      return !cmp_val;
    case S_LESS:
      return cmp_val < 0;
    case S_GREATER:
      return cmp_val > 0;
    default:
      return false;
  }
}

cd_entry *get_cd(char *table_name, char *col_name) {

  tpd_entry *this_tpd = get_tpd_from_list(table_name);

  if (this_tpd == nullptr) {
    return nullptr;
  }

  cd_entry *curr_cd = (cd_entry *) ((char *) this_tpd + this_tpd->cd_offset);

  for (int i = 0; i < this_tpd->num_columns; ++i, ++curr_cd) {
    if (!strcmp(curr_cd->col_name, col_name)) {
      return curr_cd;
    }
  }

  return nullptr;
}

int get_print_size(cd_entry *cd) {
  return (cd->col_type == T_CHAR) && cd->col_len > 16 ? cd->col_len : 16;
}

int delete_tab_file(char *tab_name) {

  char *file_name = (char *) calloc(1, (int) strlen(tab_name) + 4);
  strcpy(file_name, tab_name);
  strcat(file_name, ".tab");

  free(file_name);
  return remove(file_name);
}

bool compare_records_by_val(const char *record_a, const char *record_b,
                                cd_entry *order_cd, int field_offset, bool desc) {

  if ((int) ((record_a + field_offset)[0]) == 0) {
    return false;

  } else if((int) ((record_b + field_offset)[0]) == 0) {
    return true;

  } else if (order_cd->col_type == T_INT) {

    int *data_a = (int *) calloc(1, sizeof(int));
    int *data_b = (int *) calloc(1, sizeof(int));

    memcpy(data_a, &(record_a + field_offset + 1)[0], sizeof(int));
    memcpy(data_b, &(record_b + field_offset + 1)[0], sizeof(int));

    bool ret_val = (*data_a < *data_b);

    free(data_a);
    free(data_b);

    return desc != ret_val;

  } else {

    char *data_a = (char *) calloc(1, (size_t) order_cd->col_len);
    char *data_b = (char *) calloc(1, (size_t) order_cd->col_len);

    memcpy(data_a, (record_a + field_offset + 1), (size_t) order_cd->col_len);
    memcpy(data_b, (record_b + field_offset + 1), (size_t) order_cd->col_len);

    bool ret_val = (strcmp(data_a, data_b) < 0);

    free(data_a);
    free(data_b);

    return desc != ret_val;
  }

}
