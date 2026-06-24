#!/bin/bash

printf "Status: 200 OK\r\n"
printf "Content-Type: text/plain\r\n\r\n"

printf "hello from cgi response\r\n"
printf "server date: $(date)\r\n"
