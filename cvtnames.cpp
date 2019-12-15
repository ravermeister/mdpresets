using Microsoft.Azure.CognitiveServices.ContentModerator;
using Newtonsoft.Json;
using RatiocinationLibrary;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading;

namespace cognitiveContentModerator
{
    class Program
    {
        static int bad = 1;
        static int good = 0;
        static int glitch = -1;

        // TEXT MODERATION
        // Name of the file that contains text
        private static readonly string TextFile = "TextModerationInput.txt";

        // The name of the file to contain the output from the evaluation.
        private static string OutputFile = "TextModerationOutput.txt";
        private static readonly string mySubscriptionKey = "###############";
        private static readonly string myEndpoint = "https://eastus.api.cognitive.microsoft.com";

        static void Main(string[] args)
        {
            Log.Initialize();
            Print("Hello World");
            var files = ScanMilkDropFiles();


            // string raw_text = File.ReadAllText(TextFile);
            // var text_strings = raw_text.Split(System.Environment.NewLine.ToCharArray());
            
            
            // Load the input text.
            foreach (var i in files)
            {
                int decision = CognitivePostAPI(i);
                if (decision == good)
                    MoveGoodFile(i);
                if (decision == bad)
                    MoveBadFile(i);
                Thread.Sleep(100);
            }
        }

        private static void MoveBadFile(string path)
        {
            try
            {
                // To move a file or folder to a new location:
                //  D:\GIT\milkdrops\bad_presets\presets_untested --> presets_untested_bad
                var destinationPath = path.Replace("presets_untested", "presets_untested_bad");
                System.IO.Directory.CreateDirectory("D:\\GIT\\milkdrops\\bad_presets\\presets_untested_bad");
                File.Move(path, destinationPath);
            }
            catch (Exception e)
            {
                Log.Exception(e);
            }
        }

        private static void MoveGoodFile(string path)
        {
            try
            {
                // To move a file or folder to a new location:
                //  D:\GIT\milkdrops\bad_presets\presets_untested --> presets_untested_good
                var destinationPath = path.Replace("presets_untested", "presets_untested_good");
                System.IO.Directory.CreateDirectory("D:\\GIT\\milkdrops\\bad_presets\\presets_untested_good");
                File.Move(path, destinationPath);
            }
            catch (Exception e)
            {
                Log.Exception(e);
            }
        }

        private static List<string> ScanMilkDropFiles()
        {
            // scan
            string presetspath = "D:\\GIT\\milkdrops\\bad_presets\\presets_untested";
            DirectorySearch.Initialize("*", 1);
            //DirectorySearch.Initialize("*");
            DirectorySearch.SetSearchPath(presetspath);
            DirectorySearch.ProcessDirectory();
            var files = DirectorySearch.GetSearchList();
            return files;
        }

        static int CognitivePostAPI(string raw_text)
        {
            Print("Scanning : " + raw_text + 
                  " with cognitive API");
            try
            {
                var text = Path.GetFileName(raw_text).Replace(System.Environment.NewLine, " ");
                byte[] byteArray = System.Text.Encoding.UTF8.GetBytes(text);
                MemoryStream stream = new MemoryStream(byteArray);

                // Create a Content Moderator client and evaluate the text.
                using (var client = Clients.NewClient())
                {
                    // Screen the input text: check for profanity,
                    // autocorrect text, check for personally identifying
                    // information (PII), and classify the text into three categories
                    Log.TextEntry("Autocorrect typos, check for matching terms, PII, and classify.");
                    var screenResult =
                        client.TextModeration.ScreenText("text/plain", stream, "eng", true, true, null, true);
                    var cog = JsonConvert.SerializeObject(screenResult, Formatting.Indented);
                    Log.TextEntry(cog);

                    if (cog.Contains("ReviewRecommended\": true"))
                    {
                        // "ReviewRecommended": true
                        Log.TextEntry("#############");
                        Log.TextEntry("#### Warning:");
                        Log.TextEntry("#### Review");
                        Log.TextEntry("#### Recommended");
                        Log.TextEntry("#############");
                        return bad;
                    }
                    else
                    {
                        return good;
                    }
                }
            }
            catch (Exception e)
            {
                Log.Exception(e);
                return glitch;
            }
        }


        static void Print(String text = "")
        {
            string message = DateTime.Now.ToString("yyyyMMddHHmmssffff: " + Environment.NewLine) + text;
            Log.TextEntry(message);
        }


        // Wraps the creation and configuration of a Content Moderator client.
        public static class Clients
        {
            // The base URL fragment for Content Moderator calls.
            // Add your Azure Content Moderator endpoint to your environment variables.
            private static readonly string AzureBaseURL = myEndpoint;

            // Your Content Moderator subscription key.
            // Add your Azure Content Moderator subscription key to your environment variables.
            private static readonly string CMSubscriptionKey = mySubscriptionKey;

            // Returns a new Content Moderator client for your subscription.
            public static ContentModeratorClient NewClient()
            {
                // Create and initialize an instance of the Content Moderator API wrapper.
                ContentModeratorClient client = new ContentModeratorClient(new ApiKeyServiceClientCredentials(CMSubscriptionKey));

                client.Endpoint = AzureBaseURL;
                return client;
            }
        }

    }
}
