/*
 * Noah Harren, Bryan Bylsma
 * April 24, 2019
 */

#include "bouncer.h"
#include <ostream>
#include <fstream>

int main(int argc, char const *argv[]){
    std::cout << "Starting up the Bouncer" << std::endl;
    std::string file_name = argv[1];
        if(file_name.substr(file_name.length()-4,file_name.length()) != ".jpg"){
                std::cout << "File must be a jpg!" << std::endl;
                return 1;
        }
    std::cout << "Got the right file type" <<std::endl;
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
