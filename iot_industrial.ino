#include<ThingSpeak.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
const char* ssid = "********"; // Enter the SSID of your WiFi Network.
const char* password = "******";// Enter the Password of your WiFi Network.
char server[] = "*****"; // The SMTP Server Eg- "secure214.sgcpanel.com"
float tempC=0;
int a=1;
WiFiClient espClient;
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged TO GPIO 4
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Number of temperature devices found
int numberOfDevices;

// We'll use this variable to store a found device address
DeviceAddress tempDeviceAddress; 

//http client declaration
WiFiClient client;
HTTPClient http;    //Declare object of class HTTPClient

//For Current Sensor
#define CTSENSORCurrent 30  //Max value of current that can be calculated by the Current sensor
double amps=-1,amps1;
double analog,peakI=0,rmsI;

#define ThresholdTemp 32           //Threshold temperature 
#define ThresholdCurrent 20       //Threshold current 

void setup()
{
 
  Serial.begin(9600);
  delay(10);
  Serial.println("");
  Serial.println("");
  Serial.print("Connecting To: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("*");
  }
  Serial.println("");
  Serial.println("WiFi Connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
   sensors.begin();
  
  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();
  
  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" devices.");

  // Loop through each device, print out address
  for(int i=0;i<numberOfDevices; i++){
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)){
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
    //  printAddress(tempDeviceAddress);
      Serial.println();
    } else {
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }
  }

 

 }

void loop()
{

      sensors.requestTemperatures(); // Send the command to get temperatures
  
  // Loop through each device, print out temperature data
  for(int i=0;i<numberOfDevices; i++){
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)){
      // Output the device ID
      Serial.print("Temperature for device: ");
      Serial.println(i,DEC);
      // Print the data
      tempC = sensors.getTempC(tempDeviceAddress);
      Serial.print("Temp C: ");
      Serial.print(tempC);
      Serial.print(" Temp F: ");
      Serial.println(DallasTemperature::toFahrenheit(tempC)); // Converts tempC to Fahrenheit

String url="https://api.thingspeak.com/update?api_key=APIKEYHERE&field1="+(String)tempC;      //http request url (enter your api key)
          http.begin(url,"27:18:92:DD:A4:26:C3:07:09:B9:7A:E6:C5:21:B9:5B:48:F7:16:E1");     // begin the http connection
         http.addHeader("Content-Type", "text/plain");  //Specify content-type header
       
         int httpCode = http.GET();   //Send the request
         String payload = http.getString();                  //Get the response payload
       
         Serial.println("http code:"+httpCode);   //Print HTTP return code
         Serial.println("entry number:"+payload);    //Print request response payload
       
         http.end();  //Close connection

  
    }
    
  }
  if(tempC>ThresholdTemp)
  byte ret = sendEmailTemp();
  delay(50);
  
    analog=analogRead(A0); // Calculate Irms 

    int j;
    for(int i=1;i<=6;i++)
    {
      j=analogRead(A0);
      if(peakI<j)
      peakI=j;
      delay(1);    
      }
     rmsI=((peakI/1023)*CTSENSORCurrent*3.3)/1.414;
      peakI=0;

    
Serial.print("Amps:     ");
    Serial.print(rmsI); 
    Serial.print("RMS");
    Serial.println();
    delay(900);
String url="https://api.thingspeak.com/update?api_key=APIKEYHERE&field1="+(String)tempC;      //http request url (enter your api key)
          http.begin(url,"27:18:92:DD:A4:26:C3:07:09:B9:7A:E6:C5:21:B9:5B:48:F7:16:E1");     // begin the http connection
         http.addHeader("Content-Type", "text/plain");  //Specify content-type header
       
         int httpCode = http.GET();   //Send the request
         String payload = http.getString();                  //Get the response payload
       
         Serial.println("http code:"+httpCode);   //Print HTTP return code
         Serial.println("entry number:"+payload);    //Print request response payload
       
         http.end();  //Close connection

   if(rmsI>ThresholdCurrent)
   byte ret =sendEmailCurrent(); 

   delay(15000);
}
  

