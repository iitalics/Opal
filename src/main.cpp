#include <syntax/Scanner.h>

int main ()
{
	SourceError::color = true;

	try
	{
		Scanner scan("tests/simple.opal");
	}
	catch (SourceError& err)
	{
		std::cerr << err.what() << std::endl;
	}
}