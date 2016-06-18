using System;
using System.Collections.Generic;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using System.Net;
using System.IO;
using MyHttp;

namespace Daemon
{
    public partial class Form1 : Form
    {
        public List<AnySensor> sensors;
        public List<Actuator> actuators;
        private Thread refreshSensorListThread;
        private Thread httpThread;
        private Thread autoThread;
        private Thread[] refreshSensorValueThread;
        private Camera.PCCamera pcc;
        private Speech speech;
        List<Word> words = new List<Word>();
        MyHttpServer httpserver;

        public bool doorlock = false;
        public bool dooropen = false;
        public bool lighton = false;
        public bool jsqon = false;
        public bool curtainopen = false;
        public string lastcard = "";
        public DateTime dooraction = DateTime.Now;

        public Form1()
        {
            InitializeComponent();
            speech = new Speech(this);
            pcc = new Camera.PCCamera(this.pictureBox1.Handle, 0, 0, 320, 240);
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            CheckForIllegalCrossThreadCalls = false;
            try
            {
                log("正在初始化");
                httpserver = new MyHttpServer(233,this);
                httpThread = new Thread(new ThreadStart(httpserver.listen));
                log("HTTP服务器在233端口启动");
                httpThread.Start();
            }
            catch (Exception) { }
            new Thread(Init).Start();
        }

        private void Init()
        {
            sensors = new List<AnySensor>();
            actuators = new List<Actuator>();
            AddSensors();
            refreshSensorListThread = new Thread(RefreshSensorList);
            refreshSensorListThread.Start();
            refreshSensorValueThread = new Thread[sensors.Count];
            for (int i =0;i<sensors.Count;i++)
            {
                if (sensors[i].querytype != AnySensor.QueryType.自动||sensors[i].connected==false) continue;
                refreshSensorValueThread[i] = new Thread(new ParameterizedThreadStart(RefreshSensorValue));
                refreshSensorValueThread[i].Start(i);
            }
            control("switch", "1");
            control("switch", "2");
            control("curtain", "1200");
            control("water", "600");
            control("door", "850");
            dodoor(false);
            autoThread = new Thread(AutoControl);
            autoThread.Start();
            speech.speak("智能家居系统已经启动！");
            log("初始化完成");
        }

        public void dodoor(bool door)
        {
            dooropen = door;
            dooraction = DateTime.Now;
            if (!door) control("door", "800");
            else control("door", "1200");
        }

        private void AutoControl()
        {
            //speech.speak("开始自动控制。");
            log("自动控制已启动");
            while (true)
            {
                try
                {
                    try {
                        var card = (string)getvalue("dkq");
                        if (card != lastcard&&lastcard!="")
                        {
                            speech.speak("刷卡成功，解锁、开门。");
                            dodoor(true);
                            doorlock = false;
                        }
                        lastcard = card;
                    }
                    catch (Exception) { }
                    try
                    {
                        var human = (bool)getvalue("rsd");
                        if(!doorlock&&DateTime.Now-dooraction>new TimeSpan(0, 0, 10))
                        {
                            if (human && !dooropen)
                            {
                                dodoor(true);
                                speech.speak("门口有人，自动开门。");
                            }
                            if (!human && dooropen)
                            {
                                dodoor(false);
                                speech.speak("门口没人，自动关门。");
                            }
                        }
                    }
                    catch (Exception) { }
                    try
                    {
                        var lightin = (int)getvalue("gm1");
                        var lightout = (int)getvalue("gm2");

                    }
                    catch (Exception) { }
                    Thread.Sleep(200);
                }catch(Exception e)
                {
                    log("自动控制错误：" + e.ToString());
                }
            }
        }

        private void RefreshSensorValue(object obj)
        {
            var sensor = sensors[(int)obj];
            log("开始刷新"+sensor.name+"数值");
            while (true)
            {
                var start = DateTime.Now;
                if (sensor.querytype == AnySensor.QueryType.自动 && sensor.queryswitch == true) {
                    try
                    {
                        //log(sensor.name);
                        var vec = new List<object>();
                        foreach (var key in sensor.valuename)
                        {
                            var str = HttpGet("http://tdxls-iot.xicp.net/values?n=" + sensor.nodename + "&k=" + key);
                            str = str.Split(' ')[0];
                            str = str.Replace(".00", "");
                            int i;
                            bool b;
                            if (key == "rfid") vec.Add(str);
                            else if (int.TryParse(str, out i)) vec.Add(i);
                            else if (bool.TryParse(str, out b)) vec.Add(b);
                            else vec.Add(str);
                        }
                        sensor.add(vec);
                        sensor.lastquery = DateTime.Now;
                        sensor.status = true;
                    }
                    catch (Exception e)
                    {
                        sensor.status = false;
                        log("刷新" + sensor.name + "错误：" + e.ToString());
                    }
                }
                var end = DateTime.Now;
                if(end-start<sensor.queryinterval)
                    try { Thread.Sleep(sensor.queryinterval-(end - start)); } catch (Exception) { }
            }
        }