byte sendEmailCurrent()
{
 
  if (espClient.connect(server,2525) == 1) 
  {
    Serial.println(F("connected"));
  } 
  else 
  {
    Serial.println(F("connection failed"));
    return 0;
  }
  if (!emailResp()) 
    return 0;
  //
  Serial.println(F("Sending EHLO"));
  espClient.println("EHLO www.example.com");
  if (!emailResp()) 
    return 0;
  //
  /*Serial.println(F("Sending TTLS"));
  espClient.println("STARTTLS");
  if (!emailResp()) 
  return 0;*/
  //  
  Serial.println(F("Sending auth login"));
  espClient.println("AUTH LOGIN");
  if (!emailResp()) 
    return 0;
  //  
  Serial.println(F("ASCII_ENCODED_USERNAME_HERE"));
  // Change this to your base64, ASCII encoded username
  /*
  For example, the email address test@gmail.com would be encoded as dGVzdEBnbWFpbC5jb20=
  */
  espClient.println("ASCII_ENCODED_USERNAME_HERE"); //base64, ASCII encoded Username
  if (!emailResp()) 
    return 0;
  //
  Serial.println(F("ASCII_encoded_password"));
  // change to your base64, ASCII encoded password
  /*
  For example, if your password is "testpassword" (excluding the quotes),
  it would be encoded as dGVzdHBhc3N3b3Jk
  */
  espClient.println("ASCII_encoded_password");//base64, ASCII encoded Password
  if (!emailResp()) 
    return 0;
  //
  Serial.println(F("Sending From"));
  // change to sender email address
  espClient.println(F("MAIL From: sendermailid@example.com")); //Senders Mail id 
  if (!emailResp()) 
    return 0;
  // change to recipient address
  Serial.println(F("Sending To"));
  espClient.println(F("RCPT To: divijvignesh12@gmail.com"));
  if (!emailResp()) 
    return 0;
  //
  Serial.println(F("Sending DATA"));
  espClient.println(F("DATA"));
  if (!emailResp()) 
    return 0;
  Serial.println(F("recieversmailId@example.com"));  //Recievers mail id 
  // change to recipient address
  espClient.println(F("To:  recieversmailId@example.com"));
  // change to your address
  espClient.println(F("From: sendermailid@example.com"));"));  //Senders Mail id here
  espClient.println(F("Subject: Current Sensor Alert\r\n"));
  espClient.println(F("CT sensor values\n"));
  espClient.println(F("Current has exceeded Threshold Temperature"));

  //
  espClient.println(F("."));
  if (!emailResp()) 
    return 0;
  //
  Serial.println(F("Sending QUIT"));
  espClient.println(F("QUIT"));
  if (!emailResp()) 
    return 0;
  //
  espClient.stop();
  Serial.println(F("disconnected"));
  return 1;
}
byte sendEmailTemp()
{
 
  if (espClient.connect(server,2525) == 1) 
  {
    Serial.println(F("connected"));
  } 
  else 
  {
    Serial.println(F("connection failed"));
    return 0;
  }
  if (!emailResp()) 
    return 0;
  //
  Serial.println(F("Sending EHLO"));
  espClient.println("EHLO www.example.com");
  if (!emailResp()) 
    return 0;
  //
  /*Serial.println(F("Sending TTLS"));
  espClient.println("STARTTLS");
  if (!emailResp()) 
  return 0;*/
  //  
  Serial.println(F("Sending auth login"));
  espClient.println("AUTH LOGIN");
  if (!emailResp()) 
    return 0;
  //  
  Serial.println(F("ASCII_encoded_Username"));  //ASCII_encoded_Username here
  // Change this to your base64, ASCII encoded username
  /*
  For example, the email address test@gmail.com would be encoded as dGVzdEBnbWFpbC5jb20=
  */
  espClient.println("ASCII_encoded_Username"); //base64, ASCII encoded Username
  if (!emailResp()) 
    return 0;
  //
  Serial.println(F("ASCII_encoded_Password_Here")); //ASCII_encoded_Password 
  // change to your base64, ASCII encoded password
  /*
  For example, if your password is "testpassword" (excluding the quotes),
  it would be encoded as dGVzdHBhc3N3b3Jk
  */
  espClient.println("ASCII_encoded_Password_Here");//base64, ASCII encoded Password
  if (!emailResp()) 
    return 0;
  //
  Serial.println(F("Sending From"));
  // change to sender email address
  espClient.println(F("MAIL From: recievers_Mailid@example.com"));  //Recievers Mail id here
  if (!emailResp()) 
    return 0;
  // change to recipient address
  Serial.println(F("Sending To"));
  espClient.println(F("RCPT To: recieversmailId@example.com"));   //Recievers Mail id here
  if (!emailResp()) 
    return 0;
  //
  Serial.println(F("Sending DATA"));
  espClient.println(F("DATA"));
  if (!emailResp()) 
    return 0;
  Serial.println(F("gk@maple-software.com"));
  // change to recipient address
  espClient.println(F("To:  recieversmailId@example.com"));
  // change to your address
  espClient.println(F("From: donot-reply@maple-software.com"));
  espClient.println(F("Subject: ESP8266 test e-mail\r\n"));
  espClient.println(F("DS18B20 sensor values\n"));
  espClient.println(F("Temperature has exceeded 32^C"));
 // espClient.println(F("Temperature in Fareheit:-."));
  //
  espClient.println(F("."));
  if (!emailResp()) 
    return 0;
  //
  Serial.println(F("Sending QUIT"));
  espClient.println(F("QUIT"));
  if (!emailResp()) 
    return 0;
  //
  espClient.stop();
  Serial.println(F("disconnected"));
  return 1;
}

byte emailResp()
{
  byte responseCode;
  byte readByte;
  int loopCount = 0;
  
  while (!espClient.available()) 
  {
    delay(1);
    loopCount++;
    // Wait for 20 seconds and if nothing is received, stop.
    if (loopCount > 20000) 
    {
      espClient.stop();
      Serial.println(F("\r\nTimeout"));
      return 0;
    }
  }

  responseCode = espClient.peek();
  while (espClient.available())
  {
    readByte = espClient.read();
    Serial.write(readByte);
  }

  if (responseCode >= '4')
  {
    //  efail();
    return 0;
  }
  return 1;
}
