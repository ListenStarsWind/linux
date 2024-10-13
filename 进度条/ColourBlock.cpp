#include"ColourBlock.h"

//#include <iostream>
//
//void printColoredBlock(const std::string& color, const std::string& block) {
//    std::cout << color << block << "\033[0m"; // 0m ÖØÖÃÑÕÉ«
//}
//
//int main() {
//    // ANSI ×ªÒåÂë
//    const std::string red = "\033[41m";    // ºìÉ«±³¾°
//    const std::string green = "\033[42m";  // ÂÌÉ«±³¾°
//    const std::string blue = "\033[44m";   // À¶É«±³¾°
//    const std::string yellow = "\033[43m"; // »ÆÉ«±³¾°
//
//    // Êä³öÉ«¿é
//    printColoredBlock(red, "      ");    // ºìÉ«¿é
//    printColoredBlock(green, "      ");  // ÂÌÉ«¿é
//    printColoredBlock(blue, "      ");   // À¶É«¿é
//    printColoredBlock(yellow, "      "); // »ÆÉ«¿é
//
//    std::cout << "\033[0m" << std::endl; // ÖØÖÃÑÕÉ«
//
//    return 0;
//}