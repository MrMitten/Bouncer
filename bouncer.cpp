/*
 * Noah Harren, Bryan Bylsma
 * April 24, 2019
 */

#include "bouncer.h"
#include <ostream>
#include <fstream>
int main()
{
  std::cout << "Starting up the Bouncer" << std::endl;
  return 0;
}



/**
 *@brief This simply writes a char[] buffer to a string filename. 
 */
void WriteBuffer(char* filename, char* buffer){
    std::ofstream opener;
    opener.open(filename);
    if(opener.is_open()){
        opener << buffer;
    }
    opener.close();
}
