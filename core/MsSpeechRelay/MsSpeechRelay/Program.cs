﻿/*
   Part of SBM: SmartBody Module
   Copyright (C) 2009  University of Southern California

   SBM is free software: you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public License
   as published by the Free Software Foundation, version 3 of the
   license.

   SBM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   Lesser GNU General Public License for more details.

   You should have received a copy of the Lesser GNU General Public
   License along with SBM.  If not, see:
       http://www.gnu.org/licenses/lgpl-3.0.txt

   CONTRIBUTORS:
      Abhinav Golas, USC
      Arno Hartholt, USC
      Edward Fast, USC
*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Speech.Synthesis;
using System.Xml;
using System.IO;
using System.Threading;

/**
 * MsSpeech Relay
 * Uses the Microsoft SAPI to generate audio for SmartBody
 * Written using SAPI 5.3 Managed C# API
 */
namespace MsSpeechRelay
{
    /// <summary>
    /// Main program
    /// </summary>
    class Program
    {
        /// <summary>
        ///Timer variable
        /// </summary>
        int timer_millisec;
        /// <summary>
        /// Message handling server
        /// </summary>
        public VHMsg.Client vhmsg;
        /// <summary>
        /// Identifies this program on the message system and to other components in the VH toolkit
        /// </summary>
        public string programName;
        private bool isInitialized = false;
        private bool isRunning = true;
        /// <summary>
        /// Variable to enable/disable debug prints
        /// </summary>
        private bool doDebugChecks = true;
        /// <summary>
        /// Maps SAPI visemes to SmartBody phonemes in order to generate viseme timings
        /// </summary>
        private List<string> visemeIDMap;
        /// <summary>
        /// Where the audio will be written to
        /// </summary>
        /// Changed by Shridhar on 10/09/09
        /// This is a temporary default path. The actual path for the soud files is received from SmartBody
        private string temporaryAudioStoragePath = "..\\..\\data\\cache\\audio\\";
        /// <summary>
        /// Where the audio clip generated by this program will be stored as relative to the rendering engine
        /// </summary>
        private string temporaryAudioPlayerPath = "data\\cache\\audio\\";
        /// <summary>
        /// SAPI TTS server
        /// </summary>
        private SpeechSynthesizer ttsServer;
        /// <summary>
        /// Stores the xml portion of the reply, filled in by various callbacks of the SAPI TTS server
        /// </summary>
        private string xmlReply = "";
        /// <summary>
        /// Denotes the handling of voice requests, 0 for permissive, 1 for strict, 2 for ignore
        /// </summary>
        private int voiceRequestMode = 0;

        private Dictionary<string, string> voiceMap = null;

        /// <summary>
        /// No constructor level initialization needed
        /// </summary>
        public Program()
        {
        }

        /// <summary>
        /// Initialize all phoneme to viseme mappings
        /// </summary>
        private void pvMapInit()
        {
            visemeIDMap = new List<string>();
            /// Map constructed from viseme reference:
            /// http://msdn.microsoft.com/en-us/library/ms720881(VS.85).aspx
            /// 
            visemeIDMap.Insert(0, "_");
            /// Viseme for aa, ae, ah
            visemeIDMap.Insert(1, "Ih");
            /// Viseme for aa
            visemeIDMap.Insert(2, "Ao");
            /// ao
            visemeIDMap.Insert(3, "Ao");
            /// ey, eh, uh
            visemeIDMap.Insert(4, "Ih");
            /// er
            visemeIDMap.Insert(5, "Er");
            /// y, iy, ih, ix
            visemeIDMap.Insert(6, "EE"); // also try OO, Ih
            /// w, uw
            visemeIDMap.Insert(7, "Oh"); // also try OO
            /// ow
            visemeIDMap.Insert(8, "Oh");
            /// aw
            visemeIDMap.Insert(9, "Ih");
            /// oy
            visemeIDMap.Insert(10, "Oh");
            /// ay
            visemeIDMap.Insert(11, "Ih");
            /// h
            visemeIDMap.Insert(12, "Oh");
            /// r
            visemeIDMap.Insert(13, "R");
            /// l
            visemeIDMap.Insert(14, "D");
            /// s, z
            visemeIDMap.Insert(15, "Z");
            /// sh, ch, jh, zh
            visemeIDMap.Insert(16, "J");
            /// th, dh
            visemeIDMap.Insert(17, "Th");
            /// f, v
            visemeIDMap.Insert(18, "F");
            /// d, t, n
            visemeIDMap.Insert(19, "D"); // also try NG: 2 to 1 against
            /// k, g, ,ng
            visemeIDMap.Insert(20, "KG"); // also try NG: 2 to 1 against
            /// p, b, m
            visemeIDMap.Insert(21, "BMP");

        }

