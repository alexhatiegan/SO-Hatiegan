#!/bin/bash

# Verifică dacă numărul de argumente este corect
if [ "$#" -ne 1 ]; then
    echo "Utilizare: $0 <caracter>"
    exit 1
fi

# Caracterul primit ca și argument
char="$1"

# Folosind awk pentru a filtra și număra propozițiile corecte
counter=$(awk -v char="$char" '/^[A-Z][A-Za-z0-9 ,.!?]*[.!?]$/ && index($0, char) { count++ } END { print count }')

# Afișează rezultatul
echo "$counter"
