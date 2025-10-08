#!/bin/bash

# Script para testar leak de memória nos testes BDD usando Valgrind
# Segue o padrão estabelecido em tests/test_valgrind.sh

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "================================"
echo "  Teste de leak - BDD"
echo "  Usando Valgrind"
echo "================================"
echo ""

if ! command -v valgrind &> /dev/null; then
    echo -e "${RED}Valgrind não está instalado. Por favor, instale o Valgrind.${NC}"
    exit 1
fi

if [ ! -f "./test_runner" ]; then
    echo -e "${RED}test_runner não encontrado. Execute 'make' primeiro.${NC}"
    exit 1
fi

LOG_FILE="valgrind_bdd.log"

echo -e "${YELLOW}Executando testes BDD com Valgrind...${NC}"
echo "Isso pode levar alguns minutos..."
echo ""

valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --track-fds=yes \
    --verbose \
    --log-file="$LOG_FILE" \
    ./test_runner 2>&1

VALGRIND_EXIT=$?

echo ""
echo "================================"
echo "  Análise de leaks"
echo "================================"
echo ""

if [ ! -f "$LOG_FILE" ]; then
    echo -e "${RED}Arquivo de log não foi criado.${NC}"
    exit 1
fi

HEAP_SUMMARY=$(grep "total heap usage:" "$LOG_FILE" | tail -1)
TOTAL_ALLOCS=$(echo "$HEAP_SUMMARY" | awk '{print $5}')
TOTAL_FREES=$(echo "$HEAP_SUMMARY" | awk '{print $7}')
TOTAL_BYTES=$(echo "$HEAP_SUMMARY" | awk '{print $9, $10, $11}')

FD_LEAKS=$(grep "Open file descriptor" "$LOG_FILE" | \
    grep -v "file descriptor [0-2]:" | \
    grep -v "std" | \
    grep -v "\.cursor-server" | \
    grep -v "\.vscode" | \
    grep -v "/tmp/vgdb-pipe" | \
    grep -v "exthost" | \
    grep -v "anysphere" | \
    grep -v "valgrind_bdd.log" | \
    grep -v "/dev/urandom" | \
    grep -v "/dev/random")

if [ -z "$FD_LEAKS" ]; then
    FD_LEAK_COUNT=0
else
    FD_LEAK_COUNT=$(echo "$FD_LEAKS" | wc -l | tr -d ' ')
fi

if grep -q "All heap blocks were freed -- no leaks are possible" "$LOG_FILE"; then
    DEFINITELY_LOST="0 bytes"
    INDIRECTLY_LOST="0 bytes"
    POSSIBLY_LOST="0 bytes"
    STILL_REACHABLE="0 bytes"
else
    DEFINITELY_LOST=$(grep "definitely lost:" "$LOG_FILE" | tail -1 | awk '{print $4, $5}')
    INDIRECTLY_LOST=$(grep "indirectly lost:" "$LOG_FILE" | tail -1 | awk '{print $4, $5}')
    POSSIBLY_LOST=$(grep "possibly lost:" "$LOG_FILE" | tail -1 | awk '{print $4, $5}')
    STILL_REACHABLE=$(grep "still reachable:" "$LOG_FILE" | tail -1 | awk '{print $4, $5}')
fi

echo "Estatísticas de Heap:"
echo "  Alocações totais:   ${TOTAL_ALLOCS:-N/A}"
echo "  Liberações totais:  ${TOTAL_FREES:-N/A}"
echo "  Bytes alocados:     ${TOTAL_BYTES:-N/A}"
echo ""

if [ "$FD_LEAK_COUNT" -gt 0 ]; then
    echo -e "${YELLOW}⚠ File Descriptors Vazados: $FD_LEAK_COUNT${NC}"
    echo "$FD_LEAKS" | head -10
    echo ""
fi

echo "Análise de leaks:"
echo "  Definitivos:        ${DEFINITELY_LOST:-N/A}"
echo "  Indiretos:          ${INDIRECTLY_LOST:-N/A}"
echo "  Possíveis:          ${POSSIBLY_LOST:-N/A}"
echo "  Ainda acessível:    ${STILL_REACHABLE:-N/A}"
echo ""

if grep -q "All heap blocks were freed -- no leaks are possible" "$LOG_FILE" && [ "$FD_LEAK_COUNT" -eq 0 ]; then
    echo -e "${GREEN}✓ Nenhum leak de memória detectado.${NC}"
    echo -e "${GREEN}✓ Todos os blocos da heap foram liberados corretamente.${NC}"
    echo -e "${GREEN}✓ Nenhum File Descriptor vazado.${NC}"
    EXIT_CODE=0
elif grep -q "definitely lost: 0 bytes" "$LOG_FILE" && [ "$FD_LEAK_COUNT" -eq 0 ]; then
    echo -e "${GREEN}✓ Nenhum leak definitivo de memória detectado.${NC}"
    echo -e "${GREEN}✓ Nenhum File Descriptor vazado.${NC}"
    EXIT_CODE=0
else
    if ! grep -q "All heap blocks were freed" "$LOG_FILE" && ! grep -q "definitely lost: 0 bytes" "$LOG_FILE"; then
        echo -e "${RED}✗ leak de memória detectado!${NC}"
        echo ""
        echo "Detalhes dos leaks:"
        grep -A 10 "definitely lost" "$LOG_FILE" | head -20
        echo ""
    fi
    if [ "$FD_LEAK_COUNT" -gt 0 ]; then
        echo -e "${RED}✗ File Descriptors não foram fechados!${NC}"
        echo ""
    fi
    echo -e "${YELLOW}Veja o arquivo $LOG_FILE para mais detalhes.${NC}"
    EXIT_CODE=1
fi

echo ""
echo "================================"

if [ $EXIT_CODE -eq 0 ]; then
    rm -f "$LOG_FILE"
fi

exit $EXIT_CODE
