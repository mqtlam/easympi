#ifndef EASYMPI_H
#define EASYMPI_H

#include <mpi.h>
#include <iostream>
#include <string>
#include <vector>

namespace EasyMPI
{
	using namespace std;

	/*!
	 * EasyMPI is a class that implements basic high level parallelism functionality. 
	 * The current version uses a master-slave architecture where the slaves perform 
	 * parallel tasks while the master is responsible for the merging steps 
	 * and overall main computational steps.
	 *
	 * Simply include the header file in your program to use these functions.
	 * Make sure MPI is installed on your system.
	 */
	class EasyMPI
	{
	private:
		const static int MAX_MESSAGE_SIZE; //!< Maximum message size
		const static int MAX_NUM_PROCESSES; //!< Maximum number of processes

		static int processID; //!< Process ID
		static int numProcesses; //!< Number of processes
		static bool initialized; //!< Whether called MPI initialized
		static bool finalized; //!< Whether called MPI finalized
		static MPI_Status* mpiStatus; //!< MPI Status object

	public:
		/*!
		 * Initialize MPI. Must be called before anything else!
		 */
		static void initialize(int argc, char* argv[]);

		/*!
		 * Finalize MPI. Must be called before exiting the program!
		 */
		static void finalize();
		
		/*!
		 * Get the process ID (rank).
		 */
		static int getProcessID();

		/*!
		 * Get the total number of processes.
		 */
		static int getNumProcesses();

		/*!
		 * Get the MPI status object.
		 */
		static MPI_Status* getMPIStatus();

		/*!
		 * Master process schedules tasks (command, message) to slaves.
		 * Exits when all tasks have been completed.
		 *
		 * @param[in] commands List of commands to perform tasks
		 * @param[in] messages List of corresponding messages for each command
		 * @param[in] finishCommand String for telling every slave to finish after all tasks completed
		 */
		static void masterScheduleTasks(vector<string> commands, vector<string> messages, string finishCommand);

		/*!
		 * Slave process waits for task commands from master.
		 *
		 * @param[out] command String containing command
		 * @param[out] message String containing message for command
		 */
		static void slaveWaitForTasks(string& command, string& message);

		/*!
		 * All processes must reach this point before continuing.
		 *
		 * @param[in] slaveBroadcastMsg Message to broadcast to slaves
		 * @param[in] masterBroadcastMsg Message to broadcast to master
		 */
		static void synchronize(string slaveBroadcastMsg, string masterBroadcastMsg);

		/*!
		 * The master waits until all slave processes 
		 * reach this point before continuing. 
		 * Slave processes continue past this point.
		 *
		 * DEPRECATED.
		 *
		 * @param[in] slaveBroadcastMsg Message to broadcast to slaves
		 */
		static void masterWait(string slaveBroadcastMsg);
		
		/*!
		 * Slave processes wait until the master process 
		 * reaches this point before continuing.
		 * The master process continues past this point.
		 *
		 * DEPRECATED.
		 *
		 * @param[in] masterBroadcastMsg Message to broadcast to master
		 */
		static void slavesWait(string masterBroadcastMsg);

	public:
		/*!
		 * Construct full message.
		 * size<commandstring;messagestring>
		 *
		 * @param[in] command Command string
		 * @param[in] message Message string
		 * @return Full message
		 */
		static string constructFullMessage(string command, string message);

		/*!
		 * Parse full message.
		 * size<commandstring;messagestring>
		 *
		 * @param[in] fullMessage Full message
		 * @param[out] command Command string
		 * @param[out] message Message string
		 * @return true if message parsed successfully
		 */
		static bool parseFullMessage(string fullMessage, string& command, string& message);
	};
}

#endif