# Python 3 server example
from http.server import BaseHTTPRequestHandler, HTTPServer
from uritools import urisplit
import serial
from time import sleep
import json
from socketserver import ThreadingMixIn
import threading
import cv2
from PIL import Image
from io import BytesIO

hostName = "localhost"
proxyName = hostName  # "proxy.example.com"
serverPort = 8080
proxyPort = serverPort  # 80

#serialport = None

#videocapture = None

image = None
imagesize = 0

imagecount = 0

def getnewframe():
    global image
    global imagesize
    global imagecount
    ret, frame = videocapture.read()
    if ret:
        imagecount = imagecount + 1
        dim = (160,100)
        scaledimage = cv2.resize(frame, dim, interpolation=cv2.INTER_AREA)
        imageRGB=cv2.cvtColor(scaledimage,cv2.COLOR_BGR2RGB)
        raw =  Image.fromarray(imageRGB)
        tmpFile = BytesIO()
        raw.save(tmpFile,'JPEG')
        imagesize = tmpFile.getbuffer().nbytes
        image = tmpFile.getvalue()
                

    else:
        image = None

def getframesinbackground():
    while(1):
        sleep(0.1)
        getnewframe()

def sendserial(name, value):
    global serialport
    serialport.write(value.encode())
    serialport.write(name.encode())


counter = 0
changecounter = 0
ledsdict = {
  "white": {"name":"d", "value":0, "color":"#fff", "changed":1},
  "nearUV": {"name":"f", "value":0, "color":"#30f", "changed":1},
  "blue1": {"name":"a", "value":0, "color":"#00f", "changed":1},
  "blue2": {"name":"c", "value":0, "color":"#00f", "changed":1},
  "yellow": {"name":"b", "value":0, "color":"#ff0", "changed":1},
  "green": {"name":"g", "value":0, "color":"#0f0", "changed":1},
  "red": {"name":"e", "value":0, "color":"#f00", "changed":1}
}


def setled(ledname, newvalue):
    global ledsdict
    lednameasc = str(ledname)
    print(newvalue)
    if lednameasc in ledsdict:
        oldvalue = ledsdict[lednameasc]["value"]
        if oldvalue != newvalue:
            ledsdict[lednameasc]["value"] = str(newvalue)
            ledsdict[lednameasc]["changed"] = 1
        return 1
    else:
        print ("not found %s " % lednameasc)
    return 0


blockserial = 0;


def sendledstoUART():
    global blockserial
    while (blockserial > 0):
        sleep(0.01)
    blockserial = 1;
    global ledsdict
    global changecounter
    changedvals = 0
    for led in ledsdict:
        value = str(ledsdict[led]["value"])
        changed = ledsdict[led]["changed"]
        name = ledsdict[led]["name"]
        if changed > 0:
            changecounter = changecounter + 1
            sendserial(name, value)
            ledsdict[led]["changed"] = 0
            changedvals = changedvals + 1
    blockserial = 0;
    return changedvals



def outleds():
    global ledsdict
    returnvalue = b"\n";
    for led in ledsdict:
        color = ledsdict[led]["color"]
        value = ledsdict[led]["value"]
        line = bytes("<span style=\"background-color: %s\"><input type=\"range\" min=\"0\" max=\"255\" value=\"%s\" class=\"slider\" id=\"slider%s\"></span><span style=\"display: inline-blockserial;width:6em;\">%s</span><br>\n" % (color, str(value), led, led), "utf-8")
        returnvalue = returnvalue + line
    return returnvalue

def outjs():
    global ledsdict 
    url = str("http://" + proxyName + ":" + str(proxyPort) + "/")
    returnvalue = bytes ("""
<script>
function sendvalue(name, value) {
const Http = new XMLHttpRequest();
const url = '%s';
var geturl = url + "?" +name+ "=" +value;
Http.open("GET", geturl);
Http.send();
}
""" % url, "utf-8")
    for led in ledsdict:
        # color = ledsdict[led]["color"]
        # name = ledsdict[led]["name"]
        returnvalue = returnvalue + bytes("var slider%s = document.getElementById(\"slider%s\");\n" % (led, led), "utf-8")
        returnvalue = returnvalue + bytes("slider%s.oninput = function() { sendvalue(\"%s\",this.value); }\n" % (led, led), "utf-8")
    returnvalue = returnvalue + bytes("""
var running = 1;
setInterval(function(){
    if (running) {
    load("?ask", function(Http) { 
        data=JSON.parse(Http.responseText);
        for (const name in data) {
            if (data.hasOwnProperty(name)) {
                entry = data[name];
                if (entry.hasOwnProperty('value'))
                   document.getElementById("slider"+name).value = entry['value']
            }
        } 
    });
    }
}, 100);
function load(url, callback) {  
    running = 0;
    const Http = new XMLHttpRequest(); 
    Http.timeout = 50000;
    Http.onreadystatechange = ensureReadiness;  
    Http.ontimeout = function (e) {
    running = 0;
    document.getElementById('container').innerHTML = "lost connection"
};
    function ensureReadiness() {  
        if(Http.readyState < 4) {  return; }  
        if(Http.status !== 200) {  return; }  
        if(Http.readyState === 4) {
            callback(Http);
            running = 1;
        }             
    }  
    Http.open("GET", url);  
    Http.send();  
}
""", "utf-8")
    returnvalue = returnvalue + bytes("""
    /*
    long text
    */
    """, "utf-8")
    
    returnvalue = returnvalue + bytes("</script>\n", "utf-8")
    return returnvalue


