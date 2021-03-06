#ifndef EASYMPI_H
#define EASYMPI_H

#include <mpi.h>
#include <iostream>
#include <string>
#include <vector>

namespace EasyMPI
{
	using namespace std;

	// Forward class declarations
	class MPIScheduler;
	class Task;
	class ParameterTools;

	/*!
	 * MPIScheduler is a class that implements basic high level parallelism functionality. 
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
	 * Simply have logic to perform tasks with only one process (included in Demo.cpp).
	 *
	 * Simply include the header file in your program to use these functions. 
	 * Make sure MPI is installed on your system.
	 */
	class MPIScheduler
	{
	public:
		const static int MAX_MESSAGE_SIZE; //!< Maximum message size
		const static int MAX_NUM_PROCESSES; //!< Maximum number of processes
		const static string MASTER_FINISH_COMMAND; //!< Master finished command
		const static string SLAVE_FINISH_COMMAND; //!< Slave finished command
		const static string SYNCHRONIZATION_MASTER_MESSAGE; //!< Master synchronization message
		const static string SYNCHRONIZATION_SLAVE_MESSAGE; //!< Slave synchronization message

	private:
		static int processID; //!< Process ID
		static int numProcesses; //!< Number of processes
		static bool initialized; //!< Whether called MPI initialized
		static bool finalized; //!< Whether called MPI finalized
		static MPI_Status* mpiStatus; //!< MPI Status object
		static int syncCounter; //!< counter of synchronization calls

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
		 * Master process schedules tasks (command, parameters) to slaves.
		 * Exits when all tasks have been completed.
		 *
		 * @param[in] taskList List of tasks to perform in parallel
		 */
		static void masterScheduleTasks(vector<Task> tasksList);

		/*!
		 * Slave process waits for a task from master. 
		 * This function blocks until a task comes from the master.
		 *
		 * @return Task received from master
		 */
		static Task slaveWaitForTasks();

		/*!
		 * Slave process tells master that it is finished with the recent task.
		 */
		static void slaveFinishedTask();

		/*!
		 * All processes must reach this point before continuing. 
		 * Useful command if need to synchronize all processes.
		 */
		static void synchronize();

	private:
		/*!
		 * All processes must reach this point before continuing.
		 *
		 * DEPRECATED for API use.
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
		 * DEPRECATED for API use.
		 *
		 * @param[in] slaveBroadcastMsg Message to broadcast to slaves
		 */
		static void masterWait(string slaveBroadcastMsg);
		
		/*!
		 * Slave processes wait until the master process 
		 * reaches this point before continuing.
		 * The master process continues past this point.
		 *
		 * DEPRECATED for API use.
		 *
		 * @param[in] masterBroadcastMsg Message to broadcast to master
		 */
		static void slavesWait(string masterBroadcastMsg);
	};

	/*!
	 * The Task class encapsulates a command that is sent and received as messages.
	 * A Task consists of a command string and an optional parameter string, where the user can 
	 * add any additional information for the command in the parameter string.
	 *
	 * The Task class also has utilities to convert to a message and back.
	 *
	 */
	class Task
	{
	public:
		const static char MESSAGE_DELIMITER; //!< Delimiter to delimit command and parameters
		const static char MESSAGE_BEGIN_CHAR; //!< Begin message character
		const static char MESSAGE_END_CHAR; //!< End message character
		const static char MESSAGE_SIZE_NUM_CHARS; //!< Number of characters to represent size of message

	protected:
		string command; //!< Command string
		string parameters; //!< Optional string of command parameters

	public:
		/*!
		 * Construct an empty task.
		 * Do not use the empty task!
		 */
		Task();

		/*!
		 * Construct a task. 
		 * Commands may not include the semicolon ';' (MESSAGE_DELIMITER) symbol!
		 */
		Task(string command);
		
		/*!
		 * Construct a task. 
		 * Commands and parameters may not include the semicolon ';' (MESSAGE_DELIMITER) symbol!
		 */
		Task(string command, string parameters);

		/*!
		 * Returns the command.
		 */
		string getCommand() const;

		/*!
		 * Returns the parameters.
		 */
		string getParameters() const;

		/*!
		 * Returns if the command and parameters are empty strings.
		 */
		bool isEmpty() const;

		/*!
		 * Construct message for message passing.
		 *
		 * @param[in] task Task object
		 * @return Message string
		 */
		static string constructFullMessage(Task task);

		/*!
		 * Parse message from message passing.
		 *
		 * @param[in] message Message string
		 * @return Task object
		 */
		static Task parseFullMessage(string message);
	};

	/*!
	 * ParameterTools is a class that provides tools to parse and construct 
	 * parameter strings used in the Task object.
	 *
	 * This is an optional class; it not required to use this class for 
	 * handling parameters. This is especially true if the parameter 
	 * is very simple or not even used.
	 */
	class ParameterTools
	{
	public:
		const static char PARAMETER_DELIMITER; //!< Delimiter used in parsing

	public:
		/*!
		 * Parse a parameter string into a list of parameters. 
		 * Used after extracting the parameter list from the Task object.
		 *
		 * @param[in] parameterString String of parameters separated by a delimiter
		 * @return List of parameters in the order parsed
		 */
		static vector<string> parseParameterString(string parameterString);

		/*!
		 * Construct parameter string from a list of parameters. 
		 * Used before passing to Task object.
		 *
		 * @param[in] parameterList List of parameters
		 * @return String of parameters separated by a delimiter
		 */
		static string constructParameterString(vector<string> parameterList);
	};
}

#endif