#!/bin/bash

# Test script for ft_irc server using macOS leaks tool
# Based on test_valgrind.sh but adapted for macOS

if ! command -v leaks &> /dev/null; then
    echo "Ferramenta 'leaks' não está disponível. Esta é uma ferramenta nativa do macOS."
    echo "Tentando executar teste sem verificação de vazamentos..."
    
    # Simple test without memory leak detection
    ./ircserv 5555 pass &
    SERVER_PID=$!
    sleep 10
    kill $SERVER_PID
    wait $SERVER_PID
    echo "Teste básico concluído (sem verificação de vazamentos)."
    exit 0
fi

echo "Iniciando servidor IRC para teste de vazamentos de memória..."

# Start server in background
./ircserv 5555 pass > server_test.log 2>&1 &
SERVER_PID=$!

echo "Servidor iniciado com PID: $SERVER_PID"

# Let server run for a while
sleep 10

echo "Verificando vazamentos de memória com ferramenta 'leaks'..."

# Check for memory leaks using macOS leaks tool
leaks $SERVER_PID > leaks.log 2>&1

# Kill server
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null

echo "Servidor parado. Analisando resultados..."

# Check leaks results
if grep -q "0 leaks for" leaks.log; then
    echo "✅ Nenhum vazamento de memória detectado."
    rm -f leaks.log server_test.log
    exit 0
else
    echo "❌ Vazamento de memória detectado ou teste falhou."
    echo "Detalhes dos vazamentos:"
    cat leaks.log
    echo ""
    echo "Log do servidor:"
    cat server_test.log
    exit 1
fi
