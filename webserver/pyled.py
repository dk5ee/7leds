# Python 3 server example
from http.server import BaseHTTPRequestHandler, HTTPServer
from uritools import urisplit
import serial

hostName = "localhost"
proxyName = "localhost"
serverPort = 8080
proxyPort = 8080

ser = serial.Serial('/dev/ttyUSB3', 9600)
ser.isOpen()

def sendserial(name,value):
    global ser
    ser.write(value.encode())
    ser.write(name.encode())

counter = 0
ledsdict = {
  "blue1": {"name":"a", "value":0, "color":"#00f", "changed":0},
  "yellow": {"name":"b", "value":0, "color":"#ff0", "changed":0},
  "blue2": {"name":"c", "value":0, "color":"#00f", "changed":0},
  "white": {"name":"d", "value":0, "color":"#fff", "changed":0},
  "red": {"name":"e", "value":0, "color":"#f00", "changed":0},
  "blue3": {"name":"f", "value":0, "color":"#00f", "changed":0},
  "green": {"name":"g", "value":0, "color":"#0f0", "changed":0}
}


def setled(ledname, newvalue):
    global ledsdict
    lednameasc = str(ledname)
    if lednameasc in ledsdict:
        oldvalue = ledsdict[lednameasc]["value"]
        if oldvalue != newvalue:
            ledsdict[lednameasc]["value"] = newvalue
            ledsdict[lednameasc]["changed"] = 1
        return 1
    else:
        print ("not found %s " % lednameasc )
    return 0

def sendleds():
    global ledsdict
    changedvals = 0
    for led in ledsdict:
        value = str(ledsdict[led]["value"])
        changed = ledsdict[led]["changed"]
        name = ledsdict[led]["name"]
        if changed >0:
            sendserial(name,value)
            ledsdict[led]["changed"] = 0
            changedvals = changedvals +1
    return changedvals


def outleds():
    global ledsdict
    returnvalue = b"\n";
    for led in ledsdict:
        color = ledsdict[led]["color"]
        value = ledsdict[led]["value"]
        line = bytes("<span style=\"background-color: %s\"><input type=\"range\" min=\"0\" max=\"255\" value=\"%s\" class=\"slider\" id=\"slider%s\"></span><span style=\"display: inline-block;width:6em;\">%s</span><br>\n" % (color, str(value), led, led), "utf-8")
        returnvalue = returnvalue + line
    return returnvalue


def outjs():
    global ledsdict 
    url = str("http://" + proxyName + ":" + str(proxyPort) + "/")
    returnvalue = bytes ("<script>\nfunction sendvalue(name, value) {\n"\
" const Http = new XMLHttpRequest();\n"\
" const url = '%s';\n"\
" var geturl = url + \"?\" +name+ \"=\" +value;\n"\
" /* console.log(\"log: \"+geturl); */\n"\
" Http.open(\"GET\", geturl);\n"\
" Http.send();\n"\
"}\n" % url, "utf-8")
    for led in ledsdict:
        # color = ledsdict[led]["color"]
        # name = ledsdict[led]["name"]
        line = bytes("var slider%s = document.getElementById(\"slider%s\");\n" % (led, led), "utf-8")
        returnvalue = returnvalue + line
        line = bytes("slider%s.oninput = function() { sendvalue(\"%s\",this.value); }\n" % (led, led), "utf-8")
        returnvalue = returnvalue + line
    line = bytes("</script>\n", "utf-8")
    returnvalue = returnvalue + line
    return returnvalue


class MyServer(BaseHTTPRequestHandler):

    def do_GET(self):
        global counter
        parts = urisplit(self.path)
        
        params = parts.getquerydict()
        counter = counter + 1
        hasparam = 0
        for param in params:
            hasparam = hasparam + setled(param, params[param])
        if hasparam > 0:
            print ("param recieved")
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            sendleds()
            self.wfile.write(bytes("ok", "utf-8"))
        else:
            print ("no param recieved")
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            self.wfile.write(bytes("<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\"><title>PYLED</title><link rel=\"icon\" href=\"https://bedah.de/tux.ico\" /></head>", "utf-8"))
            self.wfile.write(bytes("<body>", "utf-8"))
            #self.wfile.write(bytes("<p>parts.scheme: %s</p>" % parts.scheme, "utf-8"))
            #self.wfile.write(bytes("<p>parts.authority: %s</p>" % parts.authority, "utf-8"))
            #self.wfile.write(bytes("<p>parts.getquerydict(): %s</p>" % parts.getquerydict(), "utf-8"))
            #self.wfile.write(bytes("<p>Request: %s</p>" % self.path, "utf-8"))
            counter = counter + 1
            self.wfile.write(bytes("<p>a counter: %d</p>" % counter, "utf-8"))
            self.wfile.write(outleds())
            self.wfile.write(outjs())
            self.wfile.write(bytes("</body></html>", "utf-8"))


if __name__ == "__main__":
    webServer = HTTPServer((hostName, serverPort), MyServer)
    print("Server started http://%s:%s" % (hostName, serverPort))
    try:
        webServer.serve_forever()
    except KeyboardInterrupt:
        pass

    webServer.server_close()
    ser.close()
    print("Server stopped.")