        private void RefreshSensorList()
        {
            while (true)
            {
                try
                {
                    for (int i = 0; i < sensors.Count; i++)
                    {
                        var li = listView1.Items[i];
                        var sensor = sensors[i];
                        li.ImageIndex = (sensor.status == true ? 1 : 0);
                        if (sensor.hLen > 0) li.SubItems[2].Text = sensor.getValue().ToString();
                        if (sensor.connected == false) li.SubItems[2].Text = "未初始化";
                        if (sensor.lastquery > DateTime.MinValue) li.SubItems[4].Text = TimeAgo(sensor.lastquery);
                    }
                    Thread.Sleep(100);
                }
                catch (Exception) { }
            }
        }

        private void AddSensors()
        {
            //在这里添加需要的传感器
            AddAutoSensor(new SR501() { name = "热释电", nodename = "i_14551367", id = "rsd" }, 300);
            AddAutoSensor(new PN532() { name = "读卡器", nodename = "i_14551454", id = "dkq" }, 400);
            AddAutoSensor(new ANALOG() { name = "土壤湿度", nodename = "i_10536691", id = "trsd" }, 1000);
            AddAutoSensor(new TSL2561() { name = "光敏1", nodename = "i_14550871", id = "gm1" }, 500);
            AddAutoSensor(new TSL2561() { name = "光敏2", nodename = "i_14550919", id = "gm2" }, 500);
            AddAutoSensor(new DHT11() { name = "温湿度", nodename = "i_14551199", id = "wsd" }, 1000);
            AddActuator("房门", "i_10537003", "PWM","door", _min: 800, _max: 1200, _def: 1200);
            AddActuator("窗帘", "i_14549807", "PWM","curtain", _min:700, _max:1200, _def:1200);
            AddActuator("浇水", "i_10536824", "PWM","water",_min:600, _max:1200, _def:600);
            AddActuator("电器开关", "i_14550878", "switch","switch",new string[] { "TSL2561", "SR501" }, new string[] { "灯","加湿器" });
        }

        private Actuator AddActuator(string _display_name,string _node_name,string _actuator_name,string _id,string[] _pic=null,string[] _dname=null,int _min=300,int _max=1200,int _def=300)
        {
            log("添加执行器：" + _display_name + " " + _node_name+" "+_actuator_name);
            listBox2.Items.Add(_display_name + " " + _node_name + " " + _actuator_name);
            var act = new Actuator() { display_name = _display_name, node_name = _node_name, actuator_name = _actuator_name, pic = _pic, dname = _dname, id = _id, min = _min, max = _max, def = _def };
            actuators.Add(act);
            return act;
        }

        private void AddAutoSensor(AnySensor sensor, int interval)
        {
            CheckSensor(sensor);
            sensor.querytype = AnySensor.QueryType.自动;
            sensor.queryinterval = new TimeSpan(0,0,0,0,interval);
            AddToSensorList(sensor);
            log(string.Format("添加自动传感器 {0} node:{1}", sensor.name, sensor.nodename));
        }

        private bool CheckSensor(AnySensor sensor)
        {
            try {
                var str = HttpGet("http://tdxls-iot.xicp.net/capability?n=" + sensor.nodename + "&type=values");
                var lout = "传感器" + sensor + "的返回内容：";
                foreach (var line in str.Split(new char[] { '\n' }, StringSplitOptions.RemoveEmptyEntries))
                {
                    var name = line.Replace(" ","").Split(',')[0];
                    //name = name.Remove(name.Length - 1);
                    sensor.valuename.Add(name);
                    lout += name + " ";
                }
                sensor.connected = true;
                log(lout);
            }
            catch (Exception e)
            {
                log(e.ToString());
                return false;
            }
            return true;
        }

        private void AddToSensorList(AnySensor sensor)
        {
            sensors.Add(sensor);
            var li = new ListViewItem();
            li.SubItems.Add(sensor.name);
            li.SubItems.Add("");
            li.SubItems.Add(sensor.querytype.ToString());
            li.SubItems.Add("从未");
            li.ImageIndex = 0;
            listView1.Items.Add(li);
        }

        public object getvalue(string id)
        {
            var sensor = sensors.Where(s => s.id == id).First();
            if (sensor.status == false) throw new Exception();
            return sensor.getValue();
        }

        public void control(string id,string ctrl_param)
        {
            control(actuators.Where(a => a.id == id).First(), ctrl_param);
        }

        public void control(Actuator act,string ctrl_param)
        {
            new Thread(new ParameterizedThreadStart(doControl)).Start(new Tuple<Actuator, string>(act, ctrl_param));
        }

        private void doControl(object _param)
        {
            var param = (Tuple<Actuator, string>)_param;
            var act = param.Item1;
            var ctrl_param = param.Item2;
            try
            {
                HttpGet("http://tdxls-iot.xicp.net/control?n=" + act.node_name + "&k=" + act.actuator_name + "&data=" + ctrl_param);
            }catch(Exception e)
            {
                log("执行器错误：" + e.ToString());
            }
        }

