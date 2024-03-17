//#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <WiFi.h> 
String ssid   =   "PTCL_AAA";
String password = "a1b2c3d4";

char *ssidAp =     "IoT Airfreshner"; // The name of the Wi-Fi network that will be created
char *passwordAp = "";   // The password required to connect to it, leave blank for an open network

static const char* host = "api.thingspeak.com";
//static const char* channelID ="1903597";
//static const char* apiKeyWrite = "2FBTSBE8KJVIPI58"; //00
//static const char* apiKeyRead= "THBCRCVMDXGOUXX2"; 

static const char* channelID ="2404245";
static const char* apiKeyWrite = "QJK7WCJEGXASV8E9";   //airfRESHNER
static const char* apiKeyRead= "9CFDZRQ82WL8G748"; 
const int httpPort = 80;


#define ledPin 2           // GPIO16 //D4
#define relay1 23 

int r1 = 0;
int timer = 2;



int ssidLength,passwordLength;

unsigned long previousMillis = 0;
const long interval = 10000;

String strData;
String url;
int cF=1;

WiFiServer server(httpPort);


void setup()   
{    
  ssid += '\0';
  password += '\0';
  Serial.begin(9600);             
  pinMode(ledPin, OUTPUT);
  pinMode(relay1, OUTPUT);
  digitalWrite(relay1,HIGH);

  
  EEPROM.begin(100);
  delay(100);

     ssidLength=EEPROM.read(0);
     passwordLength=EEPROM.read(30);
     
     ssid.remove(0);
     password.remove(0);
     
     for(int i=0;i<ssidLength;i++)
      {
        ssid +=(char)EEPROM.read(i+1);
        delay(10);
      }

     for(int i=0;i<passwordLength;i++)
      {
        password +=(char)EEPROM.read(i+31);
        delay(10);
      }
   
      EEPROM.commit();  

      char ssidChar[ssidLength+1];
      char passwordChar[passwordLength+1];
      ssid.toCharArray(ssidChar,ssidLength+1);
      password.toCharArray(passwordChar,passwordLength+1);


//      delay(1000);
//      WiFi.softAP(ssidAp, passwordAp);             // Start the access point
//      Serial.print("\nDevice Name:");
//      Serial.print(ssidAp);
//      delay(1000);
//      Serial.print(" IP:");
//      Serial.print(WiFi.softAPIP());         // Send the IP address of the ESP8266 to the computer
//      Serial.println(" started.");
//      delay(2000);
    
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssidChar,passwordChar);
    Serial.println("\nConnecting to.... ");
    Serial.println(ssidChar);
    Serial.println(passwordChar);
    delay(1000);
    digitalWrite(ledPin,HIGH); 
    server.begin();
    Serial.println("HTTP server started");
    delay(1000); 

}

//setup




