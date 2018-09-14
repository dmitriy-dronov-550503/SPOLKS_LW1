#include "stdafx.h"
#include "CommandParser.h"


CommandParser::CommandParser()
{
}


CommandParser::~CommandParser()
{
}

void CommandParser::Bind(string cmdName, CmdPtr cmdFunction)
{
	cmdMap[cmdName] = cmdFunction;
}

void CommandParser::Execute(string commandLine)
{
	vector<string> params;

	string delimiter = "\s";
	size_t pos = 0;
	string token;
	while ((pos = commandLine.find(delimiter)) != std::string::npos) {
		token = commandLine.substr(0, pos);
		params.push_back(token);
		commandLine.erase(0, pos + delimiter.length());
	}

	string cmdName = params[0];

	if (cmdMap[cmdName] != nullptr)
	{
		cmdMap[cmdName](params);
	}
}