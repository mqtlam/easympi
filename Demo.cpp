#include "EasyMPI.h"
#include <iostream>
#include <string>

void slaveDemo(std::vector<EasyMPI::Task> taskList);

int main(int argc, char* argv[])
{
	// initialize MPI: MUST CALL THIS BEFORE ANYTHING ELSE IN main()
	EasyMPI::MPIScheduler::initialize(argc, argv);

	// print rank and number of processes
	std::cout << "Rank=" << EasyMPI::MPIScheduler::getProcessID() << std::endl;
	std::cout << "Size=" << EasyMPI::MPIScheduler::getNumProcesses() << std::endl;

	// declare demo commands and their parameters
	std::vector<EasyMPI::Task> taskList;
	taskList.push_back(EasyMPI::Task("DEMO1", "params1"));
	taskList.push_back(EasyMPI::Task("DEMO2", "params2"));

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
			task = EasyMPI::MPIScheduler::slaveWaitForTasks();
		}
		else
		{
			if (taskList.empty())
				break;

			task = taskList.back();
			taskList.pop_back();
		}

		std::cout << "Got command '" << task.getCommand() << "' and parameters '" << task.getParameters() << "'" << std::endl;

		// define branches here to perform task depending on command
		if (task.getCommand().compare("DEMO1") == 0)
		{
			std::cout << "Got DEMO1 command on process " << EasyMPI::MPIScheduler::getProcessID() 
				<< " with parameters: " << task.getParameters() << std::endl;

			//
			// do stuff like call another function
			//

			// declare finished
			EasyMPI::MPIScheduler::slaveFinishedTask();
		}
		else if (task.getCommand().compare("DEMO2") == 0)
		{
			std::cout << "Got DEMO2 command on process " << EasyMPI::MPIScheduler::getProcessID() 
				<< " with parameters: " << task.getParameters() << std::endl;

			//
			// do stuff like call another function
			//

			// declare finished
			EasyMPI::MPIScheduler::slaveFinishedTask();
		}
		else if (task.getCommand().compare(EasyMPI::MPIScheduler::MASTER_FINISH_COMMAND) == 0)
		{
			std::cout << "Got the master finish command on process " << EasyMPI::MPIScheduler::getProcessID()
				<< ". Exiting slave loop..." << std::endl;

			break;
		}
		else
		{
			std::cout << "Invalid command." << std::endl;
		}
	}
}
