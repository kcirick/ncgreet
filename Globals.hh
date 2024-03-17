#ifndef GLOBALS_HH
#define GLOBALS_HH

#include <iostream>
#include <string>

using namespace std;

//--- enums ---
enum MessageType { DEBUG, INFO, WARNING, ERROR, NMSG };
enum ScrollDirection { PREV, NEXT };

//--- helper functions in Main.cc ---
string trimString(string);
string getToken(string&, char); 
void spawn(string);

#endif
