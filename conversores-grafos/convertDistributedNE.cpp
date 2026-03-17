#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <limits>

using namespace std;

int NUM_VERTICES;
int NUM_PARTITIONS;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Uso: " << argv[0] << " <arquivo_entrada> <num_vertices> <num_part>\n";
        return 1;
    }

    string inputFile = argv[1];
    string outputFile = argv[1] + std::string(".converted");
    NUM_VERTICES = std::atoi(argv[2]);
    NUM_PARTITIONS = std::atoi(argv[3]);

    // vertexPartitions[v][p] = número de vezes que o vértice v apareceu na partição p
    vector<vector<int>> vertexPartitions(NUM_VERTICES + 1, vector<int>(NUM_PARTITIONS, 0));
    // Contador global de quantos vértices pertencem a cada partição final
    vector<int> partitionVertexCount(NUM_PARTITIONS, 0);

    ifstream infile(inputFile);
    if (!infile) {
        cerr << "Erro ao abrir arquivo de entrada: " << inputFile << "\n";
        return 1;
    }

    string line;
    // Ignorar cabeçalho (primeira linha)
    getline(infile, line);

    int src, dst, part;
    while (infile >> src >> dst >> part) {
        if (src <= NUM_VERTICES && dst <= NUM_VERTICES && part < NUM_PARTITIONS) {
            vertexPartitions[src][part]++;
            vertexPartitions[dst][part]++;
        }
    }
    infile.close();

    ofstream outfile(outputFile);
    if (!outfile) {
        cerr << "Erro ao criar arquivo de saída: " << outputFile << "\n";
        return 1;
    }

    // Determinar partição final de cada vértice
    for (int v = 1; v <= NUM_VERTICES; ++v) {
        int bestPartition = 0;
        int maxCount = -1;

        for (int p = 0; p < NUM_PARTITIONS; ++p) {
            int count = vertexPartitions[v][p];
            if (count > maxCount) {
                maxCount = count;
                bestPartition = p;
            }
            else if (count == maxCount && maxCount > 0) {
                // Empate: partição com menos vértices
                if (partitionVertexCount[p] < partitionVertexCount[bestPartition]) {
                    bestPartition = p;
                }
            }
        }

        // Se o vértice não apareceu em nenhuma aresta
        if (maxCount == 0) {
            int minSize = numeric_limits<int>::max();
            for (int p = 0; p < NUM_PARTITIONS; ++p) {
                if (partitionVertexCount[p] < minSize) {
                    minSize = partitionVertexCount[p];
                    bestPartition = p;
                }
            }
        }

        partitionVertexCount[bestPartition]++;
        outfile << bestPartition << "\n";
    }

    outfile.close();

    cout << "Arquivo de saída: " << outputFile << "\n";
    return 0;
}