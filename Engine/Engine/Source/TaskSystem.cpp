#include "TaskSystem.h"
#include <Core.h>
#include <Windows.h>
#include <rigtorp/MPMCQueue.h>
#include <semaphore>
#include <format>
#include <iostream>
#include <array>

namespace
{
	using namespace toy::core;

	struct WorkerThread
	{
		std::jthread thread;
		u8 tagFlag{ WorkerTag::common };
	};

	thread_local u8 tagFlag{ WorkerTag::common };

	std::vector<WorkerThread> workers_;

	//TODO: need separate queue for u io, ui, and low priority background tasks
	
	alignas(cacheSize) std::unordered_map<std::thread::id, u8> tagsPerThreads;

	rigtorp::MPMCQueue<Task*> queue_{100};
	std::stop_source stopSource_;
	std::counting_semaphore<std::numeric_limits<u32>::max()> semaphore_{0};

	rigtorp::MPMCQueue<Task*> rhiQueue_{10};
	rigtorp::MPMCQueue<Task*> ioQueue_{10};
	rigtorp::MPMCQueue<Task*> backgroundQueue_{40};


	bool tryProcessQueue(rigtorp::MPMCQueue<Task*>& queue)
	{
		auto result = false;
		Task* task;
		if (queue.try_pop(task))
		{
			if (task->tryAcquire())
			{
				if (!task->isDone())
				{
					task->taskFunction();
				}
				task->release();
				result = true;
			}
		}

		return result;
	}

	void workerProcedure()
	{
		while (!stopSource_.get_token().stop_requested())
		{
			const auto tags = tagFlag;

			semaphore_.acquire();
			auto couldGetATask = false;
			auto areAllQueuesEmpty = false;
			while (!areAllQueuesEmpty)
			{
				if (tags & WorkerTag::rhi)
				{
					couldGetATask |= tryProcessQueue(rhiQueue_);
					areAllQueuesEmpty = areAllQueuesEmpty && rhiQueue_.empty();
				}

				if (tags & WorkerTag::io)
				{
					couldGetATask |= tryProcessQueue(ioQueue_);
					areAllQueuesEmpty = areAllQueuesEmpty && ioQueue_.empty();
				}

				if (tags & WorkerTag::common)
				{
					couldGetATask |= tryProcessQueue(queue_);
					areAllQueuesEmpty = areAllQueuesEmpty && queue_.empty();
				}

				if (tags & WorkerTag::background)
				{
					couldGetATask |= tryProcessQueue(backgroundQueue_);
					areAllQueuesEmpty = areAllQueuesEmpty && backgroundQueue_.empty();
				}

				areAllQueuesEmpty = queue_.empty() && ioQueue_.empty() && rhiQueue_.empty() && backgroundQueue_.empty();

			}
		}
	}

	
}

