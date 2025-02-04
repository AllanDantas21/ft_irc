#!/bin/bash

if ! command -v valgrind &> /dev/null; then
    echo "Valgrind não está instalado. Por favor, instale o Valgrind."
    exit 1
fi

valgrind --leak-check=full --log-file=valgrind.log ../../.././ircserv 5555 pass &

SERVER_PID=$!

sleep 60

kill $SERVER_PID

wait $SERVER_PID

if grep -q "definitely lost: 0 bytes" valgrind.log; then
    echo "Nenhum vazamento de memória detectado."
else
    echo "Vazamento de memória detectado ou teste falhou."
    exit 1
fi

exit 0