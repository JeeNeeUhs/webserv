#!/bin/bash
# envtest - verify ymawky sets cgi env vars per rfc 3875
#
# install:
#   1. save as www/cgi-bin/envtest (no extension)
#   2. chmod +x www/cgi-bin/envtest
#   3. visit http://localhost:PORT/cgi-bin/envtest
#   4. submit the form to test CONTENT_TYPE/CONTENT_LENGTH

html_escape() {
    local s="$1"
    s="${s//&/&amp;}"
    s="${s//</&lt;}"
    s="${s//>/&gt;}"
    s="${s//\"/&quot;}"
    s="${s//\'/&#39;}"
    printf '%s' "$s"
}

is_set() {
    eval "test -n \"\${${1}+x}\""
}

row() {
    local cls="$1" icon="$2" var="$3" exp="$4" act="$5"
    printf '<tr class="%s"><td class="i">%s</td><td class="v"><code>%s</code></td><td class="e">%s</td><td class="a"><code>%s</code></td></tr>\n' \
        "$cls" "$icon" "$(html_escape "$var")" "$(html_escape "$exp")" "$(html_escape "$act")"
}

check_required() {
    local var="$1" matcher="$2" expected="$3"
    if ! is_set "$var"; then
        row "err" "✗" "$var" "$expected" "(unset)"
        return
    fi
    local val="${!var}"
    case "$matcher" in
        exact:*)
            local want="${matcher#exact:}"
            if [ "$val" = "$want" ]; then
                row "ok" "✓" "$var" "$expected" "${val:-(empty)}"
            else
                row "warn" "⚠" "$var" "$expected" "${val:-(empty)}"
            fi
            ;;
        regex:*)
            local pat="${matcher#regex:}"
            if [[ "$val" =~ $pat ]]; then
                row "ok" "✓" "$var" "$expected" "${val:-(empty)}"
            else
                row "warn" "⚠" "$var" "$expected" "${val:-(empty)}"
            fi
            ;;
        any)
            row "ok" "✓" "$var" "$expected" "${val:-(empty, but set)}"
            ;;
    esac
}

printf 'Status: 200 OK\r\n'
printf 'Content-Type: text/html; charset=utf-8\r\n\r\n'

