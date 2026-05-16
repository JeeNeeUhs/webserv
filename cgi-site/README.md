# Guestbook CGI Site

Metin kutusu üzerinden mesaj alır, dosyaya yazar ve tüm mesajları listeler.
Aynı işlevsellik hem Python hem Go ile yazılmıştır.

## Dizin yapısı

```
cgi-site/
├── python/
│   └── guestbook.py      Python CGI (shebang: #!/usr/bin/env python3)
├── go/
│   ├── guestbook.go      Go CGI kaynağı (shebang: #!/usr/bin/env gorun)
│   └── Makefile          Binary derleme: make → ./guestbook
└── config/
    └── guestbook.conf    Webserv yapılandırması
```

## Veri dosyası

Her iki versiyon da mesajları şuraya yazar: `/tmp/webserv_guestbook.txt`  
Her satır bir mesajdır. Dosya yoksa otomatik oluşturulur.

## Python versiyonu

**Gereksinim:** Python 3

Doğrudan çalıştırılabilir:

```bash
chmod +x python/guestbook.py
```

Webserv config:
```
location /python {
    root cgi-site/python;
    index guestbook.py;
    methods GET POST;
    cgi_extension .py;
}
```

El ile test:
```bash
# GET
REQUEST_METHOD=GET ./python/guestbook.py

# POST
BODY="message=Merhaba"
echo -n "$BODY" | REQUEST_METHOD=POST CONTENT_LENGTH=${#BODY} ./python/guestbook.py
```

## Go versiyonu

İki seçenek vardır:

### Seçenek A — gorun (shebang ile doğrudan çalışır)

[gorun](https://github.com/erning/gorun) kurulu ise `.go` dosyası shebang ile direkt çalışır:

```bash
go install github.com/erning/gorun@latest
chmod +x go/guestbook.go
```

Webserv config:
```
location /go {
    root cgi-site/go;
    index guestbook.go;
    methods GET POST;
    cgi_extension .go;
}
```

### Seçenek B — Derlenmiş binary

```bash
cd go && make
chmod +x guestbook
```

Derlenen binary'yi CGI olarak çalıştırmak için webserv, dosyayı doğrudan `execve` ile çağırır.
Derlenmiş binary shebang gerektirmez; OS native ELF/Mach-O binary olarak tanır.

El ile test:
```bash
REQUEST_METHOD=GET ./go/guestbook

BODY="message=Hello"
echo -n "$BODY" | REQUEST_METHOD=POST CONTENT_LENGTH=${#BODY} ./go/guestbook
```

## CGI ortam değişkenleri

Webserv aşağıdaki değişkenleri set etmelidir:

| Değişken         | Açıklama                              |
|------------------|---------------------------------------|
| `REQUEST_METHOD` | `GET` veya `POST`                     |
| `CONTENT_LENGTH` | POST body byte sayısı                 |

POST body stdin üzerinden okunur.

## Notlar

- HTML çıktısında XSS koruması vardır (`<script>` → `&lt;script&gt;`)
- Boş mesajlar yok sayılır
- Mesajlar en yeni önce (ters sıra) gösterilir
