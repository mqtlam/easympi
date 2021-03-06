easympi
=======

Easily add parallel processing to your C++ applications.

This is intended for master-slave architectures where the slave processes perform certain tasks in parallel while the master process is responsible for assigning these tasks to available slave processes. The master primarily handles communications between it and slaves. The slaves primarily perform the tasks in parallel.

Please see Demo.cpp for a quick start example. There are two tasks. The master is responsible for assigning these two tasks to slaves.

The function initialize() must be called at the beginning of the program and finalize() must be called right when the program ends.

The master process needs a list of tasks to send to the slave. A task is defined as a command string and a string of parameters. The parameter string is optional and attaches additional information to a command. For example, one can create a task with command "PROCESSIMAGE" and message "123" to tell the slave to process image 123. Note: commands and parameter strings may not contain the ';' symbol!

The call to the master process scheduler is masterScheduleTasks(). The slave receives these commands and processes them accordingly. The function to wait for a message from master is slaveWaitForTasks(). The function to signal to the master that the slave is finished is slaveFinishedTask().

Improvements and corrections are welcomed.
