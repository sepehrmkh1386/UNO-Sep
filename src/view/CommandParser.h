#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <string>

#include "Command.h"

using namespace std;

class CommandParser {
public:
    static Command parse(const string& line);
};

#endif