#pragma once
#include "stdafx.h"
#include <iostream>
#include <vector>
#include <map>

using namespace std;

typedef void(*CmdPtr)(vector<string> params);

class CommandParser
{
private:
	map<string, CmdPtr> cmdMap;

public:
	CommandParser();
	~CommandParser();

	void Bind(string cmdName, CmdPtr cmdFunction);

	void Execute(string cmdName);
};