void loop() 
{ 

  WiFiClient client = server.available();

  if(WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(ledPin,HIGH);
    delay(100);
    Serial.print(".");
    digitalWrite(ledPin,LOW);
    delay(900); 
    cF=1;
  }

if(WiFi.status() == WL_CONNECTED && cF==1)  
  {
    Serial.print("\nConnected to ");
    Serial.println(ssid);
    Serial.print("IP Address:");  
    Serial.println(WiFi.localIP());
    cF=0;   
    digitalWrite(ledPin,LOW);                  
  }
     
  if (client) 
  {
      Serial.println("new client");
      int j = 0;
      int k = 1;
      String ssidTemp; 
      String passwordTemp;
      String req = client.readStringUntil('\r');
      Serial.println(req);
      
      for (int i = 0; i < req.length(); i++)
      {
        if (req[i] == '(' && j == 0)
          j = 1;
        else if (req[i] == '(' && j == 1)
          {j=0; break;}
        else if (j == 1)
          { 
           if(req[i]=='~')
             ssidTemp += ' ';
           else
             ssidTemp += req[i];
          }
      }
      
     j=0;
     for (int i = 0; i < req.length(); i++)
      {


        if (req[i] == ')' && j == 0)
          j = 1;
        else if (req[i] == ')' && j == 1)
          {j=0; break;}
        else if (j == 1)
          { 
           if(req[i]=='~')
             passwordTemp += ' ';
           else
             passwordTemp += req[i];
          }
      }

      ssid=ssidTemp;
      password=passwordTemp;
      
      ssidLength=ssid.length();
      passwordLength=password.length();
      
      EEPROM.write(0,ssidLength);
      delay(100);
      EEPROM.write(30,passwordLength);
      
      for(int i=0;i<ssidLength;i++)
      {
        EEPROM.write(i+1,ssid[i]);
        delay(10);
      }
      for(int i=0;i<passwordLength;i++)
      {
        EEPROM.write(i+31,password[i]);
        delay(10);
      }
      EEPROM.commit();

      char ssidChar[ssidLength+1];
      char passwordChar[passwordLength+1];
      ssid.toCharArray(ssidChar,ssidLength+1);
      password.toCharArray(passwordChar,passwordLength+1);

      Serial.println(ssidLength);
      Serial.println(passwordLength);
      Serial.println(ssidChar);
      Serial.println(passwordChar);

      String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
      s += "Save";
      client.print(s);
      client.flush();
      delay(2000);
      WiFi.begin(ssidChar,passwordChar);
      //WiFi.softAPdisconnect();
            

  }

  
  unsigned long currentMillis = millis(); 
  if(currentMillis - previousMillis >= interval) 
  {     
             previousMillis = currentMillis;
    

            // Use WiFiClient class to create TCP connections
            if (!client.connect(host, httpPort)) 
            {
              Serial.println("Internet Connection failed");
              digitalWrite(ledPin,HIGH);
              delay(100);
              digitalWrite(ledPin,LOW);
              delay(900);
              return;
            }

           
            url = "/channels/";   
            url+= channelID;
            url+= "/feeds/last.json?api_key=";
            url+= apiKeyRead;
            url+= "&results=2";
            
            Serial.print("Requesting URL: ");
            Serial.println(url);
                 
            client.println(String("GET ") + url + " HTTP/1.1\r\n" +
                         "Host: " + host + "\r\n" +
                         "Connection: close\r\n\r\n");
            delay(1000);
          
            while(client.available())
            {
              strData = client.readStringUntil('\n');
            //  Serial.println(strData);
              if(strData != "" || strData != "-1") 
              {
                String jsonReq = strData; //if send json data, it have only 1 argument
                int size = jsonReq.length() + 1;
                char json[size];
                jsonReq.toCharArray(json, size);
                StaticJsonBuffer<200> jsonBuffer;
                JsonObject& json_parsed = jsonBuffer.parseObject(json);
               
                if (json_parsed.containsKey("field1"))
                  {
                      String field1 = json_parsed["field1"];
                      Serial.print("Field1:");
                      Serial.print(field1); 
                      if (field1 == "1")
                      r1=1;
                         
                   }

                if (json_parsed.containsKey("field2"))
                  {
                      String field2 = json_parsed["field2"];
                      Serial.print("Field2:");
                      Serial.println(field2);
                      if (r1 == 1)
                      {
                        timer=field2.toInt();
                        digitalWrite(relay1,LOW); 
                        digitalWrite(ledPin,HIGH); 
                        Serial.print(" ON");
                        Serial.print(timer);
                        delay(timer*1000);
                        digitalWrite(relay1,HIGH); 
                        digitalWrite(ledPin,LOW); 
                        Serial.println(" OFF");
                        r1=0;
                        url= "/update?api_key=";
                        url+=apiKeyWrite;
                        url+="&field1=";
                        url+=r1;
                        url+="&field2=";
                        url+=timer;


                        delay(1000);
                        client.connect(host, httpPort);
                        delay(14000);
                        Serial.println("\nUpdate https://api.thingspeak.com"+ url);
                        client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                        "Host: " + host + "\r\n" +
                        "Connection: close\r\n\r\n");
                        strData = client.readStringUntil('\n');
                        Serial.println(strData);
                        Serial.println("Write Success");
                        delay(5000);
                      }
    
                   }                 
                   
              }
          }
       
  }
  
 
}

//Loop
