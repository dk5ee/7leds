
set up ssh with private/public key auth


open tunnel to server

`ssh -fNR 8088:localhost:8080 user@example.com`

in apache conf change virtualhost of a subdomain:

```
#1	DocumentRoot /var/www/example.com/subdomain
	ServerName subdomain.example.com
        ErrorDocument 502 http://www.example.com/proxy.html
        ErrorDocument 503 http://www.example.com/proxy.html
	ProxyPass /var/www/example.com/ !
#2	ProxyPass / http://127.0.0.1:8088/
#2	ProxyPassReverse / http://127.0.0.1:8088/
```

create http://www.example.com/proxy.html for offline time

comment #1 out, and uncomment #2

then `apache2ctl restart`