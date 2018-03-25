/* Final Project for intro to computing for EE
 * tought by Corey Pennycuff
 * 12/4/17
 * 
 * Code and Hardware done by 
 * Darrell Adams
 * Holden Brown
 * Wesley Garrison
 * Alden Kane
 * 
 * This project is Conway's The Game of Life
 * the rules of the game can be found on line
 * 
 * This code is designed to be interfaced with and Arduino uno hardware in which there are three buttons and two potentiometers 
 * 
 * the start button should be wired to pin 8 of the Arduino
 * the seed (boolean called coord) button should be wired to pin 7 of the Arduino
 * the undo button should be wired to pin 13 of the Arduino
 * 
 * One potentiometer is to be wired to analog pin 0, and the other to analog pin 1 of the arduin
 * 
 * The ROWS and COLS can be redefined following the restriction below
 * 
 * beyond that, the game can be initially seeded by scrolling through the screen, up and down, using the potentiometers and pressing the seed (coord) button
 * the game can be run by holding down the start button, if the user switches to pause the game, and and change conditions,
 * 
 * if there is something that the user wishes to undo, scroll to that position and click the undo button
 * 
 * enjoy!
 */


// include the library code:
#include <LiquidCrystal.h>
#define ROWS 8
#define COLS 20

//This code will work for any sized lCD so long as the #define Rows does not exceed double the physical number of LCD Rows and is an even number
// and so long as the #define COLS does not exceed the physical number of LCD Columns

// initialize the library by associating any needed LCD interface pin
// with the Arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int game[ROWS][COLS];        // 2D array that will hold all the information of the current game board
int curs[ROWS][COLS];        // 2D array that will hold the position of the cursor
int ypos[ROWS];              // 1D array that will be used in mapping the Y position of the cursor (using the "potTransferY" function) based on an outside analog read of a pot



int sumtop = 0;              // int that will hold a number that is used in the logic for defining the value of a cell. I.E if the cursor is there, or if the cell is living or dead. used in the main "loop" function
int sumbottom = 0;           // int very similar to "sumtop", except it is used to determine the value of the cell below the cell that "sumtop" will represent. 
 



int neighborCount(int, int); // function prototype of counting the neighbors of a single cell

void GameOfLife();           // function prototype of the actual iterating portion of the game that will call function neighbots

int potTransferX(double);    // function prototype that will map the cursor in the x position to the analog read of a potentiometer
int potTransferY(double);    // function prototype that will map the cursor in the y position to the analog read of a potentiometer
     


byte fullbit[8] = {          //a full bit of living cells. equates to two living cells, one right above the other
  0b11111,
  0b11111,
  0b11111,
  0b00000,
  0b00000,
  0b11111,
  0b11111,
  0b11111
};



