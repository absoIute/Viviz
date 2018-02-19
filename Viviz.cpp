#include "ProcSym.h"

Viviz::Viviz() {}

Viviz::Viviz(DWORD ProcessId) : processId(ProcessId)
{
	this->Refresh();
}

Viviz::~Viviz() {}

void Viviz::SetProcessId(DWORD ProcessId)
{
	this->processId = ProcessId;
	this->Refresh();
}

DWORD Viviz::GetProcessId() const
{
	return this->processId;
}

void Viviz::Refresh()
{
	this->symbols.clear();
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_OPERATION, FALSE, this->processId);

	if (hProcess)
	{
		if (SymInitialize(hProcess, NULL, TRUE))
		{
			HMODULE hMods[0x1000];
			DWORD out;
			
			if (EnumProcessModulesEx(hProcess, hMods, 0x1000, &out, LIST_MODULES_ALL))
			{
				for (auto i = 0; i < out / 4; ++i)
				{
					MODULEINFO mi;

					if (GetModuleInformation(hProcess, hMods[i], &mi, sizeof(mi)))
					{
						char path[MAX_PATH];
						if (GetModuleBaseName(hProcess, hMods[i], path, MAX_PATH))
						{
							this->module_name = path;
							
						}
						else
						{
							this->module_name = std::string();
						}
						SymEnumSymbols(hProcess, reinterpret_cast<ULONG64>(mi.lpBaseOfDll), "*", SymEnumSymbolsProc, this);
					}
				}
			}
			SymCleanup(hProcess);
		}
		CloseHandle(hProcess);
	}
}

BOOL CALLBACK Viviz::SymEnumSymbolsProc(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext)
{
	Viviz *c = static_cast<Viviz*>(UserContext); //'this' ptr passed as UserContext
	c->symbols.push_back(WinSymbol(pSymInfo, c->module_name));
	return TRUE;
}


std::vector<WinSymbol> Viviz::All() const
{
	return this->symbols;
}

std::vector<WinSymbol> Viviz::FromAddress(ULONG64 Address) const
{
	std::vector<WinSymbol> results;
	for (auto i = this->symbols.begin(); i != this->symbols.end(); ++i)
	{
		if (i->address == Address)
		{
			results.push_back(*i);
		}
	}
	return results;
}

std::vector<WinSymbol> Viviz::FromName(const std::string &Name) const
{
	std::vector<WinSymbol> results;
	for (auto i = this->symbols.begin(); i != this->symbols.end(); ++i)
	{
		if (i->name == Name)
		{
			results.push_back(*i);
		}
	}
	return results;
}

std::vector<WinSymbol> Viviz::FromModule(ULONG64 ModuleBase) const
{
	std::vector<WinSymbol> results;
	for (auto i = this->symbols.begin(); i != this->symbols.end(); ++i)
	{
		if (i->module_address == ModuleBase)
		{
			results.push_back(*i);
		}
	}
	return results;
}

std::vector<WinSymbol> Viviz::FromModule(const std::string &ModuleName) const
{
	std::vector<WinSymbol> results;
	for (auto i = this->symbols.begin(); i != this->symbols.end(); ++i)
	{
		if (i->module_name == ModuleName)
		{
			results.push_back(*i);
		}
	}
	return results;
}

std::vector<WinSymbol> Viviz::RegExMatch(const std::regex &re) const
{
	std::vector<WinSymbol> results;
	std::smatch m;
	for (auto i = this->symbols.begin(); i != this->symbols.end(); ++i)
	{
		if (std::regex_match(i->name, m, re))
		{
			results.push_back(*i);
		}
	}
	return results;
}



WinSymbol::WinSymbol(PSYMBOL_INFO pi, std::string mn) : name(pi->Name), address(pi->Address), module_address(pi->ModBase), module_name(mn) {}

WinSymbol::~WinSymbol() {}