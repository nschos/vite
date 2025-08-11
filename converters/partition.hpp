

#ifndef __PARTITION_H
#define __PARTITION_H

#include <string>

#include "../utils.hpp"
#include "../graph.hpp"

void loadPartitionFile(Graph *&g, const std::string &fileName, const std::string &partitionFileName, bool indexOneBased, Weight_t wtype = ABS_WEIGHT);
void loadPartitionFile2(Graph *&g, const std::string &fileName, const std::string &partitionFileName, Weight_t wtype);

#endif // __LOAD_SIMPLE_H
