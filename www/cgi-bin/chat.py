#!/usr/bin/env python3
import os
import sys
import json
import time
from urllib.parse import unquote_plus

CHAT_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "chat.json")
MAX_MESSAGES = 200

write = sys.stdout.buffer.write

def load_messages():
	try:
		with open(CHAT_FILE, encoding="utf-8") as f:
			return json.load(f)
	except (OSError, ValueError):
		return []


def save_messages(messages):
	with open(CHAT_FILE, "w", encoding="utf-8") as f:
		json.dump(messages[-MAX_MESSAGES:], f, ensure_ascii=False)

def parse_post_body():
	try:
		length = int(os.environ.get("CONTENT_LENGTH") or 0)
	except ValueError:
		length = 0
	if length <= 0:
		return {}
	body = sys.stdin.buffer.read(length).decode("utf-8", "replace")
	fields = {}
	for pair in body.split("&"):
		if "=" in pair:
			key, value = pair.split("=", 1)
			fields[key] = unquote_plus(value)
	return fields

def send_header(content_type):
	write(("Content-Type: %s; charset=utf-8\r\n\r\n" % content_type).encode("utf-8"))

def send_json(messages):
	send_header("application/json")
	write(json.dumps(messages, ensure_ascii=False).encode("utf-8"))

def handle_post():
	fields = parse_post_body()
	nick = fields.get("nick", "").strip()[:24]
	text = fields.get("text", "").strip()[:500]
	messages = load_messages()
	if nick and text:
		messages.append({
			"nick": nick,
			"text": text,
			"time": time.strftime("%H:%M:%S"),
		})
		save_messages(messages)
		messages = load_messages()
	send_json(messages)

def handle_get():
	if os.environ.get("QUERY_STRING"):
		send_json(load_messages())
	else:
		send_header("text/html")
		write(PAGE.encode("utf-8"))

PAGE = """<!doctype html>
<meta charset="utf-8">
<title>chat</title>

<input id="nick" placeholder="username">
<input id="text" placeholder="message">
<button onclick="send()">send</button>

refresh:
<select id="interval">
  <option value="1">1 second</option>
  <option value="2">2 second</option>
  <option value="3">3 second</option>
  <option value="4">4 second</option>
  <option value="5">5 second</option>
  <option value="6">6 second</option>
  <option value="7">7 second</option>
  <option value="8">8 second</option>
  <option value="9">9 second</option>
  <option value="10">10 second</option>
</select>

<div id="box"></div>

<script>
var timer = null;

function load() {
  fetch("chat.py?messages")
	.then(function (r) { return r.json(); })
	.then(function (messages) {
	  box.innerHTML = messages.map(function (m) {
		return m.time + " <b>" + m.nick + ":</b> " + m.text;
	  }).join("<br>");
	});
}

function send() {
  var body = "nick=" + encodeURIComponent(nick.value) +
			 "&text=" + encodeURIComponent(text.value);
  fetch("chat.py", { method: "POST", body: body })
	.then(function (r) { return r.json(); })
	.then(function (messages) {
	  box.innerHTML = messages.map(function (m) {
		return m.time + " <b>" + m.nick + ":</b> " + m.text;
	  }).join("<br>");
	});
  text.value = "";
}

function startPolling() {
  if (timer) clearInterval(timer);
  var seconds = parseInt(interval.value, 10);
  timer = setInterval(load, seconds * 1000);
}

interval.onchange = startPolling;

load();
startPolling();
</script>
"""

if __name__ == "__main__":
	if os.environ.get("REQUEST_METHOD") == "POST":
		handle_post()
	else:
		handle_get()