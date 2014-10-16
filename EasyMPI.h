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
	 * parallel tasks while the master is responsible for scheduling tasks to the slaves.
	 *
	 * Important API functions:
	 *
	 *	masterScheduleTasks()
	 *	slaveWaitForTasks()
	 *	slaveFinishedTask()
	 *
	 *	initialize()
	 *	finalize()
	 *
	 *	getProcessID()
	 *	getNumProcesses()
	 *
	 * Note that if the number of processes is 1, then this architecture fails.
	 * Simply have logic to perform tasks with only one process.
	 *
	 * Simply include the header file in your program to use these functions.
	 * Make sure MPI is installed on your system.
	 */
	class EasyMPI
	{
	public:
		const static int MAX_MESSAGE_SIZE; //!< Maximum message size
		const static int MAX_NUM_PROCESSES; //!< Maximum number of processes
		const static string MASTER_FINISH_MESSAGE; //!< Master finished message
		const static string SLAVE_FINISH_MESSAGE; //!< Slave finished message

	private:
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
		 * Abort program "cleanly."
		 */
		static void abortMPI(int errcode);

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
		 * Commands and messages may not include the semicolon ';' symbol!
		 *
		 * @param[in] commands List of commands to perform tasks in parallel
		 * @param[in] messages List of corresponding messages for each command
		 */
		static void masterScheduleTasks(vector<string> commands, vector<string> messages);

		/*!
		 * Slave process waits for task commands from master.
		 *
		 * @param[out] command String containing command
		 * @param[out] message String containing message for command
		 */
		static void slaveWaitForTasks(string& command, string& message);

		/*!
		 * Slave process tells master that it is finished with the recent task.
		 */
		static void slaveFinishedTask();

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

	private:
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