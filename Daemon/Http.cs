using System;
using System.Collections;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using Daemon;
using Microsoft.Win32;

namespace MyHttp
{
    public class HttpProcessor
    {
        private Form1 instance;
        public TcpClient socket;
        public HttpServer srv;

        private Stream inputStream;
        public StreamWriter outputStream;

        public String http_method;
        public String http_url;
        public String http_protocol_versionstring;
        public Hashtable httpHeaders = new Hashtable();


        private static int MAX_POST_SIZE = 10 * 1024 * 1024; // 10MB

        public HttpProcessor(TcpClient s, HttpServer srv, Form1 instance)
        {
            this.socket = s;
            this.srv = srv;
            this.instance = instance;
        }


        private string streamReadLine(Stream inputStream)
        {
            int next_char;
            string data = "";
            try
            {
                while (true)
                {
                    next_char = inputStream.ReadByte();
                    if (next_char == '\n') { break; }
                    if (next_char == '\r') { continue; }
                    if (next_char == -1) { Thread.Sleep(1); continue; };
                    data += Convert.ToChar(next_char);
                }
            }
            catch (Exception e) { instance.log(e.ToString()); }
            return data;
        }
        public void process()
        {
            // we can't use a StreamReader for input, because it buffers up extra data on us inside it's
            // "processed" view of the world, and we want the data raw after the headers
            inputStream = new BufferedStream(socket.GetStream());

            // we probably shouldn't be using a streamwriter for all output from handlers either
            outputStream = new StreamWriter(new BufferedStream(socket.GetStream()));
            try
            {
                parseRequest();
                readHeaders();
                if (http_method.Equals("GET"))
                {
                    handleGETRequest();
                }
                else if (http_method.Equals("POST"))
                {
                    handlePOSTRequest();
                }
            outputStream.Flush();
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception: " + e.ToString());
                writeFailure();
            }
            // bs.Flush(); // flush any remaining output
            inputStream = null; outputStream = null; // bs = null;            
            socket.Close();
        }

        public void parseRequest()
        {
            try {
                String request = streamReadLine(inputStream);
                string[] tokens = request.Split(' ');
                if (tokens.Length != 3)
                {
                    throw new Exception("invalid http request line");
                }
                http_method = tokens[0].ToUpper();
                http_url = tokens[1];
                http_protocol_versionstring = tokens[2];

                Console.WriteLine("starting: " + request);
            }catch(Exception e) { instance.log(e.ToString()); }
        }

        public void readHeaders()
        {
            try {
                String line;
                while ((line = streamReadLine(inputStream)) != null)
                {
                    if (line.Equals(""))
                    {
                        Console.WriteLine("got headers");
                        return;
                    }

                    int separator = line.IndexOf(':');
                    if (separator == -1)
                    {
                        throw new Exception("invalid http header line: " + line);
                    }
                    String name = line.Substring(0, separator);
                    int pos = separator + 1;
                    while ((pos < line.Length) && (line[pos] == ' '))
                    {
                        pos++; // strip any spaces
                    }

                    string value = line.Substring(pos, line.Length - pos);
                    Console.WriteLine("header: {0}:{1}", name, value);
                    httpHeaders[name] = value;
                }
            }
            catch (Exception e) { instance.log(e.ToString()); }
        }

        public void handleGETRequest()
        {
            srv.handleGETRequest(this);
        }

