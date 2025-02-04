#!/bin/bash

if ! command -v valgrind &> /dev/null; then
    echo "Valgrind não encontrado. Por favor, instale o Valgrind."
    exit 1
fi


timeout 5s valgrind --leak-check=full --error-exitcode=1 ./ft_ircserv > /dev/null 2>&1
RESULTADO=$?

if [ $RESULTADO -eq 0 ]; then
    echo "Nenhum vazamento de memória detectado."
else
    echo "Vazamento de memória detectado ou teste falhou."
fi

exit $RESULTADO 