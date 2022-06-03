# SevSeg_MAX7219
## A library for the MAX7219 7 segment display with decimals.


![MAX8219 Connected to Arduino UNO](https://github.com/JemRF/max7219/blob/master/pictures/MAX7219%20and%20Arduino.jpg)

## An easy to use library that allows you to write to the display, using the displayText method. 

displayText(Text, Justification)
 - Text : The text you want to display (8 characters or less, plus up to 8 decimals)
 - Justification : Left or right justified. LEFT(0) or RIGHT(1). 

e.g.:
```
sevSeg.displayText("HELLO", LEFT);
```
## Including decimals in the text will automatically take care of decimals like this:

e.g.
```
sevSeg.displayText("96.78F", RIGHT);
```
## Write to specific digits on the display using the DisplayChar method:
```
displayChar(Digit, Char, Decimal)
```
 - Digit : represents the digit number 0-7 (Rightmost Digit = 7, Leftmost Digit = 0)
 - Char : The character to display
 - Decimal : A flag to illuminate the decimal (true/false)

e.g.
```
sevSeg.displayChar(5, 'L', false);
```