        private const int BUF_SIZE = 4096;
        public void handlePOSTRequest()
        {
            // this post data processing just reads everything into a memory stream.
            // this is fine for smallish things, but for large stuff we should really
            // hand an input stream to the request processor. However, the input stream 
            // we hand him needs to let him see the "end of the stream" at this content 
            // length, because otherwise he won't know when he's seen it all! 
            try {
                int content_len = 0;
                MemoryStream ms = new MemoryStream();
                if (this.httpHeaders.ContainsKey("Content-Length"))
                {
                    content_len = Convert.ToInt32(this.httpHeaders["Content-Length"]);
                    if (content_len > MAX_POST_SIZE)
                    {
                        throw new Exception(
                            String.Format("POST Content-Length({0}) too big for this simple server",
                              content_len));
                    }
                    byte[] buf = new byte[BUF_SIZE];
                    int to_read = content_len;
                    while (to_read > 0)
                    {
                        Console.WriteLine("starting Read, to_read={0}", to_read);

                        int numread = this.inputStream.Read(buf, 0, Math.Min(BUF_SIZE, to_read));
                        Console.WriteLine("read finished, numread={0}", numread);
                        if (numread == 0)
                        {
                            if (to_read == 0)
                            {
                                break;
                            }
                            else
                            {
                                throw new Exception("client disconnected during post");
                            }
                        }
                        to_read -= numread;
                        ms.Write(buf, 0, numread);
                    }
                    ms.Seek(0, SeekOrigin.Begin);
                }
                Console.WriteLine("get post data end");
                srv.handlePOSTRequest(this, new StreamReader(ms));
            }
            catch (Exception e) { instance.log(e.ToString()); }
        }

        public void writeSuccess(string content_type = "text/html")
        {
            try {
                // this is the successful HTTP response line
                outputStream.WriteLine("HTTP/1.0 200 OK");
                // these are the HTTP headers...          
                outputStream.WriteLine("Content-Type: " + content_type);
                outputStream.WriteLine("Connection: close");
                outputStream.WriteLine("Cache-control: max-age=3600");
                // ..add your own headers here if you like

                outputStream.WriteLine(""); // this terminates the HTTP headers.. everything after this is HTTP body..
                outputStream.Flush();
            }catch(Exception e)
            {
                instance.log(e.ToString());
            }
        }

        public void writeFailure()
        {
            try
            {
                // this is an http 404 failure response
                outputStream.WriteLine("HTTP/1.0 404 File not found");
                // these are the HTTP headers
                outputStream.WriteLine("Connection: close");
                // ..add your own headers here

                outputStream.WriteLine(""); // this terminates the HTTP headers.
                outputStream.Flush();
            }
            catch (Exception e) { instance.log(e.ToString()); }
        }
    }

    public abstract class HttpServer
    {

        protected int port;
        TcpListener listener;
        bool is_active = true;

        private Form1 instance;

        public HttpServer(int port, Form1 instance)
        {
            this.port = port;
            this.instance = instance;
        }

        public void listen()
        {
            listener = new TcpListener(port);
            listener.Start();
            while (is_active)
            {
                try { 
                TcpClient s = listener.AcceptTcpClient();
                HttpProcessor processor = new HttpProcessor(s, this, instance);
                Thread thread = new Thread(new ThreadStart(processor.process));
                thread.Start();
                Thread.Sleep(1);
                }
                catch (Exception e) { instance.log(e.ToString()); }
            }
        }

        public abstract void handleGETRequest(HttpProcessor p);
        public abstract void handlePOSTRequest(HttpProcessor p, StreamReader inputData);
    }

    public class MyHttpServer : HttpServer
    {
        private Form1 instance;

