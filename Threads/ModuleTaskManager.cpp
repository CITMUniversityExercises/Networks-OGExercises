#include "ModuleTaskManager.h"


void ModuleTaskManager::threadMain()
{
	while (true)
	{
		// TODO 3:
		// - Wait for new tasks to arrive
		// - Retrieve a task from scheduledTasks
		// - Execute it
		// - Insert it into finishedTasks

		{ // begin critical section

			std::unique_lock<std::mutex> lock(mtx); // Lock mutex

			while (scheduledTasks.size() == 0)
			{
				event.wait(lock); // release mutex while waiting
			}

			Task* task = scheduledTasks.front();
			task->execute();
			scheduledTasks.pop();
			finishedTasks.push(task);
		} // end critical section

	}
}

bool ModuleTaskManager::init()
{
	// TODO 1: Create threads (they have to execute threadMain())

	for (std::thread& thread : threads)
	{
		thread = std::thread(&ModuleTaskManager::threadMain, this);
	}

	return true;
}

bool ModuleTaskManager::update()
{
	// TODO 4: Dispatch all finished tasks to their owner module (use Module::onTaskFinished() callback)

	while (finishedTasks.size() > 0)
	{
		Task* task = finishedTasks.front();
		task->owner->onTaskFinished(task);
		finishedTasks.pop();
	}

	return true;
}

bool ModuleTaskManager::cleanUp()
{
	// TODO 5: Notify all threads to finish and join them

	event.notify_all();

	for (std::thread& thread : threads)
	{
		thread.join();
	}

	// --- Delete all remaining unexecuted tasks, if any ---
	while (scheduledTasks.size() > 0)
	{
		Task* task = scheduledTasks.front();
		delete task;
		scheduledTasks.pop();
	}

	while (finishedTasks.size() > 0)
	{
		Task* task = finishedTasks.front();
		delete task;
		finishedTasks.pop();
	}

	return true;
}

void ModuleTaskManager::scheduleTask(Task *task, Module *owner)
{
	task->owner = owner;
	// TODO 2: Insert the task into scheduledTasks so it is executed by some thread
	scheduledTasks.push(task);
	event.notify_one(); // notify a thread
}