namespace toy::core
{
	void TaskSystem::initialize(const TaskSystemDescriptor& descriptor)
	{
		const auto workerCount = std::thread::hardware_concurrency()-1;

		auto tags = std::array { WorkerTag::background, WorkerTag::io, WorkerTag::rhi };
		//assign worker tags
		u32 threadIndex = 0;
		auto perWorkerTags = std::vector<u8>{};
		perWorkerTags.resize(workerCount);

		for (auto i = core::u32{}; i < workerCount; i++)
		{
			perWorkerTags[i] = WorkerTag::common;
		}

		for (auto i = core::u32{}; i < tags.size(); i++)
		{
			perWorkerTags[threadIndex] |= tags[i];
			threadIndex++;
			if (threadIndex == workers_.size())
			{
				threadIndex -= 1;
			}
		}

		workers_.reserve(workerCount);


#ifdef WIN32
		{
			const auto result = SetThreadDescription(GetCurrentThread(), L"Worker 0 [Main]");
			TOY_ASSERT(SUCCEEDED(result));
		}
#endif

		for (auto i = core::u32{}; i < workerCount; i++)
		{
			const auto tag = perWorkerTags[i];
			workers_.emplace_back(WorkerThread{ .thread = std::jthread{ [tag]() { tagFlag = tag; workerProcedure(); }} });
#ifdef WIN32
			{
				const auto result = SetThreadDescription(workers_.back().thread.native_handle(), std::format(L"Worker {}", i + 1).c_str());
				TOY_ASSERT(SUCCEEDED(result));
			}
			{
				
				//const auto result = SetThreadAffinityMask(workers_.back().thread.native_handle(), 1 << i);
				//TOY_ASSERT(result != 0);
			}
#endif
		}

		for (auto i = u32{}; i < workers_.size(); i++)
		{
			tagsPerThreads.insert(std::make_pair(workers_[i].thread.get_id(), workers_[i].tagFlag));
		}

		for (auto i = core::u32{}; i < workerCount; i++)
		{
			auto tagsString = std::wstring{};
			if (perWorkerTags[i] & WorkerTag::rhi)
			{
				if (!tagsString.empty())
				{
					tagsString = tagsString.append(L" ");
				}
				tagsString = tagsString.append(L"RHI");
			}

			if (perWorkerTags[i] & WorkerTag::io)
			{
				if (!tagsString.empty())
				{
					tagsString = tagsString.append(L" ");
				}
				tagsString = tagsString.append(L"IO");
			}

			if (perWorkerTags[i] & WorkerTag::background)
			{
				if (!tagsString.empty())
				{
					tagsString = tagsString.append(L" ");
				}
				tagsString = tagsString.append(L"BG");
			}

			if (!tagsString.empty())
			{
				const auto result = SetThreadDescription(workers_[i].thread.native_handle(), std::format(L"Worker {} [{}]", i + 1, tagsString).c_str());
				TOY_ASSERT(SUCCEEDED(result));
			}
		}
		
	}

	void TaskSystem::deinitialize()
	{

		stopSource_.request_stop();

		for (auto i = core::u32{}; i < workers_.size(); i++)
		{
			semaphore_.release();
		}

		workers_.clear();
	}

	Barrier TaskSystem::run(const std::vector<Task*>& tasks, WorkerTag tag)
	{
		TOY_ASSERT(tasks.size() <= 256);
		auto barrier = Barrier{};
		
		for (u8 index = 0; index < tasks.size(); index++)
		{
			barrier.endIndex_ = index + 1;
			barrier.tasks[index] = tasks[index];
			switch (tag)
			{
			case toy::core::common:
				queue_.push(tasks[index]);
				break;
			case toy::core::io:
				ioQueue_.push(tasks[index]);
				break;
			case toy::core::background:
				backgroundQueue_.push(tasks[index]);
				break;
			case toy::core::rhi:
				rhiQueue_.push(tasks[index]);
				break;
			default:
				break;
			}
			//semaphore_.release(tasks.size());
			semaphore_.release(workers_.size());
		}
		barrier.tag = tag;
		return barrier;
	}

	Barrier TaskSystem::run(std::span<Task> tasks, WorkerTag tag)
	{
		TOY_ASSERT(tasks.size() <= 256);
		auto barrier = Barrier{};

		
		for (u8 index = 0; index < tasks.size(); index++)
		{
			barrier.endIndex_ = index + 1;
			barrier.tasks[index] = &tasks[index];
			switch (tag)
			{
			case toy::core::common:
				queue_.push(&tasks[index]);
				break;
			case toy::core::io:
				ioQueue_.push(&tasks[index]);
				break;
			case toy::core::background:
				backgroundQueue_.push(&tasks[index]);
				break;
			case toy::core::rhi:
				rhiQueue_.push(&tasks[index]);
				break;
			default:
				break;
			}
			
			//semaphore_.release(tasks.size());
			semaphore_.release(workers_.size());

		}
		barrier.tag = tag;
		return barrier;
	}

