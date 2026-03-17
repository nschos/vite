#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <set>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " input.mtx output.graph [-w]\n";
        return 1;
    }

    std::ifstream infile(argv[1]);
    std::ofstream outfile(argv[2]);
    bool weightFlag = false;
    if (argc == 4) {
       weightFlag = true;
    }

    if (!infile) {
        std::cerr << "Erro ao abrir arquivo.\n";
        return 1;
    }

    std::string line;
    // Pular comentários e o header do MatrixMarket 
    while (std::getline(infile, line)) {
        if (line[0] != '%') break;
    }

    // Informações do grafo
    int nrows, ncols, nedges;
    std::istringstream iss(line);
    iss >> nrows >> ncols >> nedges;

    // Lista de adjacencia (índice 1-based)
    std::vector<std::set<int>> adj(nrows + 1); // set para evitar arestas duplicadas

    int u, v, w;
    if (weightFlag) {
        while (infile >> u >> v >> w) {
            if (u != v) { // pular arestas em loop
                adj[u].insert(v);
                adj[v].insert(u);
            }
        }
    } else {
        while (infile >> u >> v) {
            if (u != v) { // pular arestas em loop
                adj[u].insert(v);
                adj[v].insert(u);
            }
        }
    }


    // Escrever o arquivo de saída
    int actual_edges = 0;
    for (int i = 1; i <= nrows; ++i)
        actual_edges += adj[i].size();

    outfile << nrows << " " << actual_edges / 2 << "\n";

    for (int i = 1; i <= nrows; ++i) {
        for (int neighbor : adj[i]) {
            outfile << neighbor << " ";
        }
        outfile << "\n";
    }

    std::cout << "Conversão completa. Output escrito em " << argv[2] << "\n";
    return 0;
}