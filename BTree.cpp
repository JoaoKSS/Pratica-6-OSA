#include "BTree.h"
#include <fstream>
#include <queue>
#include <map>
#include <iomanip>  

using namespace std;

BTreeNode::BTreeNode(int m, bool leaf, BTreeNode* parent)
    : m(m), leaf(leaf), parent(parent) { }

pair<BTreeNode*, int> BTreeNode::search(int key) {
    int i = 0;
    while (i < static_cast<int>(keys.size()) && key > keys[i].first)
        i++;
    if (i < static_cast<int>(keys.size()) && keys[i].first == key)
        return {this, i};
    if (leaf)
        return {nullptr, -1};
    return children[i]->search(key);
}

int BTreeNode::find_key_index(int key) {
    int idx = 0;
    while (idx < static_cast<int>(keys.size()) && keys[idx].first < key)
        idx++;
    return idx;
}

BTreeNode* BTreeNode::insert_nonfull(int key, const string& address) {
    if (leaf) {
        int i = keys.size() - 1;
        keys.push_back({0, ""}); // aloca espaço para a nova chave
        while (i >= 0 && key < keys[i].first) {
            keys[i+1] = keys[i];
            i--;
        }
        keys[i+1] = {key, address};
        if (static_cast<int>(keys.size()) == 2 * m + 1) {
            if (parent != nullptr) {
                int idx = 0;
                for (int j = 0; j < static_cast<int>(parent->children.size()); j++) {
                    if (parent->children[j] == this) {
                        idx = j;
                        break;
                    }
                }
                parent->split_child(idx);
            } else {
                // Divisão da raiz
                BTreeNode* new_root = new BTreeNode(m, false);
                new_root->children.push_back(this);
                parent = new_root;
                new_root->split_child(0);
                return new_root;
            }
        }
        return nullptr;
    } else {
        int i = keys.size() - 1;
        while (i >= 0 && key < keys[i].first)
            i--;
        i++;
        if (static_cast<int>(children[i]->keys.size()) == 2 * m + 1) {
            split_child(i);
            if (key > keys[i].first)
                i++;
        }
        BTreeNode* ret = children[i]->insert_nonfull(key, address);
        if (ret != nullptr)
            return ret;
        if (static_cast<int>(keys.size()) == 2 * m + 1) {
            if (parent != nullptr) {
                int idx = 0;
                for (int j = 0; j < static_cast<int>(parent->children.size()); j++) {
                    if (parent->children[j] == this) {
                        idx = j;
                        break;
                    }
                }
                parent->split_child(idx);
            } else {
                BTreeNode* new_root = new BTreeNode(m, false);
                new_root->children.push_back(this);
                parent = new_root;
                new_root->split_child(0);
                return new_root;
            }
        }
        return nullptr;
    }
}

void BTreeNode::split_child(int i) {
    BTreeNode* y = children[i];                      // nó a ser dividido
    BTreeNode* z = new BTreeNode(m, y->leaf, this);    // novo nó que receberá as chaves finais
    KeyValue mid_key = y->keys[m];                   // chave mediana a ser promovida

    // z recebe as chaves de índice m+1 até o fim
    z->keys.assign(y->keys.begin() + m + 1, y->keys.end());
    // Se não for folha, transfere também os filhos
    if (!y->leaf) {
        z->children.assign(y->children.begin() + m + 1, y->children.end());
        for (auto child : z->children)
            child->parent = z;
        y->children.resize(m + 1);
    }
    // y mantém as chaves de índice 0 até m-1
    y->keys.resize(m);

    // Insere z como filho deste nó, logo após y, e promove a chave mediana
    children.insert(children.begin() + i + 1, z);
    keys.insert(keys.begin() + i, mid_key);
}

void BTreeNode::remove(int key) {
    int idx = find_key_index(key);
    if (idx < static_cast<int>(keys.size()) && keys[idx].first == key) {
        if (leaf) {
            remove_from_leaf(idx);
            if (parent != nullptr && static_cast<int>(keys.size()) < m) {
                int idx_p = 0;
                for (int j = 0; j < static_cast<int>(parent->children.size()); j++) {
                    if (parent->children[j] == this) {
                        idx_p = j;
                        break;
                    }
                }
                parent->fill(idx_p);
                parent->fix_deficiency_upwards();
            }
        } else {
            remove_from_nonleaf(idx);
        }
    } else {
        if (leaf)
            return; // chave não encontrada
        if (children[idx]->search(key).first == nullptr && static_cast<int>(children[idx]->keys.size()) == m)
            fill(idx);
        children[idx]->remove(key);
    }
}

