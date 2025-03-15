<h2>Organização e Sistemas de Arquivos 2024/2</h2>

<h3>Sobre a Atividade</h3>
Este projeto implementa uma estrutura de dados Árvore B, incluindo operações de inserção, busca, remoção e serialização para arquivo.

### Funcionalidades

- **Inserção:** Insere chaves e respectivos endereços na Árvore B.
- **Busca:** Procura por uma determinada chave e retorna o endereço associado.
- **Remoção:** Remove chaves da Árvore B, reestruturando os nós se necessário.
- **Visualização:** Imprime a estrutura da árvore com indentação para facilitar a visualização.
- **Serialização:** Salva a estrutura da árvore em um arquivo texto.

### Organização do Código

- **BTree.h / BTree.cpp:** Declaração e implementação da estrutura da Árvore B e seus nós.
- **main.cpp:** Contém exemplos de uso, inserindo, buscando e removendo itens, e demonstra a impressão da árvore.
- **makefile:** Script para compilar e executar o projeto em um ambiente Windows ou Linux.

### Como Usar

1. **Configure os parâmetros:**  
   Altere a ordem da árvore ou os vetores de entrada para inserção, busca e remoção diretamente no arquivo `main.cpp`.

2. **Compilação e Execução:**  
   No terminal, execute:
   ```
   make run
   ```
   Esse comando compila o projeto e executa o programa.

3. **Visualização:**  
   A saída no terminal exibirá a estrutura da árvore após cada operação, facilitando o acompanhamento do funcionamento da Árvore B.

### Observações

- O código foi desenvolvido para ser compatível com C++17.
- Teste da Árvore B pode ser visualizado no site: [B-Tree Visualization](https://www.cs.usfca.edu/~galles/visualization/BTree.html).