	std::vector<std::thread::id> TaskSystem::renderWorkers()
	{
		auto renderWorkers = std::vector<std::thread::id>{};
		renderWorkers.resize(workers_.size());

		for (auto i = u32{}; i < workers_.size(); i++)
		{
			renderWorkers[i] = workers_[i].thread.get_id();
		}

		return renderWorkers;
	}

	void Barrier::wait()
	{
		auto allDone = false;
		while (!allDone)
		{
			Task* task;
			u32 index = beginIndex_;
			allDone = true;

			if (tagFlag & tag)
			{
				while (index < endIndex_)
				{
					task = tasks[index];
					if (!task->isAcquired() && !task->isDone() && task->tryAcquire())
					{
						
						task->taskFunction();
						task->release();
						allDone &= task->isDone();
						
					}
					index++;
				}
			}

			if (!allDone)
			{
				
				const auto tags = tagFlag;

				//semaphore_.acquire();

				auto couldGetATask = false;

				if (tags & WorkerTag::rhi)
				{
					couldGetATask |= tryProcessQueue(rhiQueue_);
				}

				if (tags & WorkerTag::io)
				{
					couldGetATask |= tryProcessQueue(ioQueue_);

				}

				if (tags & WorkerTag::common)
				{
					couldGetATask |= tryProcessQueue(queue_);
				}

				if (tags & WorkerTag::background)
				{
					couldGetATask |= tryProcessQueue(backgroundQueue_);
				}

				
			}
			//TODO: select next free task if there are undone tasks in the barrier list
			//Then check again, if all task are done
			{
				u32 index = beginIndex_;
				while (index < endIndex_)
				{
					task = tasks[index];
					allDone = allDone && !task->isAcquired() && task->isDone();

					index++;
				}

				//INFO: underwork, worker is starving 
				using namespace std::chrono_literals;
				//std::this_thread::sleep_for(1ns);
			}
			
		}
		
	}

}

void testTaskSystem()
{
	HANDLE curProc = GetCurrentProcess();
	ULONG size;
	GetSystemCpuSetInformation(nullptr, 0, &size, curProc, 0);
	std::vector<SYSTEM_CPU_SET_INFORMATION> cpus;
	cpus.resize(size / sizeof(SYSTEM_CPU_SET_INFORMATION));
	GetSystemCpuSetInformation(cpus.data(), size, &size, curProc, 0);
	
	
	DWORD processors = 0;
	std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer;
	GetLogicalProcessorInformation(buffer.data(), &processors);
	buffer.resize(processors/ sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
	GetLogicalProcessorInformation(buffer.data(), &processors);



	using namespace toy::core;
	using namespace std::chrono_literals;
	TaskSystem ts;
	ts.initialize(TaskSystemDescriptor{});

	auto t1 = Task{};
	auto t2 = Task{};
	t1.taskFunction =[&]() 
		{
			std::this_thread::sleep_for(2s);
			std::cout << "t1" << std::endl;
			auto b = ts.run(std::vector{ &t2, &t2 });
			b.wait();
		};

	t2.taskFunction = [&]()
	{
		std::this_thread::sleep_for(1s);
		std::cout << "t2" << std::endl;
		ts.run(std::vector{ &t1, &t1 });
	};

	ts.run({ &t1 });
	std::this_thread::sleep_for(6s);
	std::cout << "end threading test" << std::endl;
	/*auto barrier = ts.createTask([&]() 
		{
			auto b1 = ts.createTask([&]()
				{
					std::this_thread::sleep_for(10s);
				});
			auto b2 = ts.createTask([&]()
				{
					std::this_thread::sleep_for(10s);
				});
			auto b3 = ts.createTask([&]()
				{
					std::this_thread::sleep_for(5s);
					b1.wait();
					std::this_thread::sleep_for(5s);
				});
			auto b4 = ts.createTask([&]()
				{
					std::this_thread::sleep_for(10s);
					b2.wait();
					std::this_thread::sleep_for(5s);
					b1.wait();
				});
		});*/

	//barrier.wait();
}