void BTreeNode::remove_from_leaf(int idx) {
    keys.erase(keys.begin() + idx);
}

void BTreeNode::remove_from_nonleaf(int idx) {
    int key_val = keys[idx].first;
    if (static_cast<int>(children[idx]->keys.size()) >= m) {
        KeyValue pred = get_predecessor(idx);
        keys[idx] = pred;
        children[idx]->remove(pred.first);
    } else if (static_cast<int>(children[idx+1]->keys.size()) >= m) {
        KeyValue succ = get_successor(idx);
        keys[idx] = succ;
        children[idx+1]->remove(succ.first);
    } else {
        merge(idx);
        children[idx]->remove(key_val);
    }
}

KeyValue BTreeNode::get_predecessor(int idx) {
    BTreeNode* cur = children[idx];
    while (!cur->leaf)
        cur = cur->children.back();
    return cur->keys.back();
}

KeyValue BTreeNode::get_successor(int idx) {
    BTreeNode* cur = children[idx+1];
    while (!cur->leaf)
        cur = cur->children.front();
    return cur->keys.front();
}

void BTreeNode::fill(int idx) {
    if (idx != 0 && static_cast<int>(children[idx-1]->keys.size()) > m)
        borrow_from_prev(idx);
    else if (idx != static_cast<int>(keys.size()) && static_cast<int>(children[idx+1]->keys.size()) > m)
        borrow_from_next(idx);
    else {
        if (idx != 0)
            merge(idx-1);
        else
            merge(idx);
    }
}

void BTreeNode::borrow_from_prev(int idx) {
    BTreeNode* child = children[idx];
    BTreeNode* sibling = children[idx-1];
    child->keys.insert(child->keys.begin(), keys[idx-1]);
    if (!child->leaf) {
        child->children.insert(child->children.begin(), sibling->children.back());
        child->children.front()->parent = child;
        sibling->children.pop_back();
    }
    keys[idx-1] = sibling->keys.back();
    sibling->keys.pop_back();
}

void BTreeNode::borrow_from_next(int idx) {
    BTreeNode* child = children[idx];
    BTreeNode* sibling = children[idx+1];
    child->keys.push_back(keys[idx]);
    if (!child->leaf) {
        child->children.push_back(sibling->children.front());
        child->children.back()->parent = child;
        sibling->children.erase(sibling->children.begin());
    }
    keys[idx] = sibling->keys.front();
    sibling->keys.erase(sibling->keys.begin());
}

void BTreeNode::merge(int idx) {
    BTreeNode* child = children[idx];
    BTreeNode* sibling = children[idx+1];
    child->keys.push_back(keys[idx]);
    child->keys.insert(child->keys.end(), sibling->keys.begin(), sibling->keys.end());
    if (!child->leaf) {
        child->children.insert(child->children.end(), sibling->children.begin(), sibling->children.end());
        for (auto c : sibling->children)
            c->parent = child;
    }
    keys.erase(keys.begin() + idx);
    children.erase(children.begin() + idx + 1);
    delete sibling;
}

void BTreeNode::rotate_internal_left() {
    if (children.size() < 2 || children[0]->keys.empty())
        return;
    BTreeNode* left_child = children[0];
    BTreeNode* right_child = children[1];
    KeyValue temp = left_child->keys.back();
    left_child->keys.pop_back();
    KeyValue old_key = keys[0];
    keys[0] = temp;
    right_child->keys.insert(right_child->keys.begin(), old_key);
    if (!left_child->leaf && !left_child->children.empty()) {
        BTreeNode* child_temp = left_child->children.back();
        left_child->children.pop_back();
        right_child->children.insert(right_child->children.begin(), child_temp);
        child_temp->parent = right_child;
    }
}