        public void log(string str)
        {
            if (listBox1.Items.Count > 30) listBox1.Items.RemoveAt(listBox1.Items.Count - 1);
            listBox1.Items.Insert(0, DateTime.Now.ToString("HH:mm:ss") + " " + str);
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            try
            {
                refreshSensorListThread.Abort();
                autoThread.Abort();
                foreach (var thread in refreshSensorValueThread)
                {
                    if (thread != null) try { thread.Abort(); } catch (Exception) { }
                }
            }
            catch (Exception){ }
            httpThread.Abort();
            Application.Exit();
        }
        public static string TimeAgo(DateTime dt)
        {
            TimeSpan span = DateTime.Now - dt;
            return span.Milliseconds.ToString();
        }
        public static string GetResponse(ref HttpWebRequest req, bool GetLocation = false, bool GetRange = false, bool NeedResponse = true)
        {
            HttpWebResponse res = null;
            try
            {
                res = (HttpWebResponse)req.GetResponse();
            }
            catch (WebException e)
            {
                StreamReader ereader = new StreamReader(e.Response.GetResponseStream(), Encoding.GetEncoding("utf-8"));
                string erespHTML = ereader.ReadToEnd();
                Console.WriteLine(erespHTML);
                throw new Exception(erespHTML);
            }
            if (GetLocation)
            {
                string ts = res.Headers["Location"];
                res.Close();
                Console.WriteLine("Location: " + ts);
                return ts;
            }
            if (GetRange && res.ContentLength == 0)
            {
                string ts = res.Headers["Range"];
                Console.WriteLine("Range: " + ts);
                return ts;
            }
            if (NeedResponse)
            {
                StreamReader reader = new StreamReader(res.GetResponseStream(), Encoding.GetEncoding("utf-8"));
                string respHTML = reader.ReadToEnd();
                res.Close();
                //Console.WriteLine(respHTML);
                return respHTML;
            }
            else
            {
                res.Close();
                return "";
            }
        }
        public static HttpWebRequest GenerateRequest(string URL, string Method, string token, bool KeepAlive = false, string ContentType = null, byte[] data = null, int offset = 0, int length = 0, string ContentRange = null, bool PreferAsync = false, int Timeout = 20 * 1000, string host = null)
        {
            Uri httpUrl = new Uri(URL);
            HttpWebRequest req = (HttpWebRequest)WebRequest.Create(httpUrl);
            req.ProtocolVersion = new System.Version("1.0");
            req.Timeout = Timeout;
            req.ReadWriteTimeout = Timeout;
            req.Method = Method;
            if (token != null) req.Headers.Add("Authorization", "Bearer " + token);
            if (host != null) req.Host = host;
            req.KeepAlive = KeepAlive;
            if (ContentType != null) req.ContentType = ContentType;
            if (ContentRange != null) req.Headers.Add("Content-Range", ContentRange);
            if (PreferAsync == true) req.Headers.Add("Prefer", "respond-async");
            if (data != null)
            {
                req.ContentLength = length;
                Stream stream = req.GetRequestStream();
                stream.Write(data, offset, length);
                stream.Close();
            }
            return req;
        }
        public static string HttpGet(string URL, string token=null, bool GetLocation = false, bool AllowAutoRedirect = true, bool NeedResponse = true, int Timeout = 5 * 1000, string host = null)
        {
            HttpWebRequest req = GenerateRequest(URL, "GET", token, false, null, null, 0, 0, null, false, Timeout, host);
            if (AllowAutoRedirect == false) req.AllowAutoRedirect = false;
            return GetResponse(ref req, GetLocation, false, NeedResponse);
        }

        public string captureImage(int width,int height)
        {
            //string fn = DateTime.Now.ToString("yyyyMMddHHmmss")+"temp.jpg";
            string fn = "temp.jpg";
            pcc.GrabImage(fn);
            var img = Image.FromFile(fn);
            new Bitmap(img, new Size(width, height)).Save("out.jpg", System.Drawing.Imaging.ImageFormat.Jpeg);
            img.Dispose();
            return fn;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            pcc.Start();
            log("摄像头打开");
        }

        private void button2_Click(object sender, EventArgs e)
        {
            pcc.Stop();
            log("摄像头关闭");
        }

        private void button3_Click(object sender, EventArgs e)
        {
            captureImage(320, 240);
        }

        private void button4_Click(object sender, EventArgs e)
        {
            pcc.Kinescope(System.DateTime.Now.ToString("yyyyMMddHHmmss") + ".avi");
        }

        private void button5_Click(object sender, EventArgs e)
        {
            Application.DoEvents();
            pcc.StopKinescope();
        }

        private void button6_Click(object sender, EventArgs e)
        {
            speech.speak(textBox1.Text);
        }
        
    }
}
