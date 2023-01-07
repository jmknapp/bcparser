#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>

void version() {
  printf("MySQL client version: %s\n", mysql_get_client_info());
}
