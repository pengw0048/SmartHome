using System;
using System.Collections.Generic;

public class Actuator
{
    public string node_name;
    public string display_name;
    public string actuator_name;
    public string ctrl_type;
    public string[] pic, dname;
    public string id;
    public int min, max,def;
}
public class AnySensor
{
    public bool status = false;
    public string name = "";
    public string nodename = "";
    public string id = "";
    public enum QueryType
    {
        自动, 手动
    }
    public QueryType querytype = 0;
    public bool queryswitch = true;
    public TimeSpan queryinterval;
    public DateTime lastquery = DateTime.MinValue;
    public int hLast = -1, hLen = 0;
    public virtual object getValue() { return null; }
    public virtual void add(List<object> values) { }
    public List<string> valuename = new List<string>();
    public bool connected = false;
    public string type = "any";
}
public class Sensor<T> : AnySensor
{
    public T[] history = new T[10];
    public void add(T value)
    {
        hLast = (hLast + 1) % history.Length;
        history[hLast] = value;
        if (hLen < history.Length) hLen++;
    }
    public T value { get { if (hLast < 0) throw new Exception("还没有数值。"); return history[hLast]; } }
    public T historyvalue(int i)
    {
        if (i + 1 > hLen) throw new ArgumentOutOfRangeException("没有那么多记录，只有" + hLen + "个");
        if (i < 0) throw new ArgumentOutOfRangeException();
        return history[(history.Length - i) % history.Length];
    }
    public class SensorEventArgs<T1> : EventArgs
    {
        public string nodename { get; }
        public string id { get; }
        public T1 value { get; }
    }
    public event EventHandler<SensorEventArgs<T>> SensorEvent;
    protected virtual void OnSensorEevent()
    {
        var handler = this.SensorEvent;
        if (handler != null)
        {
            handler(this, null);
        }
    }
    public override object getValue()
    {
        return value;
    }
}
public class DHT11 : Sensor<Tuple<int, int>>
{
    public DHT11()
    {
        type = "DHT11";
    }
    public override void add(List<object> values)
    {
        add(new Tuple<int, int>((int)values[0], (int)values[1]));
    }
}
public class PN532 : Sensor<string>
{
    public PN532()
    {
        type = "PN532";
    }
    public override void add(List<object> values)
    {
        add((string)values[0]);
    }
}
public class TSL2561 : Sensor<int>
{
    public TSL2561()
    {
        type = "TSL2561";
    }
    public override void add(List<object> values)
    {
        add((int)values[0]);
    }
}
public class ANALOG : Sensor<int>
{
    public ANALOG()
    {
        type = "ANALOG";
    }
    public override void add(List<object> values)
    {
        add((int)values[0]);
    }
}
public class SR501 : Sensor<bool>
{
    public SR501()
    {
        type = "SR501";
    }
    public override void add(List<object> values)
    {
        add((bool)values[0]);
    }
}