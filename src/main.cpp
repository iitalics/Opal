#include <syntax/Scanner.h>

int main ()
{
	SourceError::color = true;

	try
	{
		Scanner scan("tests/lex-tokens.txt");

		while (scan.get() != Tokens::END_OF_FILE)
		{
			std::cout << scan.get().str() << std::endl;
			scan.shift();
		}
	}
	catch (SourceError& err)
	{
		std::cerr << err.what() << std::endl;
	}
}