void BTreeNode::fix_deficiency_upwards() {
    BTreeNode* current = this;
    while (current->parent != nullptr && static_cast<int>(current->keys.size()) < current->m) {
        BTreeNode* parent_node = current->parent;
        int idx = 0;
        for (int j = 0; j < static_cast<int>(parent_node->children.size()); j++) {
            if (parent_node->children[j] == current) {
                idx = j;
                break;
            }
        }
        parent_node->fill(idx);
        current = parent_node;
    }
}

void BTreeNode::print_node(int level) {
    string indent(level * 4, ' ');
    cout << indent << "[";
    for (size_t i = 0; i < keys.size(); i++) {
        cout << keys[i].first;
        if (i < keys.size() - 1)
            cout << ", ";
    }
    cout << "]\n";
    if (!leaf) {
        for (auto child : children)
            child->print_node(level + 1);
    }
}

BTree::BTree(int ordem) : m(ordem - 1) {
    root = new BTreeNode(m, true);
}

pair<BTreeNode*, int> BTree::search(int key) {
    if (root)
        return root->search(key);
    return {nullptr, -1};
}

void BTree::insert(int key, const string& address) {
    if (static_cast<int>(root->keys.size()) == 2 * m + 1) {
        BTreeNode* s = new BTreeNode(m, false);
        s->children.push_back(root);
        root->parent = s;
        s->split_child(0);
        root = s;
    }
    BTreeNode* new_root = root->insert_nonfull(key, address);
    if (new_root != nullptr)
        root = new_root;
}

void BTree::remove(int key) {
    if (!root) {
        cout << "Árvore vazia.\n";
        return;
    }
    root->remove(key);
    if (root && root->keys.empty()) {
        if (!root->leaf) {
            BTreeNode* tmp = root->children[0];
            tmp->parent = nullptr;
            delete root;
            root = tmp;
        } else {
            delete root;
            root = nullptr;
        }
    }
}

void BTree::print_tree() {
    if (root)
        root->print_node();
    else
        cout << "A Árvore B está vazia.\n";
}


void BTree::saveToFile(const string& filename) {
    ofstream out(filename);
    if (!out.is_open()) {
        cerr << "Erro ao abrir arquivo para escrita: " << filename << endl;
        return;
    }
    // Se a árvore estiver vazia, registra no arquivo 
    if (!root) {
        out << "A Árvore B vazia" << endl;
        out.close();
        return;
    }

    // Percorrer a árvore em largura (BFS) para coletar os nós na ordem que serão impressos.
    queue<BTreeNode*> fila;
    vector<BTreeNode*> allNodes; // guardará os nós em ordem BFS

    fila.push(root);
    while (!fila.empty()) {
        BTreeNode* node = fila.front();
        fila.pop();
        allNodes.push_back(node);

        // Enfileira filhos
        for (auto child : node->children) {
            if (child) {
                fila.push(child);
            }
        }
    }

    // Descobrir o número máximo de chaves em qualquer nó para sabermos quantas colunas "key_i" e "end_i" imprimir
    int maxKeys = 0;
    for (auto node : allNodes) {
        if (static_cast<int>(node->keys.size()) > maxKeys) {
            maxKeys = node->keys.size();
        }
    }

    // Imprimir cabeçalho 
    out << "End. (nó)      | Tam Ch | End. (pai)     ";
    for (int i = 0; i < maxKeys; i++) {
        out << "| End. (filho_" << i << ") | Chave_" << i << " ";
    }
    // Último filho (caso haja maxKeys+1 filhos)
    out << "| End. (filho " << maxKeys << ") | Fag_Folha \n";

    // Linha divisória 
    // out << string(177, '-') << "\n";

    // Imprimir cada nó em ordem BFS
    for (BTreeNode* node : allNodes) {
        // 1) Endereço do próprio nó (imprime o ponteiro diretamente)
        out << setw(10) << left << static_cast<const void*>(node) << " ";

        // Número de chaves
        int numChaves = node->keys.size();
        out << "| " << setw(6) << numChaves << " ";

        // Endereço do pai (ou -1 se não existir)
        if (node->parent) {
            out << "| " << setw(10) << left << static_cast<const void*>(node->parent) << " ";
        } else {
            out << "| " << setw(14) << left << "-1" << " ";
        }

        // Para cada coluna até maxKeys: filho e chave
        for (int k = 0; k < maxKeys; k++) {
            // Imprime o endereço do filho k, se existir; caso contrário, imprime -1
            if (k < static_cast<int>(node->children.size()) && node->children[k]) {
                out << "| " << setw(10) << left << static_cast<const void*>(node->children[k]) << " ";
            } else {
                out << "| " << setw(14) << left << "-1" << " ";
            }

            // Imprime a chave k, se existir; caso contrário, imprime -1
            if (k < numChaves) {
                out << "| " << setw(7) << node->keys[k].first << " ";
            } else {
                out << "| " << setw(7) << "-1" << " ";
            }
        }

        // Imprime o último filho (caso exista)
        if (static_cast<int>(node->children.size()) > maxKeys && node->children[maxKeys]) {
            out << "| " << setw(10) << left << static_cast<const void*>(node->children[maxKeys]) << " ";
        } else {
            out << "| " << setw(14) << left << "-1" << " ";
        }

        // Coluna: Indicação se é folha
        out << "| " << (node->leaf ? 1 : 0) << "\n";
    }
    out.close();
}






