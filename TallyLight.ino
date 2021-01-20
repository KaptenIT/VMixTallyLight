#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>


int delayval = 10;  //Makes the color change "chase" for easy periferal sight when camera change occurs

ESP8266WiFiMulti WiFiMulti;

const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data
boolean newData = false;
int CAMnum= 8 + 2; // 1st input in vmix
int green = 16; // pin number for green
int red = 05; // pin number for red

IPAddress server(192, 168, 11, 2);  //IP address of the vMix production system

WiFiClient client;

void setup()  {
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(D3, OUTPUT);
  Serial.begin(9600);
  WiFiMulti.addAP("SsID", "password");  //Replace with SSID and PASSWORD of local wifi

  while (WiFiMulti.run() != WL_CONNECTED)  {
    Serial.print(".");
    delay(500);
  }

  // the following lines are for serial debugging of wifi connection and connection to vMix server
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
 
  WiFi.setSleepMode(WIFI_NONE_SLEEP);  // to keep the WiFi connection alive always
 
  delay(500);
 
  Serial.println("Connecting to VMIX...");
  if (client.connect(server, 8099))  {  //8099 is the vMix TCP/IP API port
    Serial.println("VMIX Connected");
    delay(500);
  }
  
 
  client.print("SUBSCRIBE TALLY\r\n");  //issues the command to vMix to start pushing any tally status changes to client
}

void loop() {
  recvWithEndMarker();
  showNewData();
}

void recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while (client.available() > 0 && newData == false) {
    rc = client.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

void showNewData() {
  if (newData == true) {
    Serial.print("This just in ... ");  //for debugging the recieved information
    Serial.println(receivedChars);      //and determining if it's what is expected

    if ( ( receivedChars[CAMnum] == '0' ) ) {  //char 10 is input 2 status...  0 is NOT IN USE and LED should glow dim white (grey).
        digitalWrite(D3, HIGH); 
        digitalWrite(green, LOW);
        digitalWrite(red, LOW);
        delay(delayval);
    }

    if ( ( receivedChars[CAMnum] == '2' ) ) { //2 = input 2 is PREVIEW and should light LED dim GREEN
        digitalWrite(green, HIGH); 
        digitalWrite(D1, LOW);
        digitalWrite(D3, LOW);
        delay(delayval);
    }

    if ( ( receivedChars[CAMnum] == '1' ) ) { //1 = input 2 is ON AIR and should light LED dim RED
        digitalWrite(green, LOW);
        digitalWrite(D3, LOW);
        digitalWrite(red, HIGH); 
        delay(delayval);
    }
    newData = false;
  }
}