def longpoll(self) :
    global changecounter
    lastval = changecounter
    sleepcount = 0
    while (lastval == changecounter and sleepcount < 100):
        sleep(0.1)
        sleepcount = sleepcount + 1
    try:
        self.send_response(200)
        self.send_header("Content-type", "application/json")
        self.end_headers()
        self.wfile.write(bytes(json.dumps(ledsdict), "utf-8"))
    except BrokenPipeError:
        pass
    return

def oneimage(self):
    global image
    global imagesize
    self.send_response(200)
    self.send_header("Content-type", "image/jpeg")
    self.send_header("Cache-Control", "no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0")
    self.end_headers()
    self.wfile.write( image )
    self.wfile.flush()
    return

def videostream(self) :
    global image
    global imagesize
    self.send_response(200)
    self.send_header("Content-Type", "multipart/x-mixed-replace; boundary=jpgboundary")
    self.send_header("Cache-Control", "no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0")
    self.end_headers()
    lastimagecount = imagecount
    print ("streaming..")
    goon= 1
    while goon:
        while (lastimagecount == imagecount):
            sleep(0.05)
        lastimagecount = imagecount
        if (image is not None):
            try:
                self.wfile.write(bytes("\n--jpgboundary\n", "utf-8"))
                
                self.send_header("Content-Type", "image/jpeg")
                self.send_header("Content-Length", imagesize)
                self.end_headers()
                self.wfile.write( image )
                #self.wfile.flush()
            except BrokenPipeError:
                goon = 0
    return


class MyServer(BaseHTTPRequestHandler):

    def do_GET(self):
        global counter
        parts = urisplit(self.path)
        
        params = parts.getquerydict()
        counter = counter + 1
        hasparam = 0
        hasasktoken = 0
        hasavideotoken = 0
        hasaimagetoken = 0
        for param in params:
            hasparam = hasparam + setled(param, params[param][0])
            if param == "ask":
                hasasktoken = 1
            if param == "video":
                hasavideotoken = 1
            if param == "image":
                hasaimagetoken = 1
        if hasparam > 0:
            # print ("param recieved")
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            sendledstoUART()
            self.wfile.write(bytes("ok", "utf-8"))
        elif hasasktoken > 0:
            longpoll(self)
        elif hasavideotoken > 0:
            videostream(self)
        elif hasaimagetoken > 0:
            oneimage(self)
        else:
            # print ("no param recieved")
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            self.wfile.write(bytes("<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\"><title>PYLED</title><link rel=\"icon\" href=\"https://bedah.de/tux.ico\" /></head>", "utf-8"))
            self.wfile.write(bytes("<body style=\"color: #888;background-color: #000;\">", "utf-8"))
            self.wfile.write(bytes("<div id=\"container\"><img src=\"/?video\" alt=\"\"></div>" , "utf-8"))
            counter = counter + 1
            self.wfile.write(outleds())
            self.wfile.write(outjs())
            self.wfile.write(bytes("</body></html>", "utf-8"))


class ThreadingSimpleServer(ThreadingMixIn, HTTPServer):
    pass


if __name__ == "__main__":
    global serialport
    serialport =serial.Serial('/dev/ttyUSB3', 9600)
    serialport.isOpen()
    global videocapture
    videocapture = cv2.VideoCapture(0)
    if not videocapture.isOpened():
        videocapture.open()
    server = ThreadingSimpleServer((hostName, serverPort), MyServer)
    videothread = threading.Thread(target=getframesinbackground, name="videothread")
    videothread.start()
    print("Server started http://%s:%s" % (hostName, serverPort))
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass

    server.server_close()
    serialport.close()
    videocapture.release()
    print("Server stopped.")