// #include "BTree.h"
// #include <fstream>
// #include <queue>
// #include <map>
// #include <unordered_map>
// #include <iomanip>
// #include <cstdint>

// using namespace std;

// // Construtor do BTreeNode
// BTreeNode::BTreeNode(int m, bool leaf, BTreeNode* parent)
//     : m(m), leaf(leaf), parent(parent) { }

// // Métodos básicos da BTreeNode
// pair<BTreeNode*, int> BTreeNode::search(int key) {
//     int i = 0;
//     while (i < static_cast<int>(keys.size()) && key > keys[i].first)
//         i++;
//     if (i < static_cast<int>(keys.size()) && keys[i].first == key)
//         return {this, i};
//     if (leaf)
//         return {nullptr, -1};
//     return children[i]->search(key);
// }

// int BTreeNode::find_key_index(int key) {
//     int idx = 0;
//     while (idx < static_cast<int>(keys.size()) && keys[idx].first < key)
//         idx++;
//     return idx;
// }

// // Exemplo de método insert_nonfull (demais métodos de inserção/remoção permanecem inalterados)
// BTreeNode* BTreeNode::insert_nonfull(int key, const string& address) {
//     if (leaf) {
//         int i = keys.size() - 1;
//         keys.push_back({0, ""});
//         while (i >= 0 && key < keys[i].first) {
//             keys[i+1] = keys[i];
//             i--;
//         }
//         keys[i+1] = {key, address};
//         if (static_cast<int>(keys.size()) == 2 * m + 1) {
//             if (parent != nullptr) {
//                 int idx = 0;
//                 for (int j = 0; j < static_cast<int>(parent->children.size()); j++) {
//                     if (parent->children[j] == this) {
//                         idx = j;
//                         break;
//                     }
//                 }
//                 parent->split_child(idx);
//             } else {
//                 // Divisão da raiz
//                 BTreeNode* new_root = new BTreeNode(m, false);
//                 new_root->children.push_back(this);
//                 parent = new_root;
//                 new_root->split_child(0);
//                 return new_root;
//             }
//         }
//         return nullptr;
//     } else {
//         int i = keys.size() - 1;
//         while (i >= 0 && key < keys[i].first)
//             i--;
//         i++;
//         if (static_cast<int>(children[i]->keys.size()) == 2 * m + 1) {
//             split_child(i);
//             if (key > keys[i].first)
//                 i++;
//         }
//         BTreeNode* ret = children[i]->insert_nonfull(key, address);
//         if (ret != nullptr)
//             return ret;
//         if (static_cast<int>(keys.size()) == 2 * m + 1) {
//             if (parent != nullptr) {
//                 int idx = 0;
//                 for (int j = 0; j < static_cast<int>(parent->children.size()); j++) {
//                     if (parent->children[j] == this) {
//                         idx = j;
//                         break;
//                     }
//                 }
//                 parent->split_child(idx);
//             } else {
//                 BTreeNode* new_root = new BTreeNode(m, false);
//                 new_root->children.push_back(this);
//                 parent = new_root;
//                 new_root->split_child(0);
//                 return new_root;
//             }
//         }
//         return nullptr;
//     }
// }

