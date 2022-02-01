
/*
 * Setup fupnction. Will be runed after each deep sleep and at start
 */
void setup() {
  Serial.begin(115200);
  Serial.print("Hello World");
}

/*
 * Loop is checking if we can go to deep sleep after sending data to TTN
 */
void loop() {

}
