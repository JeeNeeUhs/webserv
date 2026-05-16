#!/usr/bin/env gorun
package main

import (
	"bufio"
	"fmt"
	"html"
	"net/url"
	"os"
	"strconv"
	"strings"
)

const dataFile = "/tmp/webserv_guestbook.txt"

func readMessages() []string {
	f, err := os.Open(dataFile)
	if err != nil {
		return nil
	}
	defer f.Close()

	var lines []string
	sc := bufio.NewScanner(f)
	for sc.Scan() {
		if line := strings.TrimSpace(sc.Text()); line != "" {
			lines = append(lines, line)
		}
	}
	return lines
}

func appendMessage(msg string) {
	f, err := os.OpenFile(dataFile, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		return
	}
	defer f.Close()
	fmt.Fprintln(f, msg)
}

func render(messages []string) string {
	var rows strings.Builder
	if len(messages) == 0 {
		rows.WriteString(`      <p class="empty">Henüz mesaj yok.</p>` + "\n")
	} else {
		for i := len(messages) - 1; i >= 0; i-- {
			rows.WriteString(`      <div class="msg">` + html.EscapeString(messages[i]) + "</div>\n")
		}
	}

	return fmt.Sprintf(`<!DOCTYPE html>
<html lang="tr">
<head>
  <meta charset="utf-8">
  <title>Guestbook</title>
  <style>
    body { font-family: sans-serif; max-width: 640px; margin: 48px auto; padding: 0 16px; color: #222; }
    h1   { border-bottom: 2px solid #0078d4; padding-bottom: 8px; }
    form { display: flex; flex-direction: column; gap: 8px; }
    textarea { resize: vertical; padding: 8px; font-size: 14px; border: 1px solid #ccc; border-radius: 4px; }
    button { align-self: flex-start; padding: 8px 20px; background: #0078d4; color: #fff;
             border: none; border-radius: 4px; cursor: pointer; font-size: 14px; }
    button:hover { background: #005fa3; }
    h2   { margin-top: 36px; }
    .msg { padding: 10px 0; border-bottom: 1px solid #e0e0e0; font-size: 15px; white-space: pre-wrap; }
    .empty { color: #888; font-style: italic; }
  </style>
</head>
<body>
  <h1>Guestbook</h1>
  <form method="POST" action="">
    <textarea name="message" rows="4" placeholder="Mesajınızı buraya yazın..."></textarea>
    <button type="submit">Gönder</button>
  </form>
  <h2>Mesajlar (%d)</h2>
%s</body>
</html>`, len(messages), rows.String())
}

func main() {
	method := strings.ToUpper(os.Getenv("REQUEST_METHOD"))

	if method == "POST" {
		clStr := os.Getenv("CONTENT_LENGTH")
		cl, _ := strconv.Atoi(clStr)
		if cl > 0 {
			buf := make([]byte, cl)
			n, _ := os.Stdin.Read(buf)
			vals, err := url.ParseQuery(string(buf[:n]))
			if err == nil {
				if msg := strings.TrimSpace(vals.Get("message")); msg != "" {
					appendMessage(msg)
				}
			}
		}
	}

	messages := readMessages()

	fmt.Print("Content-Type: text/html; charset=utf-8\r\n")
	fmt.Print("\r\n")
	fmt.Print(render(messages))
}
