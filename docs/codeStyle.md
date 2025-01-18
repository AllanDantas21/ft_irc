# Introdução
Este guia de estilo de código foi criado para padronizar a escrita de código C++ dentro do nosso projeto. Seguir estas diretrizes ajudará a garantir que o código seja consistente, legível e fácil de manter.

## Nomenclatura

### Classes
Use CamelCase para nomes de classes.  
Exemplos: `MyClass`, `EmployeeData`.

### Funções
Use camelCase para nomes de funções. ( Primeira letra minuscula)
Exemplos: `calculateTotal()`, `getUserName()`.

### Variáveis
Use camelCase para nomes de variáveis.  
Exemplos: `totalAmount`, `userName`.

### Constantes
Use UPPER_CASE com underscores para constantes.  
Exemplos: `MAX_COUNT`, `DEFAULT_TIMEOUT`.

## Estrutura de Arquivos
Reforçando que usamos a arquitetura de espelho (Mirror Arquiteture),
Para cada arquivo .CPP em /srcs, devera ter o respectivo .HPP em /incs.
 
Cada classe deve ser definida em um arquivo `.hpp` e implementada em um arquivo `.cpp`.  
Use o nome da classe para nomear os arquivos.  
Exemplo: `MyClass.hpp`, `MyClass.cpp`.

## Formato de Código

### Indentação
Use 4 espaços para indentação.

### Chaves
As chaves de abertura `{` devem estar na mesma linha da declaração. (com um espaço na abertura)
A chave de fechamento `}` deve estar alinhada com a linha da declaração.

```cpp
if (condition) {
    // Código
} else {
    // Código
}
```

### Espaçamento
Use um espaço ao redor de operadores (`=`, `+`, `-`, etc.).  
Use um espaço após as vírgulas em listas de parâmetros.

```cpp
int sum = a + b;
void func(int a, int b);
```

## Comentários

### Comentários de Linha Única
Use `//` para comentários de linha única.

```cpp
// Este é um comentário de linha única
int x = 5; // Inicializa x com 5
```

### Comentários de Bloco
Use `/* */` para comentários de bloco.

```cpp
/*
 * Este é um comentário de bloco.
 * Ele pode abranger várias linhas.
 */
```

## Boas Práticas
- Evite funções e classes muito grandes. (Podemos deixar 70 linhas como um "limite")
- Prefira variáveis e funções com nomes descritivos.
- Evite o uso de números mágicos. Use constantes com nomes significativos.

## Exemplo de Código

```cpp
// MyClass.hpp
#ifndef MYCLASS_H
#define MYCLASS_H

class MyClass {
private:
    int value;

public:
    MyClass();
    ~MyClass();
    
    int add(int a, int b);
};

#endif // MYCLASS_H

// MyClass.cpp
#include "MyClass.h"

MyClass::MyClass() {
    value = 0;
}

MyClass::~MyClass() {
    // Destrutor
}

int MyClass::add(int a, int b) {
    return a + b;
}
```