// void BTreeNode::split_child(int i) {
//     BTreeNode* y = children[i];
//     BTreeNode* z = new BTreeNode(m, y->leaf, this);
//     KeyValue mid_key = y->keys[m];

//     // z recebe as chaves de índice m+1 até o fim
//     z->keys.assign(y->keys.begin() + m + 1, y->keys.end());
//     if (!y->leaf) {
//         z->children.assign(y->children.begin() + m + 1, y->children.end());
//         for (auto child : z->children)
//             child->parent = z;
//         y->children.resize(m + 1);
//     }
//     y->keys.resize(m);

//     children.insert(children.begin() + i + 1, z);
//     keys.insert(keys.begin() + i, mid_key);
// }

// // Métodos de remoção (mantidos conforme versão antiga)
// void BTreeNode::remove(int key) {
//     int idx = find_key_index(key);
//     if (idx < static_cast<int>(keys.size()) && keys[idx].first == key) {
//         if (leaf) {
//             remove_from_leaf(idx);
//             if (parent != nullptr && static_cast<int>(keys.size()) < m) {
//                 int idx_p = 0;
//                 for (int j = 0; j < static_cast<int>(parent->children.size()); j++) {
//                     if (parent->children[j] == this) {
//                         idx_p = j;
//                         break;
//                     }
//                 }
//                 parent->fill(idx_p);
//                 parent->fix_deficiency_upwards();
//             }
//         } else {
//             remove_from_nonleaf(idx);
//         }
//     } else {
//         if (leaf)
//             return;
//         if (children[idx]->search(key).first == nullptr && static_cast<int>(children[idx]->keys.size()) == m)
//             fill(idx);
//         children[idx]->remove(key);
//     }
// }

// void BTreeNode::remove_from_leaf(int idx) {
//     keys.erase(keys.begin() + idx);
// }

// void BTreeNode::remove_from_nonleaf(int idx) {
//     int key_val = keys[idx].first;
//     if (static_cast<int>(children[idx]->keys.size()) >= m) {
//         KeyValue pred = get_predecessor(idx);
//         keys[idx] = pred;
//         children[idx]->remove(pred.first);
//     } else if (static_cast<int>(children[idx+1]->keys.size()) >= m) {
//         KeyValue succ = get_successor(idx);
//         keys[idx] = succ;
//         children[idx+1]->remove(succ.first);
//     } else {
//         merge(idx);
//         children[idx]->remove(key_val);
//     }
// }

// KeyValue BTreeNode::get_predecessor(int idx) {
//     BTreeNode* cur = children[idx];
//     while (!cur->leaf)
//         cur = cur->children.back();
//     return cur->keys.back();
// }

// KeyValue BTreeNode::get_successor(int idx) {
//     BTreeNode* cur = children[idx+1];
//     while (!cur->leaf)
//         cur = cur->children.front();
//     return cur->keys.front();
// }

// void BTreeNode::fill(int idx) {
//     if (idx != 0 && static_cast<int>(children[idx-1]->keys.size()) > m)
//         borrow_from_prev(idx);
//     else if (idx != static_cast<int>(keys.size()) && static_cast<int>(children[idx+1]->keys.size()) > m)
//         borrow_from_next(idx);
//     else {
//         if (idx != 0)
//             merge(idx-1);
//         else
//             merge(idx);
//     }
// }

// void BTreeNode::borrow_from_prev(int idx) {
//     BTreeNode* child = children[idx];
//     BTreeNode* sibling = children[idx-1];
//     child->keys.insert(child->keys.begin(), keys[idx-1]);
//     if (!child->leaf) {
//         child->children.insert(child->children.begin(), sibling->children.back());
//         child->children.front()->parent = child;
//         sibling->children.pop_back();
//     }
//     keys[idx-1] = sibling->keys.back();
//     sibling->keys.pop_back();
// }

// void BTreeNode::borrow_from_next(int idx) {
//     BTreeNode* child = children[idx];
//     BTreeNode* sibling = children[idx+1];
//     child->keys.push_back(keys[idx]);
//     if (!child->leaf) {
//         child->children.push_back(sibling->children.front());
//         child->children.back()->parent = child;
//         sibling->children.erase(sibling->children.begin());
//     }
//     keys[idx] = sibling->keys.front();
//     sibling->keys.erase(sibling->keys.begin());
// }

