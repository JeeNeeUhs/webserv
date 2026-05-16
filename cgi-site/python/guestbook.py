#!/usr/bin/env python3
import sys
import os

DATA_FILE = "/tmp/webserv_guestbook.txt"


def url_decode(s):
    s = s.replace('+', ' ')
    result = []
    i = 0
    while i < len(s):
        if s[i] == '%' and i + 2 < len(s):
            try:
                result.append(chr(int(s[i + 1:i + 3], 16)))
                i += 3
                continue
            except ValueError:
                pass
        result.append(s[i])
        i += 1
    return ''.join(result)


def parse_body(body):
    params = {}
    for pair in body.split('&'):
        if '=' in pair:
            k, v = pair.split('=', 1)
            params[url_decode(k)] = url_decode(v)
    return params


def html_escape(s):
    return (s.replace('&', '&amp;')
             .replace('<', '&lt;')
             .replace('>', '&gt;')
             .replace('"', '&quot;'))


def read_messages():
    if not os.path.exists(DATA_FILE):
        return []
    with open(DATA_FILE, 'r') as f:
        return [line.rstrip('\n') for line in f if line.strip()]


def append_message(msg):
    with open(DATA_FILE, 'a') as f:
        f.write(msg + '\n')


def render(messages):
    rows = ''
    if messages:
        for msg in reversed(messages):
            rows += '      <div class="msg">%s</div>\n' % html_escape(msg)
    else:
        rows = '      <p class="empty">Henüz mesaj yok.</p>\n'

    return """<!DOCTYPE html>
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
</html>""" % (len(messages), rows)


method = os.environ.get('REQUEST_METHOD', 'GET').upper()

if method == 'POST':
    try:
        length = int(os.environ.get('CONTENT_LENGTH', '0') or '0')
    except ValueError:
        length = 0
    body = sys.stdin.read(length) if length > 0 else ''
    params = parse_body(body)
    msg = params.get('message', '').strip()
    if msg:
        append_message(msg)

messages = read_messages()

sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n")
sys.stdout.write("\r\n")
sys.stdout.write(render(messages))