        /// <summary>
        /// Generate TTS audio, takes SSML input
        /// </summary>
        /// <param name="message">SSML message</param>
        /// <param name="outputFileName">File name to save output in</param>
        /// <param name="voice">Voice to use</param>
        /// <returns></returns>
        private bool generateAudio(string message, string outputFileName, string messageOutputFileName, string voice)
        {
            if (doDebugChecks)
            {
                Console.WriteLine("Generating audio for message with voice: " + voice);
            }
            bool allOk = true;
            xmlReply = "<speak><soundFile name=\"" + messageOutputFileName + "\"/>";

            ttsServer.SetOutputToWaveFile(outputFileName);
            if (voiceRequestMode < 2 && voiceMap != null && voiceMap.Count > 0)
            {
                /// Voice request mode is either permissive or strict
                /// 
                if (voiceMap.ContainsKey(voice))
                {
                    Console.WriteLine("Selecting SAPI voice: " + voiceMap[voice] + " for VHuman voice: " + voice);
                    try
                    {
                        ttsServer.SelectVoice(voiceMap[voice]);
                    }
                    catch (ArgumentException e)
                    {
                        Console.WriteLine("Exception while choosing voice: " + e.ToString());
                    }
                }
                else
                {
                    Console.WriteLine("Error: Requested voice not found!");
                    if (voiceRequestMode == 1)
                    {
                        allOk = false;
                    }
                }
            }
            else
            {
                voiceRequestMode = 2;
                Console.WriteLine("Warning: Defaulting to ignore voice map request, since no voice maps are specified");
            }

            /// We have a pre-tweaked message, no need to tamper with it
            /// 
            if (doDebugChecks)
            {
                Console.WriteLine("Debug: Generating speech for SSML string: \"" + message + "\"...\n");
            }
            if (allOk)
            {
                ttsServer.SpeakSsml(message);
            }
            xmlReply += "</speak>";
            return allOk;
        }

