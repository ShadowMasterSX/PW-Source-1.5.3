#include <string.h>
#include "amemory.h"

namespace abase
{
fast_allocator::node_t fast_allocator::_a_table[fast_allocator::MAX_SIZE];

int fast_allocator::_other_counter = 0;
int fast_allocator::_inside_counter = 0;
int fast_allocator::_large_size_counter[] = {0};

}
