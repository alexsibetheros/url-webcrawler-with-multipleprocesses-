#ifndef hashhead
#define hashhead

#include "core.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
l_node_point* create_table(int table_size);
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int hash_function(char* key, int size);
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void insert_into_hash(core_point my_core, char* data, int place);
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void print_hash(core_point my_core);
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void delete_hash(core_point my_core);
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int check_url(core_point my_core, char* get_url);
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif /*hashhead*/