        /// <summary>
        /// Process Speech Message and send reply
        /// </summary>
        /// <param name="message">The message</param>
        private void ProcessSpeechMessage(string message)
        {
            Console.WriteLine("Processing message.\n");
            char[] myDelims = { ' ', '\n', '\r', '\t' };
            string[] splitargs = message.Split(myDelims);

            if (splitargs[0] != "RemoteSpeechCmd")
            {
                Console.WriteLine("Error: Speech callback called for spurious command message: " + message + " , returning");
                return;
            }

            string actor = splitargs[2];
            string id = splitargs[3];
            string voice = splitargs[4];

            /// Trim path from filename, and add a .wav to it, so that a relative path can be generated
            string fileName = splitargs[5];

            // getting the path name (excluding filename) and also converting it to an absolute path
            temporaryAudioStoragePath = fileName.Substring(0, fileName.LastIndexOf('/')+1);
            temporaryAudioPlayerPath = System.IO.Path.GetFullPath(temporaryAudioStoragePath);

            // create directory if it doesn't exist
            if (!Directory.Exists(temporaryAudioStoragePath))
                Directory.CreateDirectory(temporaryAudioStoragePath);

            int trimPosition = fileName.LastIndexOf('/') + 1;
            fileName = fileName.Substring(trimPosition, fileName.LastIndexOf('.') - trimPosition) + ".wav";
            string outputFileName = temporaryAudioStoragePath + fileName;
            string outputPlayerFilename = temporaryAudioPlayerPath + fileName;

            /// Get XML portion of message
            /// 
            string xmlMessageString = message.Substring(message.IndexOf('<'));
            /// Remove spurious whitespaces from xml message, since it is a multi-line message
            xmlMessageString = xmlMessageString.Replace('\n', ' ');
            xmlMessageString = xmlMessageString.Replace('\t', ' ');
            xmlMessageString = xmlMessageString.Replace('\r', ' ');

            XmlDocument xmlMessage = new XmlDocument();
            xmlMessage.InnerXml = xmlMessageString;
            XmlElement root = xmlMessage.DocumentElement;

            /// We make the message such that it only contains the speech XML element
            XmlElement speechPart = root;
            string spID = speechPart.GetAttribute("id");

            /// Currently the SSML messages generated by SmartBody do not fully correspond to the SSML spec, so we need to make minor changes to make it comply
            speechPart.RemoveAttribute("id");
            speechPart.RemoveAttribute("type");
            speechPart.RemoveAttribute("ref");
            speechPart.SetAttribute("version", "1.0");
            speechPart.SetAttribute("xml:lang", "en-US");

            /// We want to rename all tags T0, T1, ... as spID:T0...
            /// 
            XmlNodeList marks = speechPart.GetElementsByTagName("mark");
            foreach (XmlNode n in marks)
            {
                XmlElement elem = (XmlElement)n;
                elem.SetAttribute("name", spID + ":" + elem.GetAttribute("name"));
            }

            /// Done, now generate audio
            /// 
            bool allOk = generateAudio(speechPart.OuterXml.Replace("speech","speak"), outputFileName, outputPlayerFilename, voice);

            /// Now, send back message to whoever wanted the audio
            /// 
            string replyMessage = "RemoteSpeechReply " + actor + " " + id;
            if (allOk)
            {
                replyMessage += " OK: <?xml version=\"1.0\" encoding=\"UTF-8\"?>" + xmlReply;
            }
            else
            {
                replyMessage += " ERROR: Unable to choose suggested voice: \"" + voice + "\" and strict mode used";
            }

            vhmsg.SendMessage(replyMessage);
            if (doDebugChecks)
            {
                Console.WriteLine("Debug: Sending reply: \"" + replyMessage + "\"\n");
            }
        }

        /// <summary>
        /// Message Handling Code:
        /// Callback for every message, will direct program flow
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e">Variable containg the message params and the actual message</param>
        private void MessageHandler(object sender, VHMsg.Message e)
        {
            if (!isInitialized)
            {
                /// We can't possibly be here!
                Console.WriteLine("Warning: Message Handler started without intialization, performing relevant initializations.\n");
                InitializeAndRun(new List<string>());
            }
            if (e == null)
                return;
            string[] splitargs = e.s.Split(" ".ToCharArray());

            if (splitargs[0] == "adc" || splitargs[0] == "wsp")
            {
                /// Ignore them
            }
            else if (splitargs[0] == "vrAllCall")
            {
                /// Answer the roll call
                vhmsg.SendMessage("vrComponent tts " + programName);
            }
            else if (splitargs[0] == "vrKillComponent")
            {
                /// Controls when the programs kills itself
                //if (splitargs.Length > 1 && ( splitargs[1] == programName || splitargs[1] == "all" || splitargs[1] == "\"all\""))
                // Changed by Shridhar on 09/29/09..... Elvin name should be tts and program name should be MSSpeechRelay
                // Updating to meet Elvin Message specifications on Wiki
                if (splitargs.Length > 1 && (splitargs[1] == "tts" || splitargs[1] == "all" || splitargs[1] == "\"all\""))
                {
                    Console.WriteLine("Kill message received, goodbye\n");
                    vhmsg.SendMessage("vrProcEnd tts " + programName);
                    /// Not running anymore
                    /// 
                    isRunning = false;
                    //CleanUp();
                    //System.Windows.Forms.Application.Exit();
                }
            }
            else if (splitargs[0] == "RemoteSpeechCmd")
            {
                if (doDebugChecks)
                {
                    Console.WriteLine("Received speech command: " + e.s + "\n");
                }
                ProcessSpeechMessage(e.s);
            }
            else
            {
                /// This fallback command is there because it only registers for very specific messages, so if it receives uncalled for messages this will catch it
                Console.WriteLine("Received unknown command: " + e.s + "\n");
            }
        }

