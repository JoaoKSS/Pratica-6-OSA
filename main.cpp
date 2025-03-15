#include "BTree.h"
#include <vector>
#include <string>
#include <iostream>

using namespace std;
// site usado para teste da árvore B: https://www.cs.usfca.edu/~galles/visualization/BTree.html

int main() {
// Define a ordem da Árvore B (exemplo: ordem = 2, 3 ou 4)
// usando a formula 2 * ordem - 1 = grau máximo da árvore
// usando a formula ordem - 1 = grau mínimo da árvore
/*------------------------------------------------------------------------------------------*/
    // BTree btree(2); /* Equivale a grau máximo 3 no programa de simulação "B-Trees"*/
    BTree btree(3); /* Equivale a grau máximo 5 no programa de simulação "B-Trees"*/
    // BTree btree(4); /*Equivale a grau máximo 7 no programa de simulação "B-Trees"*/
/*-----------------------------------------------------------------------------------------*/

    // Vetor de elementos a serem inseridos na árvore
    vector<int> elementos = {15, 3, 8, 23, 1, 9, 14, 18, 10, 20, 5, 6, 12, 30, 2, 4, 7, 11, 13, 16, 19, 21, 26, 28, 27, 29, 17, 22, 25, 24, 31, 35, 33, 36, 32, 34, 38, 37};
    // Cabeçalho para a inserção
     cout << "\n*************************************************\n";
     cout << "*        INICIANDO A INSERÇÃO DA ÁRVORE         *\n";
     cout << "*************************************************\n" << endl;

    for (int chave : elementos) {
        cout << "Inserindo " << chave << "..." << endl;
        btree.insert(chave, "End_" + to_string(chave));
        btree.print_tree();
        cout << "----------------------------------------" << endl;
    }

    // Vetor de chaves para busca
    vector<int> buscas = {1, 14, 18, 31, 40};
    // Cabeçalho para a busca
    cout << "\n*************************************************\n";
    cout << "*          INICIANDO A BUSCA DA ÁRVORE          *\n";
    cout << "*************************************************\n" << endl;
    for (int chave : buscas) {
        auto resultado = btree.search(chave);
        if(resultado.first != nullptr) {
            // cout << "Chave " << chave << " encontrada com endereço: " 
            //      << resultado.first->keys[resultado.second].second << endl; 
            cout << "Chave " << chave << " encontrada com sucesso!" << endl;
        } else {
            cout << "Chave " << chave << " não encontrada na árvore!" << endl;
        }
        cout << "----------------------------------------" << endl;
    }

    // Salva a árvore em arquivo
    btree.saveToFile("arvore_b.txt");

    // Vetor de elementos a serem removidos da árvore
    vector<int> elementos2 = {15, 3, 8, 23, 1, 9, 14, 18, 10, 20, 5, 6, 12, 30, 2, 4, 7, 11, 13, 16, 19, 21, 26, 28, 27, 29, 17, 22, 25, 24, 31, 35, 33, 36, 32, 34, 38, 37};
     // Cabeçalho para a remoção
     cout << "\n*************************************************\n";
     cout << "*         INICIANDO A REMOÇÃO DA ÁRVORE         *\n";
     cout << "*************************************************\n" << endl;
    // Removendo na mesma ordem das inserções
    for (int chave : elementos2) {
        cout << "Removendo chave " << chave << "..." << endl;
        btree.remove(chave);
        btree.print_tree();
        cout << "----------------------------------------" << endl;
    }

    return 0;
}
