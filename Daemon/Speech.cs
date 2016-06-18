using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.IO;
using System.Speech.Recognition;
using System.Speech.Synthesis;

namespace Daemon
{
    class Speech
    {
        SpeechRecognitionEngine speechRecognitionEngine = null;
        List<Word> words = new List<Word>();
        Form1 instance;
        public Speech(Form1 instance)
        {
            this.instance = instance;
            try
            {
                speechRecognitionEngine = createSpeechEngine("zh-CN");
                speechRecognitionEngine.AudioLevelUpdated += new EventHandler<AudioLevelUpdatedEventArgs>(engine_AudioLevelUpdated);
                speechRecognitionEngine.SpeechRecognized += new EventHandler<SpeechRecognizedEventArgs>(engine_SpeechRecognized);
                loadGrammarAndCommands();
                speechRecognitionEngine.SetInputToDefaultAudioDevice();
                speechRecognitionEngine.RecognizeAsync(RecognizeMode.Multiple);
            }
            catch (Exception ex)
            {
                instance.log("Voice recognition load failed: "+ex.ToString());
            }
        }
        ~Speech()
        {
            speechRecognitionEngine.RecognizeAsyncStop();
            speechRecognitionEngine.Dispose();
        }

        private SpeechRecognitionEngine createSpeechEngine(string preferredCulture)
        {
            foreach (RecognizerInfo config in SpeechRecognitionEngine.InstalledRecognizers())
            {
                if (config.Culture.ToString() == preferredCulture)
                {
                    speechRecognitionEngine = new SpeechRecognitionEngine(config);
                    break;
                }
            }
            if (speechRecognitionEngine == null)
            {
                instance.log("The desired culture is not installed on this machine, the speech-engine will continue using "
                    + SpeechRecognitionEngine.InstalledRecognizers()[0].Culture.ToString() + " as the default culture. "+
                    "Culture " + preferredCulture + " not found!");
                speechRecognitionEngine = new SpeechRecognitionEngine(SpeechRecognitionEngine.InstalledRecognizers()[0]);
            }
            return speechRecognitionEngine;
        }
        private void loadGrammarAndCommands()
        {
            try
            {
                Choices texts = new Choices();
                string[] lines = File.ReadAllLines("example.txt");
                foreach (string line in lines)
                {
                    if (line.StartsWith("--") || line == String.Empty) continue;
                    var parts = line.Split(new char[] { '|' });
                    words.Add(new Word() { Text = parts[0], AttachedText = parts[1], IsShellCommand = (parts[2] == "true") });
                    texts.Add(parts[0]);
                }
                Grammar wordsList = new Grammar(new GrammarBuilder(texts));
                speechRecognitionEngine.LoadGrammar(wordsList);
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }
        void engine_SpeechRecognized(object sender, SpeechRecognizedEventArgs e)
        {
            speechRecognitionEngine.RecognizeAsyncStop();
            Word cmd = null;
            var tosay = "";
            try
            {
                cmd = words.Where(c => c.Text == e.Result.Text).First();

                if (cmd.IsShellCommand)
                {
                    switch (cmd.AttachedText)
                    {
                        case "lighton":
                            if (instance.lighton) break;
                            instance.control("switch", "-1");
                            instance.lighton = true;
                            speak("好的，我来为你开灯。");
                            break;

                        case "lightoff":
                            if (!instance.lighton) break;
                            speak("好的，灯将会关上。");
                            instance.control("switch", "1");
                            instance.lighton = false;
                            break;
                        case "jsqon":
                            if (instance.jsqon) break;
                            speak("好的，打开加湿器。");
                            instance.control("switch", "-2");
                            instance.jsqon = true;
                            break;

                        case "jsqoff":
                            if (!instance.jsqon) break;
                            speak("好的，关上加湿器。");
                            instance.control("switch", "2");
                            instance.jsqon = false;
                            break;

                        case "curtainopen":
                            if (instance.curtainopen) break;
                            speak("好的，打开窗帘。");
                            instance.control("curtain", "700");
                            instance.curtainopen = true;
                            break;

                        case "curtainclose":
                            if (!instance.curtainopen) break;
                            speak("好的，关上窗帘。");
                            instance.control("curtain", "1200");
                            instance.curtainopen = false;
                            break;

                        case "water":
                            speak("好的，开始浇水。");
                            instance.control("water", "1200");
                            Thread.Sleep(6000);
                            instance.control("water", "600");

                            break;

                        case "doorlock":
                            instance.doorlock = true;
                            instance.dodoor(false);
                            speak("门已经上锁，请刷卡开锁。");
                            break;

                        case "dooropen":
                            if (instance.doorlock) { speak("门已经上锁，请刷卡开锁。"); break; }
                            if (instance.dooropen) break;
                            speak("好的，打开门。");
                            instance.dodoor(true);
                            break;

                        case "doorclose":
                            if (!instance.dooropen) break;
                            speak("好的，关上门。");
                            instance.dodoor(false);
                            break;

                        case "gettemp":
                            try
                            {
                                var temp = (Tuple<int, int>)instance.getvalue("wsd");
                                speak("现在室内温度" + temp.Item1 + "度，湿度百分之" + temp.Item2);
                            }
                            catch (Exception)
                            {
                                speak("出了一点问题，暂时不知道数值。");
                            }
                            break;
                    }
                }
                else
                {
                    tosay = cmd.AttachedText;
                }
            }
            catch (Exception ex) { instance.log(ex.ToString()); }
            instance.txtSpoken.Text = tosay;
            speak(tosay);
            speechRecognitionEngine.RecognizeAsync(RecognizeMode.Multiple);
        }
        void engine_AudioLevelUpdated(object sender, AudioLevelUpdatedEventArgs e)
        {
            instance.prgLevel.Value = e.AudioLevel;
        }

        public void speak(string text)
        {
            new Thread(new ParameterizedThreadStart(doSpeak)).Start(text);
        }

        void doSpeak(object text)
        {
            using (var speak = new SpeechSynthesizer())
                speak.Speak((string)text);
        }
    }
    public class Word
    {
        public Word() { }
        public string Text { get; set; }
        public string AttachedText { get; set; }
        public bool IsShellCommand { get; set; }
    }
}