        public void ParseArguments(List<string> args)
        {
            /// Initialize TTS
            /// 
            ttsServer = new SpeechSynthesizer();

            int n = args.Count;
            string voiceHM;
            switch (voiceRequestMode)
            {
                case 0:
                    voiceHM = "permissive";
                    break;
                case 1:
                    voiceHM = "strict";
                    break;
                case 2:
                    voiceHM = "ignore";
                    break;
                default:
                    voiceHM = "unknown - please check program code";
                    break;
            }
            /// Sadly, we need the next argument for some arguments, so we need run the loop using indices
            for (int i = 0; i < n; i++)
            {
                string s = args[i];
                if (s == "-h" || s == "--help")
                {
                    Console.WriteLine("MsSpeechRelay help");
                    Console.WriteLine("Warning: Requires a running ActiveMQ server to run\n");
                    Console.WriteLine("Usage: msspeechrelay <options>");
                    Console.WriteLine("\t-h, --help:\tDisplay this help menu");
                    Console.WriteLine("\t-v, --voicemode:\t Voice request handling (Default: " + voiceHM);
                    Console.WriteLine("\t\tstrict:\tDo not default to a default voice if requested voice is not available (Warning: May not produce any audio if selected voice not available)");
                    Console.WriteLine("\t\tpermissive:\tDefault to a default voice if requested voice is not available");
                    Console.WriteLine("\t\tignore:\tIgnore all voice requests and only use default voice");
                    Console.WriteLine("\t-t, --testfile <filename>:\tRespond to message in file and exit, rather than listen on the ActiveMQ message queue");
                    Console.WriteLine("\t-o, --outputpath <path>:\tWrite output audio to this path (Default: " + temporaryAudioStoragePath + ")");
                    Console.WriteLine("\t-ro, --relativepath <path>:\tOutput path relative to renderer (Default: " + temporaryAudioPlayerPath + ")");
                    Console.WriteLine("\t-m, --map <src> <target>:\tMap voice name <src> to SAPI voice <target>. Available voices on this machine (use without quotes):");
                    foreach (InstalledVoice v in ttsServer.GetInstalledVoices())
                    {
                        Console.WriteLine("\t\t\"" + v.VoiceInfo.Name + "\"");
                    }
                    isRunning = false;
                }
                else if (s == "-v" || s == "--voicemode")
                {
                    if (i + 1 < args.Count)
                    {
                        string mode = args[i + 1];
                        if (mode == "permissive")
                        {
                            voiceRequestMode = 0;
                        }
                        else if (mode == "strict")
                        {
                            voiceRequestMode = 1;
                        }
                        else if (mode == "ignore")
                        {
                            voiceRequestMode = 2;
                        }
                        else
                        {
                            Console.WriteLine("Warning: Invalid voice mode: " + mode + " requested. Defaulting to: " + voiceHM);
                        }
                        i++;
                        Console.WriteLine("Notify: Selecting voice mode: " + voiceHM);
                    }
                    else
                    {
                        Console.WriteLine("Error: No voice mode specified, defaulting to: " + voiceHM + ". See -h/--help for options");
                    }
                }
                else if (s == "-t" || s == "--testfile")
                {
                    /// Just converting the argument so that the code further along the line can simply parse the argument
                    args[i] = "-testfile";
                    i++;
                    Console.WriteLine("Notify: Attempting to use test file");
                }
                else if (s == "-o" || s == "--outputpath")
                {
                    if (i + 1 < args.Count)
                    {
                        string path = args[i + 1];
                        temporaryAudioStoragePath = path;
                        i++;
                        Console.WriteLine("Notify: Audio output path set to: " + path);
                    }
                    else
                    {
                        Console.WriteLine("Error: No path specified, defaulting to: \"" + temporaryAudioStoragePath + "\". See -h/--help for command details");
                    }
                }
                else if (s == "-ro" || s == "--relativepath")
                {
                    if (i + 1 < args.Count)
                    {
                        string path = args[i + 1];
                        temporaryAudioPlayerPath = path;
                        i++;
                        Console.WriteLine("Notify: Renderer Audio source path set to: " + path);
                    }
                    else
                    {
                        Console.WriteLine("Error: No path specified, defaulting to: \"" + temporaryAudioPlayerPath + "\". See -h/--help for command details");
                    }
                }
                else if (s == "-m" || s == "--map")
                {
                    if (i + 2 < args.Count)
                    {
                        string src = args[i + 1], target = args[i + 2];
                        if (voiceMap == null)
                        {
                            voiceMap = new Dictionary<string, string>();
                        }
                        voiceMap.Add(src, target);
                        Console.WriteLine("Notify: VHuman voice: " + src + " mapped to SAPI voice: " + target);
                        i += 2;
                    }
                    else
                    {
                        Console.WriteLine("Error: Insufficient arguments to voice map, need 2 arguments. See -h/--help for command details");
                    }
                }
                else
                {
                    Console.WriteLine("Error: Unknown argument: \"" + s + "\" provided. See -h/--help for command options");
                }
                
            }
        }

