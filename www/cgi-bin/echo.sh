#!/bin/bash

printf "Status: 200 OKok\r\n"
printf "Content-Type: text/plain\r\n\r\n" # Header ile body arasındaki boş satır (zorunlu)

# Body
printf "Merhaba! Bu bir CGI yan.\r\n"
printf "Sunucu zaman: $(date)\r\n"

sleep 10

printf "zartladim\r\n"
