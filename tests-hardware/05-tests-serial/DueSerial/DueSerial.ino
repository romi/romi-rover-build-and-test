
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

static void send_counter()
{
        Serial1.println(counter);
        Serial.println(counter);
        counter++;
}

void loop()
{
        send_counter();
        delay(1000);
}
