import processing.serial.*;

Serial myPort;        // The serial port
int xPos = 1;         // horizontal position of the graph
float height_old = 0;
float height_new = 0;
float inByte = 0;


void setup () {
  
  size(1000, 400);   // set the window size:     

  // List all the available serial ports
  println(Serial.list());
  
  myPort = new Serial(this, Serial.list()[2], 9600);// Open whatever port is the one you're using.

  myPort.bufferUntil('\n');   // don't generate a serialEvent() unless you get a newline character:
  // set inital background:
  background(0xff);
}


void draw () {
  // everything happens in the serialEvent()
}


void serialEvent (Serial myPort) {
  
  String inString = myPort.readStringUntil('\n');// get the ASCII string:

  if (inString != null) {
    
    inString = trim(inString);// trim off any whitespace:

    // If leads off detection is true notify with blue line
    if (inString.equals("!")) { 
      stroke(0, 0, 0xff); //Set stroke to blue ( R, G, B)
      inByte = 512;  // middle of the ADC range (Flat Line)
    }
    // If the data is good let it through
    else {
      stroke(0xff, 0, 0); //Set stroke to red ( R, G, B)
      inByte = float(inString); 
     }
     
     //Map and draw the line for new data point
     inByte = map(inByte, 0, 1023, 0, height);
     height_new = height - inByte; 
     line(xPos - 1, height_old, xPos, height_new);
     height_old = height_new;
    
      // at the edge of the screen, go back to the beginning:
      if (xPos >= width) {
        xPos = 0;
        background(0xff);
      } 
      else {
        // increment the horizontal position:
        xPos++;
      }
    
  }
}