        /// <summary>
        /// Initialize the program
        /// </summary>
        /// <param name="args">Add any arguments to the function in this variable, remember the position of the arguments though</param>
        public void InitializeAndRun(List<string> args)
        {
            ParseArguments(args);
            if (!isRunning)
            {
                return;
            }
            /// The proud name of this program
            /// 
            programName = "msspeechrelay";
            //programName = "cerevoicerelay";

            /// Start message server
            /// 
            using (vhmsg = new VHMsg.Client())
            {
                vhmsg.OpenConnection();
                Console.WriteLine( "VHMSG_SERVER: {0}", vhmsg.Server );
                Console.WriteLine( "VHMSG_SCOPE: {0}", vhmsg.Scope );

                /// We only need Remote Speech commands
                vhmsg.SubscribeMessage("RemoteSpeechCmd");
                vhmsg.SubscribeMessage("vrKillComponent");
                vhmsg.SubscribeMessage("vrAllCall");
                //vhmsg.SubscribeMessage("*");

                vhmsg.MessageEvent += new VHMsg.Client.MessageEventHandler(MessageHandler);

                vhmsg.SendMessage("vrComponent tts" + programName );
                isInitialized = true;

                /// If temporary audio directory doesn't exist, create it
                /// 
                if (!System.IO.File.Exists(temporaryAudioStoragePath))
                {
                    /// One command creates the entire path, even if parent directories are not present
                    System.IO.Directory.CreateDirectory(temporaryAudioStoragePath);
                }

                /// Initialize phoneme to viseme mappings
                /// 
                pvMapInit();

                /// Add message callbacks to receive TTS events
                /// 
                ttsServer.BookmarkReached += new EventHandler<BookmarkReachedEventArgs>(ttsServer_BookmarkReached);
                ttsServer.PhonemeReached += new EventHandler<PhonemeReachedEventArgs>(ttsServer_PhonemeReached);
                ttsServer.VisemeReached += new EventHandler<VisemeReachedEventArgs>(ttsServer_VisemeReached);

                /// Debugging support
                /// Parses in a test file containing the same commands as would be received via VHMsg
                /// NOTE: Only expects 1 command per file
                /// 

                Console.WriteLine("MsSpeechRelay: Ready to receive messages");
                if (args.Contains("-testfile"))
                {
                    string debugFile = args[args.IndexOf("-testfile") + 1];
                    System.IO.StreamReader parser = new System.IO.StreamReader(debugFile);
                    string testMessage = parser.ReadToEnd();
                    ProcessSpeechMessage(testMessage);
                    isRunning = false;
                    vhmsg.SendMessage("vrProcEnd tts " + programName);
                }

                // initializing timer value
                timer_millisec = System.DateTime.Now.Millisecond;

                /// Loop to process events
                /// 
                while (isRunning)
                {
                    System.Windows.Forms.Application.DoEvents();


                    // code to limit the CPU usage as otherwise it takes up the complete CPU
                    // Currently limiting to 60 frames per second. Might not me accurately 60 due to granularity issues.
                    int timesincelastframe = System.DateTime.Now.Millisecond - timer_millisec;
                    timer_millisec = System.DateTime.Now.Millisecond;
                    int ttW;
                    ttW = (1000 / 60) - timesincelastframe;
                    if (ttW > 0)
                        Thread.Sleep(ttW); 
                }
            }

        }

