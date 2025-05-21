# Documentação das Pipelines GitHub Actions

## Visão Geral

1. **Compile ft_irc**: Verifica se o projeto compila corretamente
2. **Valgrind Memory Leak Test**: Verifica vazamentos de memória no servidor

## Pipeline de Compilação

**Arquivo**: `.github/workflows/compile.yml`

### Objetivo
Verificar se o código-fonte do projeto compila sem erros.

### Gatilhos
- Push para a `main` 
- Pull Requests para a `main`

### Passos
1. Checkout do repositório
2. Instalação das dependências necessárias (make, g++, valgrind)
3. Compilação do projeto usando o comando `make`

### Comportamento
Esta pipeline falha se o código não compilar corretamente, o que impede a introdução de código quebrado nas branches principais.

## Pipeline de Teste de Vazamento de Memória

**Arquivo**: `.github/workflows/valgrind_test.yml`

### Objetivo
Verificar se o servidor IRC possui vazamentos de memória usando a ferramenta Valgrind.

### Gatilhos
- Push para as branches `main` ou `fix_valgrind_test`
- Pull Requests para as branches `main` ou `fix_valgrind_test`

### Passos
1. Checkout do repositório
2. Instalação do Valgrind
3. Concessão de permissão de execução ao script de teste
4. Compilação do projeto
5. Execução do script de teste de vazamento de memória

### Comportamento
O script `test_valgrind.sh` inicia o servidor com o Valgrind, espera 10 segundos e verifica se existem vazamentos de memória. A pipeline falha se forem detectados vazamentos.