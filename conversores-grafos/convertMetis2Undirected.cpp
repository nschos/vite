#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

void metisToEdgeList(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream infile(inputFile);
    std::ofstream outfile(outputFile);

    if (!infile.is_open()) {
        std::cerr << "Erro ao abrir arquivo de entrada." << std::endl;
        return;
    }
    if (!outfile.is_open()) {
        std::cerr << "Error ao abrir arquivo de saída." << std::endl;
        return;
    }

    std::string line;
    int lineNumber = 0;
    int numVertices = 0, numEdges = 0, format = 0;

    // Leitura do header
    while (std::getline(infile, line)) {
        if (line[0] == '%') continue;  // pular comentários
        std::istringstream iss(line);
        iss >> numVertices >> numEdges >> format;
        break;
    }

    // Leitura do restante do grafo
    int nodeIndex = 1;
    while (std::getline(infile, line)) {
        if (line[0] == '%') continue;  // pular comentários
        std::istringstream iss(line);
        int neighbor;
        while (iss >> neighbor) {
            // Escrever as arestas no arquivo de saída
            if (nodeIndex < neighbor) {  // Evitar arestas duplicadas
                outfile << nodeIndex << " " << neighbor << "\n";
            }
        }
        nodeIndex++;
    }

    infile.close();
    outfile.close();
    std::cout << "Conversão completa" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <input_file> <output_file>" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];

    metisToEdgeList(inputFile, outputFile);
    return 0;
}