// void BTreeNode::merge(int idx) {
//     BTreeNode* child = children[idx];
//     BTreeNode* sibling = children[idx+1];
//     child->keys.push_back(keys[idx]);
//     child->keys.insert(child->keys.end(), sibling->keys.begin(), sibling->keys.end());
//     if (!child->leaf) {
//         child->children.insert(child->children.end(), sibling->children.begin(), sibling->children.end());
//         for (auto c : sibling->children)
//             c->parent = child;
//     }
//     keys.erase(keys.begin() + idx);
//     children.erase(children.begin() + idx + 1);
//     delete sibling;
// }

// void BTreeNode::rotate_internal_left() {
//     if (children.size() < 2 || children[0]->keys.empty())
//         return;
//     BTreeNode* left_child = children[0];
//     BTreeNode* right_child = children[1];
//     KeyValue temp = left_child->keys.back();
//     left_child->keys.pop_back();
//     KeyValue old_key = keys[0];
//     keys[0] = temp;
//     right_child->keys.insert(right_child->keys.begin(), old_key);
//     if (!left_child->leaf && !left_child->children.empty()) {
//         BTreeNode* child_temp = left_child->children.back();
//         left_child->children.pop_back();
//         right_child->children.insert(right_child->children.begin(), child_temp);
//         child_temp->parent = right_child;
//     }
// }

// void BTreeNode::fix_deficiency_upwards() {
//     BTreeNode* current = this;
//     while (current->parent != nullptr && static_cast<int>(current->keys.size()) < current->m) {
//         BTreeNode* parent_node = current->parent;
//         int idx = 0;
//         for (int j = 0; j < static_cast<int>(parent_node->children.size()); j++) {
//             if (parent_node->children[j] == current) {
//                 idx = j;
//                 break;
//             }
//         }
//         parent_node->fill(idx);
//         current = parent_node;
//     }
// }

// void BTreeNode::print_node(int level) {
//     string indent(level * 4, ' ');
//     cout << indent << "[";
//     for (size_t i = 0; i < keys.size(); i++) {
//         cout << keys[i].first;
//         if (i < keys.size() - 1)
//             cout << ", ";
//     }
//     cout << "]\n";
//     if (!leaf) {
//         for (auto child : children)
//             if(child)
//                 child->print_node(level + 1);
//     }
// }

// // Métodos de serialização e desserialização
// void BTreeNode::serialize(ofstream &out, unordered_map<BTreeNode*, int64_t> &nodeOffsets,
//                           unordered_map<BTreeNode*, ChildOffsetInfo> &childOffsets)
// {
//     // Registra o offset atual deste nó
//     int64_t myOffset = out.tellp();
//     nodeOffsets[this] = myOffset;
    
//     // Grava a flag de folha (char: 1 para true, 0 para false)
//     char isLeaf = leaf ? 1 : 0;
//     out.write(&isLeaf, sizeof(isLeaf));

//     // Grava o número de chaves (int32_t)
//     int32_t numKeys = static_cast<int32_t>(keys.size());
//     out.write(reinterpret_cast<const char*>(&numKeys), sizeof(numKeys));
    
//     // Para cada chave, grava o valor e depois o tamanho e conteúdo do endereço
//     for (const auto &kv : keys) {
//         int32_t key = kv.first;
//         out.write(reinterpret_cast<const char*>(&key), sizeof(key));
//         int32_t addrLen = static_cast<int32_t>(kv.second.size());
//         out.write(reinterpret_cast<const char*>(&addrLen), sizeof(addrLen));
//         out.write(kv.second.c_str(), addrLen);
//     }
    
//     // Grava o número de filhos (int32_t)
//     int32_t numChildren = static_cast<int32_t>(children.size());
//     out.write(reinterpret_cast<const char*>(&numChildren), sizeof(numChildren));
    
