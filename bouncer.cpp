/*
 * Noah Harren, Bryan Bylsma
 * April 24, 2019
 */

#include "bouncer.h"

int main()
{
  std::cout << "Starting up the Bouncer" << std::endl;
  return 0;
}



/**
 *@brief This simply writes a char[] buffer to a string filename. 
 */
void WriteBuffer(std::string filename, char [] buffer){
    std::ostream opener(filename);
    if(opener.is_open()){
        opener << buffer;
    }
    opener.close();
}