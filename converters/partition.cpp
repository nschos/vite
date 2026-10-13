#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <vector>
#include <utility>
#include <random>

#include "partition.hpp"



void loadPartitionFile2(Graph *&g, const std::string &fileName, const std::string &partitionFileName, Weight_t wtype)
{
  std::ifstream ifs;

  ifs.open(fileName.c_str());
  if (!ifs) {
    std::cerr << "Error opening Matrix Market format file: " << fileName << std::endl;
    exit(EXIT_FAILURE);
  }

  std::string line;
  bool isComplex = false, isPattern = false, isSymmetric = false, isGeneral = false;

  std::getline(ifs, line);
  {
    std::string ls[5];

    std::istringstream iss(line);
    iss >> ls[0] >> ls[1] >> ls[2] >> ls[3] >> ls[4];

    if (!iss) {
      std::cerr << "Error parsing matrix market file: " << fileName << std::endl;
      exit(EXIT_FAILURE);
    }

    std::cout << "Processing: " << ls[0] << " " << ls[1] << " " << ls[2] << " " << ls[3] <<
      " " << ls[4] << std::endl;
    for (int i = 1; i < 5; i++)
      std::transform(ls[i].begin(), ls[i].end(), ls[i].begin(), toupper);

    if (ls[0] != "%%MatrixMarket") {
      std::cerr << "Error in the first line of: " << fileName << " " << ls[0] <<
	", expected: %%MatrixMarket" << std::endl;
      exit(EXIT_FAILURE);
    }

    if (ls[1] != "MATRIX") {
      std::cerr << "The object type should be matrix for file: " << fileName << std::endl;
      exit(EXIT_FAILURE);
    }

    if (ls[2] != "COORDINATE") {
      std::cerr << "The object type should be coordinate for file: " << fileName << std::endl;
      exit(EXIT_FAILURE);
    }

    if (ls[3] == "COMPLEX") {
      std::cout << "Warning will only process the real part" << std::endl;
      isComplex = true;
    }

    if (ls[3] == "PATTERN") {
      std::cout << "Note: matrix type is pattern.  All weights will be set to 1.0 unless -r is passed" <<
	std::endl;
      isPattern = true;
    }

    if (ls[4] == "GENERAL")
      isGeneral = true;
    else if (ls[4] == "SYMMETRIC") {
      std::cout << "Note: matrix type is symmetric" << std::endl;
      isSymmetric = true;
    }
  }

  if (!isSymmetric && !isGeneral) {
    std::cerr << "Error: matrix market type should be SYMMETRIC or GENERAL for file: " << fileName <<
      std::endl;
    exit(EXIT_FAILURE);
  }

  do {
    std::getline(ifs, line);
  } while (line[0] == '%');

  GraphElem numVertices, numEdges;
  GraphElem ns, nt, ne;

  std::istringstream iss(line);

  iss >> ns >> nt >> ne;
  if (!iss || iss.fail()) {
      std::cerr << "Error parsing Matrix Market format: " << line << std::endl;
      exit(EXIT_FAILURE);
  }

  numVertices = ns;
  numEdges = ne;

  std::cout << "Loading Matrix Market file: " << fileName << ", numvertices: " << numVertices <<
      ", numEdges: " << numEdges << std::endl;
  
  std::string crd;
  GraphElem source, dest;
  GraphWeight weight = 1.0;
  std::vector<GraphElem> edgeCount(numVertices+1);
  std::vector<GraphElemTuple> edgeList;

  // weights will be converted to positive numbers
  if (isSymmetric) {

      for (GraphElem i = 0; i < numEdges; i++) {

          std::getline(ifs, crd);
          std::istringstream iss(crd);

          if (isPattern)
              iss >> source >> dest;
          else {
              iss >> source >> dest >> weight;
              if (wtype == ABS_WEIGHT)
                  weight = std::fabs(weight);
          }

          source--; // Matrix market has 1-based indexing
          dest--;

          assert((source >= 0) && (source < numVertices));
          assert((dest >= 0) && (dest < numVertices));

          if (wtype == ONE_WEIGHT)
              weight = 1.0;

          if (wtype == RND_WEIGHT)
              weight = genRandom(RANDOM_MIN_WEIGHT, RANDOM_MAX_WEIGHT);

          if (source != dest) {
              edgeList.emplace_back(source, dest, weight);
              edgeList.emplace_back(dest, source, weight);
              edgeCount[source+1]++;
              edgeCount[dest+1]++;
          }
          else {
              edgeList.emplace_back(source, dest, weight);
              edgeCount[source+1]++;
          }
      }
  }
  else { // if General type (directed)
      
      for (GraphElem i = 0; i < numEdges; i++) {

          std::getline(ifs, crd);
          std::istringstream iss(crd);

          if (isPattern)
              iss >> source >> dest;
          else {
              iss >> source >> dest >> weight;
              if (wtype == ABS_WEIGHT)
                  weight = std::fabs(weight);
          }

          source--; // Matrix market has 1-based indexing
          dest--;

          assert((source >= 0) && (source < numVertices));
          assert((dest >= 0) && (dest < numVertices));

          if (wtype == ONE_WEIGHT)
              weight = 1.0;

          if (wtype == RND_WEIGHT)
              weight = genRandom(RANDOM_MIN_WEIGHT, RANDOM_MAX_WEIGHT);

          edgeList.emplace_back(source, dest, weight);
          edgeCount[source+1]++;
      }
  }

  ifs.close();

  // PARTITIONING FILE ////////////////
  std::vector<GraphElem> partitionByIndex(numVertices);
  std::vector<GraphElem> partitionCount;
  GraphElem index = 0;
  
  ifs.open(partitionFileName.c_str(), std::ifstream::in);
  if (!ifs) {
    std::cerr << "Error opening Simple format file: " << partitionFileName << std::endl;
    exit(EXIT_FAILURE);
  }

  GraphElem partition;

  do {
    std::getline(ifs, line);
    std::istringstream iss(line);

    if (!line.empty()) {
      iss >> partition;

      partitionByIndex[index] = partition;

      while (partitionCount.size() <= partition) {
        partitionCount.push_back(0);
      }

      partitionCount[partition]++;
      index++;
    }
  } while (!ifs.eof());

  ifs.close();
  // END PARTITIONING FILE ////////////

  numEdges = edgeList.size();

  g = new Graph(numVertices, numEdges);

  // PARTITION /////////////////////
  std::vector<GraphElem> newPartitionCounter(partitionCount.size());
  std::vector<GraphElem> newIndex(numVertices);

  for (int i = 0; i < numVertices; i++) {
    GraphElem partition = partitionByIndex[i];
    GraphElem index = 0;
    for (int j = 0; j < partition; j++) {
      index += partitionCount[j];
    }
    index += newPartitionCounter[partition];
    newPartitionCounter[partition]++;
    newIndex[i] = index;
  }

  for (int i = 0; i < edgeList.size(); i++) {  
    edgeList[i].i_ = newIndex[edgeList[i].i_];
    edgeList[i].j_ = newIndex[edgeList[i].j_];
  }
  // END PARTITION ////////////////////////////////

  processGraphData(*g, edgeCount, edgeList, numVertices, numEdges, true);

  g->partCounter = newPartitionCounter;
} // loadMatrixMarketFile



