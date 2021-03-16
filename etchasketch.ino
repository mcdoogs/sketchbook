/*
  This sketch lets you use an analog joystick module (KY-023 or similar;
  anything with voltage proportional to X/Y pins and a switch for clicking)
  to 'draw' on an LED matrix controlled by a MAX72XX IC. Only supports a
  single matrix at this time. Clicking the joystick clears the display.
*/

#include "LedControl.h" // http://wayoda.github.io/LedControl/

/* pins for joystick */
const int SW_pin = 2; // digital pin connected to switch output
const int X_pin = 0; // analog pin connected to X output
const int Y_pin = 1; // analog pin connected to Y output

/* pins for LED Control (Max72XX) */
LedControl lc=LedControl(12,10,11,1);

/* config */
unsigned long moveDelay = 250; // milliseconds to delay ability to move
int deadzone = 64; // joystick deadzone to ignore small fluctuations
int matrixWidth = 8; // width (in pixels) of matrix
int matrixHeight = 8; // height (in pixels) of matrix

/* state variables */
unsigned long lastLoop; // timestamp of last loop
unsigned long sinceMove = 0; // timestamp of last move
int joystickClick; // current reading from joystick 'switch' (clicking down)
int currentX = 4; // current X location of 'pen'
int currentY = 4; // current Y location of 'pen'

void setup() {
  /* Wake up the MAX72XX from power-saving */
  lc.shutdown(0,false);
  /* Set the intensity to something less eye-searingly bright */
  lc.setIntensity(0,4);
  /* clear display */
  lc.clearDisplay(0);

  /* Set up joystick pins */
  pinMode(SW_pin, INPUT);
  digitalWrite(SW_pin, HIGH);
  joystickClick = digitalRead(SW_pin);

  /* initialize lastLoop */
  lastLoop = millis();

  /* light up initial position on matrix */
  lc.setLed(0, currentY, currentX, true);
}

/* basic int clamps to keep the drawing on the matrix */
int clampToMatrixHeight(int x) {
  return min(matrixHeight - 1, max(x, 0));
}

int clampToMatrixWidth(int x) {
  return min(matrixWidth - 1, max(x, 0));
}

/* read from the joystick and if necessary, move the pen and draw */
void joystickMoveDirectional() {
  // Get a new reading from the joystick's 'switch'
  int newClick = digitalRead(SW_pin); 
  // find the number of millis since the last loop, add it to the time elapsed since last move
  sinceMove += (millis() - lastLoop);

  // if enough have passed, check for input
  if (sinceMove > moveDelay) {
    /* read the X/Y coordinates of joystick */
    int x = analogRead(X_pin);
    int y = analogRead(Y_pin);
    /* get the current X/Y coordinates of the 'pen'; these may or may not be changed by joystick input */
    int newX = currentX;
    int newY = currentY;

    /* check to see if the joystick is being pushed in either axis; accomodate for deadzone and clamp to matrix size */
    if(x > 512 + deadzone) {
      newX = clampToMatrixWidth(currentX + 1); // move right one pixel
    }
    else if(x < 512 - deadzone) {
      newX = clampToMatrixWidth(currentX - 1); // move left one pixel
    }
    if(y > 512 + deadzone) {
      newY = clampToMatrixHeight(currentY + 1); // move up one pixel
    }
    else if(y < 512 - deadzone) {
      newY = clampToMatrixHeight(currentY - 1); // move down one pixel
    }

    /* if either of the X/Y values has changed: update state, light pixel, and reset the sinceMove counter */
    if ((newY != currentY) || (newX != currentX)) {
      currentY = newY;
      currentX = newX;
      lc.setLed(0, currentY, currentX, true);
      sinceMove = 0;
    }
  }

  /* if the switch output has changed, we know the joystick has been 'clicked down' */
  if (joystickClick != newClick) {
    lc.clearDisplay(0); // clear the display
    joystickClick = newClick; // update state
  }

  /* finally, set lastLoop to the current timestamp */
  lastLoop = millis();
}

void loop() {
  joystickMoveDirectional();
}
