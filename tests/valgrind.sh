#!/bin/bash

make

if [ $? -ne 0 ]; then
  echo "Error"
  exit 1
fi

valgrind --leak-check=full --track-origins=yes --error-exitcode=1 ./ircserver

if [ $? -ne 0 ]; then
  echo "Leak"
  exit 1
else
  echo "No"
fi

make fclean