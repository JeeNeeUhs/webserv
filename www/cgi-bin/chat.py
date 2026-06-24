#!/usr/bin/env python3
import os, sys, json, time
from urllib.parse import unquote_plus

n = int(os.environ.get("CONTENT_LENGTH") or 0)
F = os.path.join(os.path.dirname(os.path.abspath(__file__)), "chat.json")
W = sys.stdout.buffer.write

def msgs():
    try: return json.load(open(F, encoding="utf-8"))
    except: return []

if os.environ.get("REQUEST_METHOD") == "POST":
    body = sys.stdin.buffer.read(n).decode("utf-8", "replace")
    d = dict(p.split("=", 1) for p in body.split("&") if "=" in p)
    nick = unquote_plus(d.get("nick", "")).strip()[:24]
    text = unquote_plus(d.get("text", "")).strip()[:500]
    m = msgs()
    if nick and text:
        m.append({"nick": nick, "text": text, "time": time.strftime("%H:%M:%S")})
        json.dump(m[-200:], open(F, "w", encoding="utf-8"), ensure_ascii=False)
    W(b"Content-Type: application/json; charset=utf-8\r\n\r\n")
    W(json.dumps(msgs(), ensure_ascii=False).encode("utf-8"))
else:
    W(b"Content-Type: text/html; charset=utf-8\r\n\r\n")
    W("""<!doctype html><meta charset=utf-8><title>chat</title>
<input id=nick placeholder=nick> <input id=t placeholder=mesaj>
<button onclick=send()>gonder</button>
<div id=box></div>
<script>
function load(b){fetch("chat.py",{method:"POST",body:b||""}).then(r=>r.json()).then(m=>{
box.innerHTML=m.map(x=>x.time+" <b>"+x.nick+":</b> "+x.text).join("<br>")})}
function send(){load("nick="+encodeURIComponent(nick.value)+"&text="+encodeURIComponent(t.value));t.value=""}
load();setInterval(load,1000);
</script>""".encode("utf-8"))