cat <<'HTML_HEAD'
<!DOCTYPE html>
<html>
<head>
<title>cgi env test</title>
<style>
body {
    background: #1a1a2e;
    color: #c9b0ff;
    font-family: "Arial Rounded MT Bold", "Helvetica Rounded", system-ui, sans-serif;
    font-weight: normal;
    margin: 0;
    min-height: 100vh;
    padding: 2em 1em;
    box-sizing: border-box;
    display: flex;
    flex-direction: column;
    align-items: center;
}
.container { max-width: 1100px; width: 100%; }
h1 { font-weight: normal; font-size: 2.5em; margin: 0 0 0.3em 0; }
h2 {
    font-weight: normal;
    font-size: 1.2em;
    color: #c9b0ff;
    margin: 2em 0 0.8em 0;
    padding-bottom: 0.3em;
    border-bottom: 1px solid #2a2a48;
}
.subtitle { color: #7a6494; font-size: 0.9em; margin: 0 0 2em 0; }
code {
    font-family: "SF Mono", Menlo, Consolas, monospace;
    font-size: 0.85em;
    color: #c9b0ff;
}
.layout {
    display: grid;
    grid-template-columns: 3fr 2fr;
    gap: 1.5em;
    margin-top: 2em;
    align-items: start;
}
.col { min-width: 0; }
.col h2:first-child,
.col-stack section h2 { margin-top: 0; }
.col-stack {
    display: flex;
    flex-direction: column;
    gap: 1.5em;
    min-width: 0;
}
@media (max-width: 800px) {
    .layout { grid-template-columns: 1fr; }
}
table {
    width: 100%;
    border-collapse: collapse;
    background: #232342;
    border-radius: 4px;
    overflow: hidden;
    font-size: 0.9em;
    table-layout: fixed;
}
thead { background: #2a2a48; }
th, td { padding: 0.3em 0.1em; text-align: left; vertical-align: top; }
th { font-weight: normal; color: #7a6494; font-size: 0.85em; }
/* status column — both th and td need the width for table-layout: fixed */
.i { width: 1.8em; padding: 0.3em 0.1em; text-align: center; }
td.i { font-size: 1.1em; }
.v { width: 35%; word-break: break-word; }
.e { color: #7a6494; font-size: 0.85em; width: 28%; }
.a { color: #c9b0ff; overflow-wrap: break-word; }
tr.ok td.i { color: #6ec98a; }
tr.warn td.i { color: #d9c46e; }
tr.warn td.a { color: #d9c46e; }
tr.err td.i { color: #c46e7a; }
tr.err td.a { color: #c46e7a; }
tr.na td.i { color: #555; }
tr.na td.a { color: #555; }
tr + tr td { border-top: 1px solid #2a2a48; }
.envdump {
    background: #11111e;
    border: 1px solid #2a2a48;
    border-radius: 4px;
    padding: 1em 1.2em;
    color: #7a6494;
    font-size: 0.8em;
    overflow-x: auto;
    line-height: 1.5;
}
.divider {
    color: #333;
    text-align: center;
    margin: 2em 0 1em 0;
    font-size: 0.8em;
    letter-spacing: 0.3em;
    user-select: none;
}
form { display: flex; gap: 0.5em; flex-wrap: wrap; }
input[type=text] {
    background: #11111e;
    color: #c9b0ff;
    border: 1px solid #473769;
    border-radius: 3px;
    padding: 0.6em 0.8em;
    font-family: inherit;
    font-size: 0.95em;
    outline: none;
    flex: 1;
    min-width: 12em;
}
input[type=text]:focus { border-color: #c9b0ff; }
button {
    background: #473769;
    color: #c9b0ff;
    border: none;
    padding: 0.6em 1.2em;
    font-family: inherit;
    font-size: 0.95em;
    cursor: pointer;
    border-radius: 3px;
}
button:hover { background: #5a4485; }
.footer { color: #666; font-size: 0.8em; text-align: center; margin-top: 3em; }
.legend {
    color: #7a6494;
    font-size: 0.8em;
    margin: 0.5em 0 1.5em 0;
}
.legend span { margin-right: 1.2em; }
.legend .ok { color: #6ec98a; }
.legend .warn { color: #d9c46e; }
.legend .err { color: #c46e7a; }
.legend .na { color: #555; }
</style>
</head>
<body>
<div class="container">
<h1>cgi env test</h1>
HTML_HEAD

req_summary="${REQUEST_METHOD:-?} ${SCRIPT_NAME:-?}"
if [ -n "$QUERY_STRING" ]; then req_summary="${req_summary}?${QUERY_STRING}"; fi
printf '<p class="subtitle">incoming: <code>%s</code></p>\n' "$(html_escape "$req_summary")"

printf '<p class="legend"><span class="ok">✓ ok</span><span class="warn">⚠ set, value unexpected</span><span class="err">✗ missing</span></p>\n'

printf '<div class="layout">\n'

# ─── LEFT COLUMN: required ───
printf '<section class="col">\n'
printf '<h2>required</h2>\n'
printf '<table><thead><tr><th class="i"></th><th class="v">variable</th><th class="e">expected</th><th class="a">actual</th></tr></thead><tbody>\n'

check_required "GATEWAY_INTERFACE" "exact:CGI/1.1"  "CGI/1.1"
check_required "SERVER_PROTOCOL"   "exact:HTTP/1.1" "HTTP/1.1"
check_required "SERVER_SOFTWARE"   "any"            "non-empty"
check_required "SERVER_NAME"       "any"            "hostname or IP"
check_required "SERVER_PORT"       "regex:^[0-9]+$" "port number"
check_required "REQUEST_METHOD"    "regex:^(GET|POST|PUT|DELETE|HEAD|OPTIONS|PATCH)$" "GET/PUT/etc"
check_required "SCRIPT_NAME"       "regex:^/"       "URL path, /..."
check_required "REMOTE_ADDR"       "regex:^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$" "dotted quad"
check_required "REMOTE_HOST"       "any"            "hostname or IP"
if is_set "QUERY_STRING"; then
    row "ok" "✓" "QUERY_STRING" "set, may be empty" "${QUERY_STRING:-(empty, but set)}"
else
    row "err" "✗" "QUERY_STRING" "set, may be empty" "(unset)"
fi

printf '</tbody></table>\n'
printf '</section>\n'

# ─── RIGHT COLUMN: conditional + optional stacked ───
printf '<div class="col-stack">\n'

printf '<section>\n'
printf '<h2>conditional (MUST when body sent)</h2>\n'
printf '<table><thead><tr><th class="i"></th><th class="v">variable</th><th class="e">expected</th><th class="a">actual</th></tr></thead><tbody>\n'

if [ "$REQUEST_METHOD" = "POST" ] || [ "$REQUEST_METHOD" = "PUT" ]; then
    check_required "CONTENT_TYPE"   "any"            "media type"
    check_required "CONTENT_LENGTH" "regex:^[0-9]+$" "byte count"
else
    if is_set "CONTENT_TYPE"; then
        row "warn" "⚠" "CONTENT_TYPE" "should not be set" "$CONTENT_TYPE"
    else
        row "na" "—" "CONTENT_TYPE" "POST/PUT only" "(unset, correct)"
    fi
    if is_set "CONTENT_LENGTH"; then
        row "warn" "⚠" "CONTENT_LENGTH" "should not be set" "$CONTENT_LENGTH"
    else
        row "na" "—" "CONTENT_LENGTH" "POST/PUT only" "(unset, correct)"
    fi
fi

printf '</tbody></table>\n'
printf '</section>\n'

printf '<section>\n'
printf '<h2>optional / conventional</h2>\n'
printf '<table><thead><tr><th class="i"></th><th class="v">variable</th><th class="e">expected</th><th class="a">actual</th></tr></thead><tbody>\n'

if is_set "HTTP_HOST"; then
    row "ok" "✓" "HTTP_HOST" "Host: header" "$HTTP_HOST"
else
    row "warn" "⚠" "HTTP_HOST" "Host: header" "(unset)"
fi

if is_set "PATH_INFO"; then
    row "ok" "✓" "PATH_INFO" "extra URL path" "${PATH_INFO:-(empty)}"
else
    row "na" "—" "PATH_INFO" "extra URL path" "(unset, fine)"
fi

if is_set "PATH"; then
    row "ok" "✓" "PATH" "exec search path" "$PATH"
else
    row "warn" "⚠" "PATH" "exec search path" "(unset)"
fi

printf '</tbody></table>\n'
printf '</section>\n'

printf '</div>\n'   # close .col-stack
printf '</div>\n'   # close .layout

cat <<EOF
<div class="divider">~ ~ ~</div>
<h2>test POST</h2>
<p class="subtitle">submit to verify <code>CONTENT_TYPE</code> and <code>CONTENT_LENGTH</code></p>
<form action="/cgi-bin/envtest.sh" method="POST">
<input type="text" name="data" placeholder="any test string" value="hello">
<button type="submit">POST it</button>
</form>
EOF

printf '<h2>full environment</h2>\n'
printf '<pre class="envdump">'
env | sort | while IFS= read -r line; do
    printf '%s\n' "$(html_escape "$line")"
done
printf '</pre>\n'

cat <<'EOF'
<div class="footer">ymawky :: hand-rolled aarch64 :: envtest v0.5</div>
</div>
</body>
</html>
EOF