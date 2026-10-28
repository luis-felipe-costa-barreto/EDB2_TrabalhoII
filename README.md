# Arvores Balanceadas (AVL e Rubro-Negra)

Este projeto consiste em uma implementação em C++ das estruturas de dados **Árvore AVL** e **Árvore Rubro-Negra**, permitindo a execução de operações básicas e um benchmark comparativo de desempenho entre ambas.

## Funcionalidades

- **Interação Individual**:
  - Inserção, remoção e busca de chaves.
  - Exibição do percurso em ordem.
  - Impressão visual da estrutura da árvore no terminal.
  - Validação das propriedades de balanceamento.
  - Relatório detalhado com tempo de execução (nanossegundos), consumo de memória, quantidade de rotações e recolorações.
- **Modo Comparativo (Benchmark)**:
  - Teste automático com $N$ elementos (entre 2 e 2.000.000).
  - Execução com conjuntos de dados crescentes, decrescentes e aleatórios.
  - Medição de tempo, altura da árvore e uso de memória para inserções, buscas e remoções.

## Requisitos

- Compilador C++ com suporte ao padrão C++11 ou superior (ex.: `g++`, `clang++`).

## Compilação e Execução

Para compilar o código utilizando o `g++`:

```bash
g++ arvores.cpp -o arvores
```

Para executar o programa:

```bash
./arvores
```

## Estrutura do Código

- `Arvore`: Classe base abstrata que define a interface das árvores e cuida da medição de tempo e memória.
- `AVL`: Implementação da Árvore AVL, utilizando fator de balanceamento baseado em altura.
- `RN`: Implementação da Árvore Rubro-Negra, utilizando ponteiros de pai, no nulo compartilhado (`nil`) e balanceamento por cor/rotação.
- `desenhar`: Função genérica para renderizar a estrutura da árvore em texto no terminal.
- `comparar`: Módulo responsável pela comparação automatizada de desempenho entre as duas estruturas.
