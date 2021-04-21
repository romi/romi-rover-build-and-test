
void setup()
{
        Serial.begin(115200);
        while (!Serial)
                ;
        
        Serial1.begin(115200);
        while (!Serial1)
                ;
}

static void send_request()
{
        Serial.println("Sending: #G:xxxx");
        Serial1.print("#G:xxxx\r\n");
}

static void read_reply()
{
        Serial.print("Reading reply: ");
        while (true) {
                
                int c = Serial1.read();
                
                if (c == -1) {
                        Serial.println("Read error!");
                        break;
                } else if (c == '\r') {
                        // skip
                } else if (c == '\n') {
                        Serial.println();
                        break;
                } else {
                        Serial.print((char) c);
                }
        }
}

void loop()
{
        send_request();
        read_reply();
        delay(1000);
}
