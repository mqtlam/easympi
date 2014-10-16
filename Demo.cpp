#include "EasyMPI.h"
#include <iostream>
#include <string>

void masterDemo();
void slaveDemo();

int main(int argc, char* argv[])
{
	// initialize
	EasyMPI::EasyMPI::initialize(argc, argv);

	// print rank and num processes
	std::cout << "Rank=" << EasyMPI::EasyMPI::getProcessID() << std::endl;
	std::cout << "Size=" << EasyMPI::EasyMPI::getNumProcesses() << std::endl;

	// master slave demo
	if (EasyMPI::EasyMPI::getProcessID() == 0)
	{
		masterDemo();
	}
	else
	{
		slaveDemo();
	}

	// finalize
	EasyMPI::EasyMPI::finalize();

	return 0;
}

void masterDemo()
{
	// declare commands and messages
	std::vector<std::string> commands;
	std::vector<std::string> messages;

	// demo commands
	commands.push_back("DEMO1");
	messages.push_back("message for demo1");
	commands.push_back("DEMO2");
	messages.push_back("message for demo2");

	// define finish command
	std::string finishCommand = "FINISHDEMO";

	// schedule tasks
	EasyMPI::EasyMPI::masterScheduleTasks(commands, messages, finishCommand);
}

void slaveDemo()
{
	// loop to wait for tasks
	while (true)
	{
		// wait for a task
		std::string command;
		std::string message;
		EasyMPI::EasyMPI::slaveWaitForTasks(command, message);

		std::cout << "Got command '" << command << "' and message '" << message << "'" << std::endl;

		// define branches here to perform task depending on command
		if (command.compare("DEMO1") == 0)
		{
			std::cout << "Got DEMO1 command on process " << EasyMPI::EasyMPI::getProcessID() 
				<< " with message: " << message << std::endl;
		}
		else if (command.compare("DEMO2") == 0)
		{
			std::cout << "Got DEMO2 command on process " << EasyMPI::EasyMPI::getProcessID() 
				<< " with message: " << message << std::endl;
		}
		else if (command.compare("FINISH") == 0)
		{
			std::cout << "Got the FINISH command on process " << EasyMPI::EasyMPI::getProcessID()
				<< ". Exiting slave loop..." << std::endl;
			break;
		}
		else
		{
			std::cout << "Invalid command." << std::endl;
		}
	}
}