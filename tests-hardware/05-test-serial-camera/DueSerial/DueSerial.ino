
void setup()
{
        Serial.begin(115200);
        while (!Serial)
                ;
        
        Serial1.begin(115200);
        while (!Serial1)
                ;
}

static int counter = 0;

static void send_message()
{
        Serial1.println(counter);
        Serial.println(counter);
        counter += 2;
}

static void read_reply()
{
}

void loop()
{
        send_message();
        read_reply();
        delay(1000);
}