        public MyHttpServer(int port, Form1 _instance)
            : base(port,_instance)
        {
            instance = _instance;
        }
        public override void handleGETRequest(HttpProcessor p)
        {
            instance.log("HTTP请求：" + p.http_url);
            try {
                var url = p.http_url.Substring(1);
                if (p.http_url.StartsWith("/doPWM"))
                {
                    var a = p.http_url.Split('?')[1];
                    var node = a.Split('&')[0];
                    var value = a.Split('&')[1];
                    foreach (var act in instance.actuators)
                    {
                        if (act.node_name == node)
                        {
                            if (act.id == "door") { instance.doorlock = false; instance.dooraction = DateTime.Now; }
                            instance.control(act, value);
                            break;
                        }
                    }
                    url = "ok.html";
                }
                if (p.http_url.StartsWith("/doSwitch"))
                {
                    var a = p.http_url.Split('?')[1];
                    var node = a.Split('&')[0];
                    var value = (int.Parse(node.Substring(node.Length - 1))+1).ToString();
                    node = node.Substring(0,node.Length - 1);
                    var sign = a.Split('&')[1];
                    if (sign == "true") value = "-" + value;
                    foreach (var act in instance.actuators)
                    {
                        if (act.node_name == node)
                        {
                            instance.control(act, value);
                            break;
                        }
                    }
                    url = "ok.html";
                }
                if (p.http_url.StartsWith("/initApp"))
                {
                    using (var sw = new StreamWriter("init.json"))
                    {
                        var firstItem = true;
                        sw.WriteLine("{\"sensors\":[");
                        foreach (var sensor in instance.sensors)
                        {
                            if (!firstItem) sw.Write(",");
                            firstItem = false;
                            sw.WriteLine("{\"type\":\"" + sensor.type + "\",\"name\":\"" + sensor.name + "\"}");
                        }
                        firstItem = true;
                        sw.WriteLine("],\"actuators\":[");
                        foreach (var actuator in instance.actuators)
                        {
                            if (!firstItem) sw.Write(",");
                            firstItem = false;
                            if (actuator.actuator_name == "PWM")
                                sw.WriteLine("{\"type\":\"" + actuator.actuator_name + "\",\"name\":\"" + actuator.display_name + "\",\"node\":\"" + actuator.node_name + "\",\"min\":\"" + actuator.min + "\",\"max\":\"" + actuator.max + "\",\"def\":\"" + actuator.def + "\"}");
                            else if (actuator.actuator_name == "switch")
                            {
                                var arr = "";
                                var first = true;
                                for(int i=0;i<actuator.pic.Length;i++)
                                {
                                    if (!first) arr += ",";
                                    first = false;
                                    arr += "{\"pic\":\"" + actuator.pic[i] + "\",\"dname\":\"" + actuator.dname[i] + "\"}";
                                }
                                sw.WriteLine("{\"type\":\"" + actuator.actuator_name + "\",\"name\":\"" + actuator.display_name + "\",\"node\":\"" + actuator.node_name + "\",\"arr\":[" + arr + "]}");
                            }
                        }
                        sw.WriteLine("]}");
                    }
                    url = "init.json";
                }
                if (p.http_url.StartsWith("/getSensor"))
                {
                    using (var sw = new StreamWriter("sensor.json"))
                    {
                        var firstItem = true;
                        sw.WriteLine("{\"sensors\":[");
                        foreach (var sensor in instance.sensors)
                        {
                            if (!firstItem) sw.Write(",");
                            firstItem = false;
                            sw.WriteLine("{\"value\":\"" + (sensor.connected && sensor.status ? sensor.getValue() : "未连接") + "\"}");
                        }
                        sw.WriteLine("]}");
                    }
                    url = "sensor.json";
                }
                if (p.http_url.StartsWith("/imageHD"))
                {
                    instance.captureImage(640, 480);
                    url = "out.jpg";
                }
                if (p.http_url.StartsWith("/imageSD"))
                {
                    instance.captureImage(240, 180);
                    url = "out.jpg";
                }
                if (url == "") url = "index.html";
                if (File.Exists(url))
                {
                    p.writeSuccess(getContentType(url));
                    Stream fs = File.Open(url, FileMode.Open);
                    fs.CopyTo(p.outputStream.BaseStream);
                    p.outputStream.BaseStream.Flush();
                    fs.Close();
                }
                else
                {
                    p.writeFailure();
                }
            }catch(Exception e) { instance.log(e.ToString()); }
        }

        public override void handlePOSTRequest(HttpProcessor p, StreamReader inputData)
        {
            Console.WriteLine("POST request: {0}", p.http_url);
            string data = inputData.ReadToEnd();

            p.writeSuccess();
            p.outputStream.WriteLine("<html><body><h1>test server</h1>");
            p.outputStream.WriteLine("<a href=/test>return</a><p>");
            p.outputStream.WriteLine("postbody: <pre>{0}</pre>", data);


        }

        public static string getContentType(string fileName)
        {
            string mimeType = "application/octet-stream";
            string ext = Path.GetExtension(fileName).ToLower();
            RegistryKey regKey = Registry.ClassesRoot.OpenSubKey(ext);
            if (regKey != null && regKey.GetValue("Content Type") != null)
            {
                mimeType = regKey.GetValue("Content Type").ToString();
            }
            return mimeType;
        }
    }
}