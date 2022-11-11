#include "tokens.hpp"
#include <stdio.h>
#include <iostream>
#include <string>

std::string current_string = "";

void addWord(const char *word)
{
	std::string current(word);
	current_string += current;
}

void convertEscape(const char *escape)
{
	std::string current(escape);
	if (current == "\\n")
		current_string += "\n";
	else if (current == "\\r")
		current_string += "\r";
	else if (current == "\\t")
		current_string += "\t";
	else if (current == "\\\0")
		current_string += "\0";
	else if (current == "\\\\")
		current_string += "\\";
	else if (current == "\\\"")
		current_string += "\"";
	else
	{
		// We have a case of hexadecimal representation. We need to get the first and second digit after the x so we can convert.
		// Take the 2 characters after \x
		current = current.substr(2, 2);
		int left = (int)current[0] - '0';
		// Right digit has a challenge - can be either A-F or 0-9.
		int right;
		if (current[1] <= 'F' && current[1] >= 'A')
		{
			// We have to add 10 to the representation distance from A.
			right = (int)current[1] - 'A' + 10;
		}
		else
		{
			right = (int)current[1] - '0';
		}
		// Since we are at hexa, we have to multiply right by 16.
		char translation = (char)(left * 16 + right);
		current_string += translation;
	}
}

void illegalEscape(const char *escape)
{
	std::string current(escape);
	// Take the string without the backslash
	current = current.substr(1);
	std:: cout << "Error undefined escape sequence " << current << std::endl;
	exit(0);
}

int main()
{
	int token;
	while ((token = yylex()))
	{
		if (token == STRING)
		{
			std::cout << yylineno << " STRING " << current_string << std::endl;
			current_string = "";
		}
	}
	return 0;
}
