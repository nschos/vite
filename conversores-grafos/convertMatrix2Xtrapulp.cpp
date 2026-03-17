#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>

struct Edge {
    int32_t u, v;
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " input.mtx output.adj\n";
        return 1;
    }

    std::ifstream fin(argv[1]);
    if (!fin.is_open()) {
        std::cerr << "Erro ao abrir arquivo de entrada.\n";
        return 1;
    }

    std::string line;
    // Ignorar cabeçalho %%MatrixMarket
    std::getline(fin, line);

    // Ignorar linhas de comentário
    while (std::getline(fin, line)) {
        if (line[0] != '%') break;
    }

    std::istringstream iss(line);
    int nrows, ncols, nnz;
    iss >> nrows >> ncols >> nnz;

    std::vector<Edge> edges;
    edges.reserve(nnz * 2); // para simétrico

    int32_t u, v;
    double w; // ignorado

    while (fin >> u >> v) {
        // Converter para 0-based
        u -= 1;
        v -= 1;

        // ignorar pesos (se existir, apenas descartar)
        if (fin.peek() != '\n' && fin >> w) {
            // apenas descarta o peso
        }

        if (u == v) continue; // ignorar loops

        edges.push_back({u, v});
        edges.push_back({v, u}); // garantir não-direcionado
    }

    fin.close();

    // Descobre o número de vértices (max index + 1)
    int32_t nverts = 0;
    for (auto &e : edges) {
        nverts = std::max(nverts, std::max(e.u, e.v));
    }
    nverts += 1; // já que índices são 0-based

    int32_t nedges = edges.size();

    // Salva em binário
    std::ofstream fout(argv[2], std::ios::binary);
    if (!fout.is_open()) {
        std::cerr << "Erro ao abrir arquivo de saída.\n";
        return 1;
    }

    // fout.write(reinterpret_cast<char*>(&nverts), sizeof(int32_t));
    // fout.write(reinterpret_cast<char*>(&nedges), sizeof(int32_t));
    for (auto &e : edges) {
        fout.write(reinterpret_cast<char*>(&e.u), sizeof(int32_t));
        fout.write(reinterpret_cast<char*>(&e.v), sizeof(int32_t));
    }

    fout.close();
    std::cout << "Conversão concluída: " << nedges << " arestas gravadas (0-based).\n";

    return 0;
}