void loadPartitionFile(Graph *&g, const std::string &fileName, const std::string &partitionFileName, bool indexOneBased, Weight_t wtype)
{
  std::ifstream ifs;

  double t0, t1;

  t0 = mytimer();

  ifs.open(fileName.c_str(), std::ifstream::in);
  if (!ifs) {
    std::cerr << "Error opening Simple format file: " << fileName << std::endl;
    exit(EXIT_FAILURE);
  }

  std::string line;
  GraphElem maxVertex = -1, numEdges = 0, numVertices, skipLines = 0;

  do {
    GraphElem v0, v1;
    GraphWeight w = 0.0;

    std::getline(ifs, line);

    if(line[0]=='#' || line[0]=='%') {
        skipLines++;
        continue;
    }

    std::istringstream iss(line);
    if (wtype == ORG_WEIGHT || wtype == ABS_WEIGHT)
        iss >> v0 >> v1 >> w;
    else
        iss >> v0 >> v1;

    if (indexOneBased) {
        v0--; 
        v1--;
    }

    if (v0 > maxVertex)
      maxVertex = v0;
    if (v1 > maxVertex)
      maxVertex = v1;
      
    numEdges++;

  } while (!ifs.eof());

  numVertices = maxVertex + 1;
  numEdges -= 1; // last line

  std::cout << "Loading Simple file: " << fileName << ", numvertices: " << numVertices << std::endl;

  ifs.close();
 
  std::vector<GraphElem> partitionByIndex(numVertices);
  std::vector<GraphElem> partitionCount;
  GraphElem index = 1;
  
  // if (partitionBased) {
    ifs.open(partitionFileName.c_str(), std::ifstream::in);
    if (!ifs) {
      std::cerr << "Error opening Simple format file: " << partitionFileName << std::endl;
      exit(EXIT_FAILURE);
    }

    GraphElem partition;

    do {
      std::getline(ifs, line);
      std::istringstream iss(line);

      if (!line.empty()) {
        iss >> partition;

        partitionByIndex[index] = partition;

        while (partitionCount.size() <= partition) {
          partitionCount.push_back(0);
        }

        partitionCount[partition]++;
        index++;
      }
    } while (!ifs.eof());

    ifs.close();
  // }
  
  // file bounds known, start reading data
  ifs.open(fileName.c_str(), std::ifstream::in);

  std::vector<GraphElem> edgeCount(numVertices + 1);
  std::vector<GraphElemTuple> edgeList;

  for (GraphElem i = 0; i < numEdges+skipLines; i++) {
    GraphElem v0, v1;
    GraphWeight w = 0.0;

    std::getline(ifs, line);
    if(line[0]=='#' || line[0]=='%')		
        continue;
    
    std::istringstream iss(line);
 
    if (wtype == ORG_WEIGHT || wtype == ABS_WEIGHT)
        iss >> v0 >> v1 >> w;
    else
        iss >> v0 >> v1;

    if (indexOneBased) {
        v0--; 
        v1--;
    }
     
    if (wtype == ONE_WEIGHT)
        w = 1.0;

    if (wtype == RND_WEIGHT)
        w = genRandom(RANDOM_MIN_WEIGHT, RANDOM_MAX_WEIGHT);
    
    if (wtype == ABS_WEIGHT)
        w = std::fabs(w);

    edgeList.push_back({v0, v1, w});
    edgeCount[v0+1]++;
  }

  ifs.close();

  numEdges = edgeList.size();
  g = new Graph(numVertices, numEdges);

  std::vector<GraphElem> newPartitionCounter(partitionCount.size());
  // if (partitionBased) {
    std::vector<GraphElem> newIndex(numVertices);
  
    for (int i = 1; i < numVertices; i++) {
      GraphElem partition = partitionByIndex[i];
      GraphElem index = 1;
      for (int j = 0; j < partition; j++) {
        index += partitionCount[j];
      }
      index += newPartitionCounter[partition];
      newPartitionCounter[partition]++;
      newIndex[i] = index;
    }

    for (int i = 0; i < edgeList.size(); i++) {  
      edgeList[i].i_ = newIndex[edgeList[i].i_];
      edgeList[i].j_ = newIndex[edgeList[i].j_];
    }
  // }

  processGraphData(*g, edgeCount, edgeList, numVertices, numEdges);
  // if (partitionBased) {
    g->partCounter = newPartitionCounter;
  // }
  
  t1 = mytimer();  
  std::cout << "Total graph processing time: " << (t1 - t0) << std::endl;
} // loadSimpleFile