//     // Para cada filho, grava um placeholder (int64_t com -1) e registra a posição no arquivo
//     ChildOffsetInfo info;
//     for (int i = 0; i < numChildren; i++) {
//         int64_t placeholder = -1;
//         info.positions.push_back(out.tellp());
//         out.write(reinterpret_cast<const char*>(&placeholder), sizeof(placeholder));
//     }
//     childOffsets[this] = info;
// }

// BTreeNode* BTreeNode::deserialize(ifstream &in, int m_val,
//                                   unordered_map<int64_t, vector<int64_t>> &nodesChildOffsets, int64_t nodeOffset)
// {
//     in.seekg(nodeOffset);
    
//     // Lê a flag de folha
//     char isLeaf;
//     in.read(&isLeaf, sizeof(isLeaf));
//     bool leaf = (isLeaf == 1);
    
//     // Cria o nó (o pai será definido posteriormente)
//     BTreeNode* node = new BTreeNode(m_val, leaf);
    
//     // Lê o número de chaves
//     int32_t numKeys;
//     in.read(reinterpret_cast<char*>(&numKeys), sizeof(numKeys));
    
//     // Lê cada chave e seu endereço
//     for (int i = 0; i < numKeys; i++) {
//         int32_t key;
//         in.read(reinterpret_cast<char*>(&key), sizeof(key));
//         int32_t addrLen;
//         in.read(reinterpret_cast<char*>(&addrLen), sizeof(addrLen));
//         string address(addrLen, ' ');
//         in.read(&address[0], addrLen);
//         node->keys.push_back({key, address});
//     }
    
//     // Lê o número de filhos
//     int32_t numChildren;
//     in.read(reinterpret_cast<char*>(&numChildren), sizeof(numChildren));
    
//     vector<int64_t> childOffsets;
//     for (int i = 0; i < numChildren; i++) {
//         int64_t childOff;
//         in.read(reinterpret_cast<char*>(&childOff), sizeof(childOff));
//         childOffsets.push_back(childOff);
//         // Inicializa o ponteiro do filho como nullptr; a atribuição ocorrerá após a leitura completa
//         node->children.push_back(nullptr);
//     }
//     nodesChildOffsets[nodeOffset] = childOffsets;
    
//     return node;
// }

// // Métodos de salvamento e carregamento da árvore
// void BTree::saveToFile(const string &filename) {
//     // Abre o arquivo em modo binário para escrita
//     ofstream out(filename, ios::binary);
//     if (!out.is_open()) {
//         cerr << "Erro ao abrir arquivo para escrita: " << filename << endl;
//         return;
//     }

//     // Cabeçalho: placeholder para o offset da raiz e o valor de m
//     int64_t rootOffsetPlaceholder = -1;
//     out.write(reinterpret_cast<const char*>(&rootOffsetPlaceholder), sizeof(rootOffsetPlaceholder));
//     int32_t m_val = m;
//     out.write(reinterpret_cast<const char*>(&m_val), sizeof(m_val));

//     // Mapas para manter o offset de cada nó e as posições onde serão atualizados os offsets dos filhos
//     unordered_map<BTreeNode*, int64_t> nodeOffsets;
//     unordered_map<BTreeNode*, ChildOffsetInfo> childOffsets;

//     // Serializa todos os nós utilizando BFS
//     queue<BTreeNode*> fila;
//     vector<BTreeNode*> todosNos;
//     fila.push(root);
//     while (!fila.empty()) {
//         BTreeNode* node = fila.front();
//         fila.pop();
//         todosNos.push_back(node);
//         node->serialize(out, nodeOffsets, childOffsets);

//         for (auto child : node->children) {
//             if (child)
//                 fila.push(child);
//         }
//     }

//     out.flush();
//     out.close();

//     // Reabre o arquivo em modo read/write para atualizar os offsets dos filhos
//     fstream outRW(filename, ios::binary | ios::in | ios::out);
//     if (!outRW.is_open()) {
//         cerr << "Erro ao reabrir arquivo para atualização: " << filename << endl;
//         return;
//     }

//     // Atualiza os placeholders dos offsets dos filhos para cada nó serializado
//     for (auto node : todosNos) {
//         ChildOffsetInfo info = childOffsets[node];
//         for (size_t i = 0; i < node->children.size(); i++) {
//             int64_t childOffset = -1;
//             if (node->children[i] && nodeOffsets.find(node->children[i]) != nodeOffsets.end())
//                 childOffset = nodeOffsets[node->children[i]];
//             int64_t pos = info.positions[i];
//             outRW.seekp(pos);
//             outRW.write(reinterpret_cast<const char*>(&childOffset), sizeof(childOffset));
//         }
//     }
    
