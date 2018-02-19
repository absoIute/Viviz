# Viviz
Windows C++ API for fetching information on symbols in a process.

## Lazy Example
```cpp
#include "Viviz.h"
#include <iostream>

int main()
{
	Viviz symbols(GetCurrentProcessId()); //initializing w/ the target's processid

	for (const auto &symbol : symbols.FromName("LoadLibraryA")) //finding all symbols named 'LoadLibraryA'
	{
		std::cout << std::hex << symbol.address << '\t' << symbol.module_name << '\n';
	}

	return 0;
}
```