byte topbit[8] = {           //creates a living cell on the top with a dead cell below it
  0b11111,
  0b11111,
  0b11111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

byte bottombit[8] = {        //creates a living cell on the bottom with a dead cell above it
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11111,
  0b11111,
  0b11111
};

byte topxclear[8] = {       //Creates the the cursor on the top with a dead cell below it
  0b10001,
  0b01110,
  0b01110,
  0b10001,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};
////
byte bottomxclear[8] = {     //Creates the the cursor on the bottom with a dead cell above it
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b10001,
  0b01110,
  0b01110,
  0b10001
};

byte topxfill[8] = {         //creates the cursor on the top with a living cell below it
  0b10001,
  0b01110,
  0b01110,
  0b10001,
  0b00000,
  0b11111,
  0b11111,
  0b11111
};

byte bottomxfill[8] = {     //Creates the the cursor on the bottom with a living cell above it
  0b11111,
  0b11111,
  0b11111,
  0b00000,
  0b10001,
  0b01110,
  0b01110,
  0b10001
};

byte empty[8] = {          //Creates the complete blank cell
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};


void setup() {
  Serial.begin(9600); 
  lcd.begin(COLS, ROWS);              // set up the LCD's number of columns and rows:
  
  //instanstiate all the custom chars
  lcd.createChar(1, fullbit);         
  lcd.createChar(3, bottombit);
  lcd.createChar(2, topbit);
  lcd.createChar(4, topxclear);
  lcd.createChar(5, bottomxclear);
  lcd.createChar(6, topxfill);
  lcd.createChar(7, bottomxfill);
  lcd.createChar(8, empty);
  
  // for loops to create the arrays for the game information, and the cursor information and seed them with all zeroes
  for(int K = 0; K<ROWS; K++){
  ypos[K] = K;
  for(int P =0; P<COLS; P++) {
    game[K][P] =0;
    curs[K][P] = 0;
  }
}
}


void loop(){
  double xAnalog = analogRead(A1);                            // read the analog postion of the pot that controls the x movement
  double yAnalog = analogRead(A0);                            // read the analog postion of the pot that controls the y movement
  bool Coord = digitalRead(7);                                // bool for the button that controls whether or not to seed a cell
  bool start = digitalRead(8);                                // bool for the button that controls whether or note to start the game
  bool undo = digitalRead(13);                                 // bool for the button that will unseed a cell
  int x_val = potTransferX(xAnalog);                          // call the potTransfer function to map the x position of the cursor
  int y_val = potTransferY(yAnalog);                          // call the pottransfer function to map the y postion of the cursor

   for(int i = 0; i<ROWS; i++) {
    for(int j =0; j<COLS; j++) {
      curs[i][j] = 0;
    }
   }

 if(Coord){                                                   // if the seed button is pressed, seed the game array that will be used to display, and to run the Game Of Life Algorithym 
      game[y_val][x_val] = 1;
    }

    curs[y_val][x_val] =2;                                   // seed the cursor array with the postion of the cursor gotten from the pot transfer x and y functions.


    // The following for loops are to run through the game and cursor array, and based on the game array value, and the curs array value, will print the desired custom char
    // there are also several cases that use the undo bool do rewrite the game board with a zero at the cursor position, and then rewrite the display.

    for(int R =0; R<ROWS; R=R+2){
      for(int C = 0; C<COLS; C++){
        sumtop =0;
        sumbottom=0;
        sumtop = curs[R][C] + game[R][C];
        sumbottom = curs[R+1][C] + game[R+1][C];
        if(undo && sumtop == 3 && sumbottom == 0) {
          lcd.setCursor(C,R/2);
          lcd.write(byte(4));
          game[y_val][x_val] =0;
        }
        if(undo && sumtop == 3 && sumbottom == 1) {
          lcd.setCursor(C,R/2);
          lcd.write(byte(6));
          game[y_val][x_val] =0;
        }
        if(undo && sumtop == 0 && sumbottom == 3) {
          lcd.setCursor(C,R/2);
          lcd.write(byte(5));
          game[y_val][x_val] =0;
        }
        if(undo && sumtop == 1 && sumbottom ==3) {
          lcd.setCursor(C,R/2);
          lcd.write(byte(7));
          game[y_val][x_val] =0;
        }
        if(sumtop == 0 && sumbottom ==0 && (!undo)){
          lcd.setCursor(C,R/2);
          lcd.write(byte(8));
        }
        else if(sumtop == 1 && sumbottom == 0 && (!undo)){
          lcd.setCursor(C,R/2);
          lcd.write(byte(2));
        }
        else if(sumtop == 1 && sumbottom == 1 && (!undo)) {
          lcd.setCursor(C,R/2);
          lcd.write(byte(1));
        }
        else if(sumtop == 0 && sumbottom == 1 && (!undo)) {
          lcd.setCursor(C,R/2);
          lcd.write(byte(3));
        }
        else if(sumtop == 2 && sumbottom == 0 && (!undo)) {
          lcd.setCursor(C,R/2);
          lcd.write(byte(4));
        }
        else if(sumtop == 2 && sumbottom == 1 && (!undo)) {
          lcd.setCursor(C,R/2);
          lcd.write(byte(6));
        }
        else if(sumtop == 0 && sumbottom == 2 && (!undo)) {
          lcd.setCursor(C,R/2);
          lcd.write(byte(5));
        }
        else if(sumtop == 1 && sumbottom == 2 && (!undo)) {
          lcd.setCursor(C,R/2);
          lcd.write(byte(7));
        }
        else if(sumtop==3 && sumbottom == 0 && (!undo)){
          lcd.setCursor(C,R/2);
          lcd.write(byte(4));
          
        }
        else if(sumtop==3 && sumbottom == 1 && (!undo)){

          lcd.setCursor(C,R/2);
          lcd.write(byte(6));
          
        }
        else if(sumtop==0 && sumbottom == 3 && (!undo)){
          lcd.setCursor(C,R/2);
          lcd.write(byte(5));
  
        }
        else if(sumtop==1 && sumbottom == 3 && (!undo)){
          lcd.setCursor(C,R/2);
          lcd.write(byte(7));
          
        }        
      }
    }

    delay(10);
    if(start) { // if the start button is being pushed, being the iterations of the game
  for (int R =0; R<ROWS; R=R+2){                           // run through the rows, counting by two, to deal with the LCD cells that we "cut" in half with the special chars
    for(int C =0; C<COLS; C++) {                           // run through all the columns
      lcd.setCursor(C,R/2);                                // set the cursor on the LCD to the Columns, then ROWS divide by two, again to deal with the special chars
      if(game[R][C] == 1 && game[R+1][C] == 1) {           // if an even number cell is living, and its neighbor below it is living, write a full byte box sepcial char.
        lcd.write(byte(1));
      }
      else if(game[R][C] == 1 && game[R+1][C] == 0) {      // if an even number cell is living and its neighbor below it is dead, print out a half filled, top byte box
        lcd.write(byte(2));
      }
      else if(game[R][C] == 0 && game[R+1][C] == 1) {      // if an even number ceel is dead and its neighbor below it is alive, print out a half filled, bottom byte box
        lcd.write(byte(3));
      }
      else if(game[R][C] == 0 && game[R+1][C] == 0) {      // if an even number cell is dead, and its neighbor below it is also dead, print out and full empty box
        lcd.print(' ');
      }
    }
  }
   GameOfLife();
  delay(1000);
    }
      
      
}


void GameOfLife() {                                         // function definition for Game of Life function
  

  int neighbors = 0;                                        // initialization of number of neibors

  //for(int k =0; k<GENERATIONS; k++) {                     // start of for loop to loop the number of generations
    int tempGame[ROWS][COLS];                               // create new temporary gameboard for each generation
    for(int i = 0; i < ROWS; i++) {                         //for loop to run through the rows of the board
        for(int j = 0; j < COLS; j++) {                     //for loop to run through the columns of the board
            neighbors = neighborCount(i, j);                // call neighborCount function passing in the current position on
                                                            // on the board, and setting the number of neighbors of the cell 
                                                            // to variable neighbors
            if (game[i][j] == 0) {                          // if statement to check value of current cell on the game board
                if (neighbors == 3) {                       // if the cell is dead and it has three neighbors
                    tempGame[i][j] = 1;                     // set it to be living in the next generation by changing the value
                                                            // of the temporary game board 
                } else {                                    // else if the cell is dead but does not have three neighbors, cell remains dead
                    tempGame[i][j] = 0;
                }
            } else if (game[i][j] == 1) {                   // if statement to check if the current cell is living
                if (neighbors < 2 || neighbors > 3) {       // if it has fewer than two, or greater than three neigbors
                    tempGame[i][j] = 0;                     // the cell becomes dead in the next generation
                } else {
                    tempGame[i][j] = 1;                     // else the cell remains living
                }
            }
        }
    }
    for(int i = 0; i < ROWS; i++) {                         // for loop again to run through the rows of the board
        for(int j = 0; j < COLS; j++) {                     // run through the columns
            game[i][j] = tempGame[i][j];                    // set the current position of the real game board to the current position
                                                            // of the temp board. Now the current position of the real game board
                                                            // is the correct value of the cell for the next generation
            //printf("%d", game[i][j]);                     // print out the value of the current position
        }
    }
  }



int neighborCount(int row, int col) { // neighborCount function
    int neighbors = 0; // create variable neihbors
    for(int i = row - 1; i < row + 2; i++) {                // run from one below the current cell, to one above the current cell 
        for(int j = col - 1; j < col + 2; j++) {            // run from one to the left of the current cell to one to the right
                                                            // this makes a three by three box in which you check all the cells
            if (i == row && j == col) {                     // if state to check if the current cell is the cell you're checking for 
                                                            // for neighbors. IE the middle cell of the three by three square
                continue;                                   // do nothing
            }
            if (i > -1 && j > -1 && i < ROWS && j < COLS) { // first two conditionals check to make sure the
                                                            // current neighbor is not off the 
                                                            // of the left of the four by twenty
                                                            // gameboard. Next two check to make sure it isn't 
                                                            // off of the right side
                neighbors += game[i][j];                    // add the value of neigbor to the value of the position
                                                            // in the game board. I.E if the neighbor cell is living
                                                            // neighbor will increase by one. saves an if statement
            }
        }
    }
    return neighbors;
}

int potTransferX(double x)
{
  //Use map function to scale down analog inputs, which are 5V inputs described over a 8-Bit (1023 Number) range
  int xPosition = map(x, 0, 1023, 0, COLS-1);
  return xPosition;
}


int potTransferY(double y)
{
  //Use map function to scale down analog inputs, which are 5V inputs described over a 8-Bit (1023 Number) range
   int yPosition;
   for(int i =0; i<ROWS; i++){                         // run through a loop the number of ROWS
    if( y<(1023/ROWS)) {                               // if analog Y is below the max analog position divided by the number of rows for the display (the minimum row position
      yPosition = ypos[0];                             // yPosition is the number that corresponds to the the cursors position on the display
                                                       // array ypos is just a 1 D array with length the number of rows, and elements that count up from zero
    }
    else if( y>(1023-(1023/ROWS))) {                   // if analog Y is above the max analogo position - one division of the analog max position (the max row position)
      yPosition = ypos[ROWS-1];
    }
    else if( y > ((1023/ROWS)*(i)) && y <((1023/ROWS)*(i+1))) { // finds the analog postion between two steps. finds Ypos that is not on either end
      yPosition = ypos[i];
    }
    
   }
   return yPosition;
}