//     // Atualiza o cabeçalho com o offset real da raiz
//     if (root && nodeOffsets.find(root) != nodeOffsets.end()) {
//         int64_t rootOffset = nodeOffsets[root];
//         outRW.seekp(0);
//         outRW.write(reinterpret_cast<const char*>(&rootOffset), sizeof(rootOffset));
//     }

//     outRW.close();
// }

// void BTree::loadFromFile(const string &filename) {
//     ifstream in(filename, ios::binary);
//     if (!in.is_open()) {
//         cerr << "Erro ao abrir arquivo para leitura: " << filename << endl;
//         return;
//     }
    
//     // Lê o cabeçalho: offset da raiz e m
//     int64_t rootOffset;
//     in.read(reinterpret_cast<char*>(&rootOffset), sizeof(rootOffset));
//     int32_t m_val;
//     in.read(reinterpret_cast<char*>(&m_val), sizeof(m_val));
    
//     m = m_val;
//     unordered_map<int64_t, BTreeNode*> offsetToNode;
//     unordered_map<int64_t, vector<int64_t>> nodesChildOffsets;
    
//     queue<int64_t> filaOff;
//     filaOff.push(rootOffset);
    
//     while (!filaOff.empty()) {
//         int64_t off = filaOff.front();
//         filaOff.pop();
//         if (offsetToNode.find(off) != offsetToNode.end())
//             continue;
//         BTreeNode *node = BTreeNode::deserialize(in, m, nodesChildOffsets, off);
//         offsetToNode[off] = node;
//         vector<int64_t> childs = nodesChildOffsets[off];
//         for (auto childOff : childs) {
//             if (childOff != -1)
//                 filaOff.push(childOff);
//         }
//     }
    
//     // Reconstrói os ponteiros dos filhos e associa os pais
//     for (auto &pairOff : offsetToNode) {
//         int64_t off = pairOff.first;
//         BTreeNode *node = pairOff.second;
//         vector<int64_t> childOffs = nodesChildOffsets[off];
//         for (size_t i = 0; i < childOffs.size(); i++) {
//             int64_t childOff = childOffs[i];
//             if (childOff != -1 && offsetToNode.find(childOff) != offsetToNode.end()) {
//                 node->children[i] = offsetToNode[childOff];
//                 offsetToNode[childOff]->parent = node;
//             }
//         }
//     }
    
//     if (offsetToNode.find(rootOffset) != offsetToNode.end())
//         root = offsetToNode[rootOffset];
//     else
//         root = nullptr;
    
//     in.close();
// }

// // Construtor da BTree
// BTree::BTree(int ordem) : m(ordem - 1) {
//     root = new BTreeNode(m, true);
// }

// pair<BTreeNode*, int> BTree::search(int key) {
//     if (root)
//         return root->search(key);
//     return {nullptr, -1};
// }

// void BTree::insert(int key, const string &address) {
//     if (root == nullptr) {
//         root = new BTreeNode(m, true);
//     }
//     if (static_cast<int>(root->keys.size()) == 2 * m + 1) {
//         BTreeNode *s = new BTreeNode(m, false);
//         s->children.push_back(root);
//         root->parent = s;
//         s->split_child(0);
//         root = s;
//     }
//     BTreeNode *new_root = root->insert_nonfull(key, address);
//     if (new_root != nullptr)
//         root = new_root;
// }

// void BTree::remove(int key) {
//     if (!root) {
//         cout << "Árvore vazia.\n";
//         return;
//     }
//     root->remove(key);
//     if (root && root->keys.empty()) {
//         if (!root->leaf) {
//             BTreeNode *tmp = root->children[0];
//             tmp->parent = nullptr;
//             delete root;
//             root = tmp;
//         } else {
//             delete root;
//             root = nullptr;
//         }
//     }
// }

// void BTree::print_tree() {
//     if (root)
//         root->print_node();
//     else
//         cout << "A Árvore B está vazia.\n";
// }