        /// <summary>
        /// Viseme callback - maps SAPI visemes to SmartBody visemes/phonemes
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void ttsServer_VisemeReached(object sender, VisemeReachedEventArgs e)
        {
            if (doDebugChecks)
            {
                int vindex = e.Viseme;
                /// This is a safety capture, if either of these is being triggered, some changes have been made to the SAPI API, change the map accordingly
                if (vindex < 0)
                {
                    Console.WriteLine("Viseme index bound by 0: " + vindex.ToString() + "\n");
                    vindex = 0;
                }
                else if (vindex >= visemeIDMap.Count)
                {
                    Console.WriteLine("Viseme index bound by " + visemeIDMap.Count + ": " + vindex.ToString() + "\n");
                    vindex = visemeIDMap.Count - 1;
                }

                if (vindex != e.Viseme) Console.WriteLine("Viseme index truncated from " + e.Viseme.ToString() + " to " + vindex + "\n");
                Console.WriteLine("Reached viseme: " + e.Viseme.ToString() + " aka: " + visemeIDMap[vindex] + " at time: " + e.AudioPosition.TotalSeconds.ToString() + " for duration: " + e.Duration.ToString() + "\n");
            }

            /// This is where the magic happens
            xmlReply += "<viseme start=\"" + e.AudioPosition.TotalSeconds.ToString() + "\" type=\"" + visemeIDMap[e.Viseme] + "\"/>";
        }

        /// <summary>
        /// Phoneme reached callback - doesn't do anything
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void ttsServer_PhonemeReached(object sender, PhonemeReachedEventArgs e)
        {
            if (doDebugChecks)
            {
                Console.WriteLine("Reached phoneme: " + e.Phoneme + " at time: " + e.AudioPosition.TotalSeconds.ToString() + " for duration: " + e.Duration.ToString() + "\n");
                //byte[] b = System.Text.Encoding.Unicode.GetBytes(e.Phoneme.ToCharArray());
                //Console.WriteLine("Chars for bytes: ");
                //Console.WriteLine(System.Text.Encoding.ASCII.GetChars(b));
                //string asciiPhoneme = System.Text.Encoding.ASCII.GetString(System.Text.Encoding.Convert(Encoding.Unicode,Encoding.ASCII,b));
                //Console.WriteLine("Test ascii phoneme: " + asciiPhoneme + "\n");
                //Console.WriteLine("Viseme attached= " + phonemeToViseme[phoneme] + "\n");
            }
        }

        /// <summary>
        /// Bookmark reached callback - used to mark word beginnings and ends
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void ttsServer_BookmarkReached(object sender, BookmarkReachedEventArgs e)
        {
            String bookmark;
            bookmark = e.Bookmark.Substring(e.Bookmark.IndexOf(':') + 1);

            if (doDebugChecks)
            {
                Console.WriteLine("Reached bookmark: " + bookmark + "\n");
            }
            xmlReply += "<mark name=\"" + bookmark + "\" time=\"" + e.AudioPosition.TotalSeconds.ToString() + "\"/>";

            /// Hack begins
            /// Since we don't have a word beginning/ending callback, we resort
            /// to making a simple hack relying on the fact that each word in the SSML
            /// message is enclosed in a (book)mark, so the number of marks is
            /// always even. But the logic used should not rely on the tag name, only
            /// whether it's the odd or even (in order of occurence)
            /// 
            /// NOTE: The use of the # symbol is purely arbitrary, some symbol was needed which would not occer in the speech message
            /// 
            string markerString = "#";
            if (xmlReply.Contains(markerString))
            {
                /// word tag exists, write end time
                /// 
                xmlReply = xmlReply.Replace(markerString, e.AudioPosition.TotalSeconds.ToString());
                xmlReply += "</word>";
            }
            else
            {
                /// no word tag, add new word tag
                /// 
                xmlReply += "<word end=\"" + markerString + "\" start=\"" + e.AudioPosition.TotalSeconds.ToString() + "\">";
            }
        }

        /// <summary>
        /// Do relevant clean up before exiting
        /// </summary>
        public void CleanUp()
        {
            //phonemeToViseme.Clear();
            vhmsg.CloseConnection();
        }

        /// <summary>
        /// Disables debug output, enabled by default
        /// </summary>
        public void disableDebugMessages()
        {
            doDebugChecks = false;
        }

        /// <summary>
        /// Our main function
        /// </summary>
        /// <param name="args"></param>
        [STAThread]
        static void Main(string[] args)
        {
            Program main = new Program();
            List<string> commandLineArguments = new List<string>(args);
            main.InitializeAndRun(commandLineArguments);
        }
    }
}
