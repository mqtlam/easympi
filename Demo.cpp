#include "EasyMPI.h"
#include <iostream>
#include <string>
#include <algorithm>

void slaveDemo(std::vector<EasyMPI::Task> taskList);

/*!
 * This demo has the master send two tasks SIMPLE_DEMO and PARAM_LIST_DEMO to slaves. 
 * SIMPLE_DEMO uses a simple parameter string. 
 * PARAM_LIST_DEMO uses a list of parameters.
 */
int main(int argc, char* argv[])
{
	// initialize MPI: MUST CALL THIS BEFORE ANYTHING ELSE IN main()
	EasyMPI::MPIScheduler::initialize(argc, argv);

	// print rank and number of processes
	std::cout << "Rank=" << EasyMPI::MPIScheduler::getProcessID() << std::endl;
	std::cout << "Size=" << EasyMPI::MPIScheduler::getNumProcesses() << std::endl << std::endl;

	// set up parameter list for PARAM_LIST_DEMO
	std::vector<std::string> paramList;
	paramList.push_back("parameter 1");
	paramList.push_back("parameter 2");
	std::string paramString = EasyMPI::ParameterTools::constructParameterString(paramList);

	// declare demo commands and their parameters
	std::vector<EasyMPI::Task> taskList;
	taskList.push_back(EasyMPI::Task("SIMPLE_DEMO", "this is a parameter string"));
	taskList.push_back(EasyMPI::Task("PARAM_LIST_DEMO", paramString));

	// begin master/slave demo
	// this also handles the case when number of processes is 1
	if (EasyMPI::MPIScheduler::getProcessID() == 0 && EasyMPI::MPIScheduler::getNumProcesses() > 1)
	{
		// run scheduler if master
		EasyMPI::MPIScheduler::masterScheduleTasks(taskList);
	}
	else
	{
		// run if slave or number of processes is 1
		std::reverse(taskList.begin(), taskList.end()); // if you want to preserve task ordering
		slaveDemo(taskList);
	}

	// finalize: anything called after this cannot use MPI
	EasyMPI::MPIScheduler::finalize();

	return 0;
}

// The slave is responsible for checking (command, parameters) sent by the master.
// Simply create logic to handle different command cases.
// This also handles for when number of processes is 1.
void slaveDemo(std::vector<EasyMPI::Task> taskList)
{
	// loop to wait for tasks
	while (true)
	{
		// wait for a task
		EasyMPI::Task task;

		// wait for task if more than one process
		// otherwise if only one process, then perform task on master process
		if (EasyMPI::MPIScheduler::getNumProcesses() > 1)
		{
			// block until a task is received from the master
			task = EasyMPI::MPIScheduler::slaveWaitForTasks();
		}
		else
		{
			// if only one process is available, the master also processes tasks
			if (taskList.empty())
				break;

			// note: gets task from the back
			task = taskList.back();
			taskList.pop_back();
		}

		// print message received
		std::cout << "Got command '" << task.getCommand() << "' and parameters '" << task.getParameters() << "' from master" << std::endl;

		// *** define branches here to perform task depending on command ***
		if (task.getCommand().compare("SIMPLE_DEMO") == 0)
		{
			// output command and parameter string
			std::cout << "Got SIMPLE_DEMO command on process " << EasyMPI::MPIScheduler::getProcessID() 
				<< " with parameter string: '" << task.getParameters() << "'" << std::endl;
			std::cout << std::endl;

			// ... do other stuff ...

			// declare finished
			EasyMPI::MPIScheduler::slaveFinishedTask();
		}
		else if (task.getCommand().compare("PARAM_LIST_DEMO") == 0)
		{
			std::string paramString = task.getParameters();
			std::vector<std::string> paramList = EasyMPI::ParameterTools::parseParameterString(paramString);
			int numParameters = paramList.size();

			// output command and parameter list
			std::cout << "Got PARAM_LIST_DEMO command on process " << EasyMPI::MPIScheduler::getProcessID() 
				<< " with " << numParameters << " parameters: " << std::endl;
			for (std::vector<std::string>::iterator it = paramList.begin(); it != paramList.end(); it++)
				std::cout << "\t" << *it << std::endl;
			std::cout << std::endl;

			// ... do other stuff ...

			// declare finished
			EasyMPI::MPIScheduler::slaveFinishedTask();
		}
		else if (task.getCommand().compare(EasyMPI::MPIScheduler::MASTER_FINISH_COMMAND) == 0)
		{
			std::cout << "Got the master finish command on process " << EasyMPI::MPIScheduler::getProcessID()
				<< ". Exiting slave loop..." << std::endl;

			// this branch is crucial to exit the slave loop
			break;
		}
		else
		{
			std::cout << "Invalid command." << std::endl;
		}
	}
}
