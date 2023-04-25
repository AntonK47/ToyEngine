#pragma once
#include <Core.h>
#include <rigtorp/MPMCQueue.h>
#include <thread>
#include <functional>
#include <unordered_map>
#include <array>
#include <semaphore>

namespace toy::editor
{
	enum class TaskStatus
	{
		unknown = 0,
		queued,
		onProcessing,
		canceled,
		done
	};
	
	struct TaskControl
	{
		auto report(const float progress)
		{
			taskProgress.exchange(progress);
		}

		auto stopRequested() -> bool
		{
			return shouldStop.load();
		}
		std::atomic<float> taskProgress{0.0f};
		std::atomic_bool shouldStop{false};
		std::atomic<core::u32 > status{0};
	};

	class BackgroundTasksSystem final
	{
	private:
		using OnCompute = std::function<void(TaskControl&)>;
		struct Task
		{
			core::UID uid;
			OnCompute onCompute;
		};

	public:
		BackgroundTasksSystem(const core::u32 workerCount = 1, const size_t queueSize = 100) : queue_{queueSize}
		{
			workers_.resize(workerCount);
			for (auto i = core::u32{}; i < workerCount; i++)
			{
				workers_[i] = std::make_unique<std::jthread>([this](std::stop_source stopSource) {
					Task task;
					while (!stopSource.get_token().stop_requested())
					{
						while (queue_.try_pop(task) && !stopSource.get_token().stop_requested())
						{
							semaphore_.acquire();

							get(task.uid)->status.exchange((core::u32)TaskStatus::onProcessing);

							std::stop_callback callback(stopSource.get_token(),
								[&]
								{
									auto p = get(task.uid);
									p->shouldStop.exchange(true);
								});

							task.onCompute(*get(task.uid));

							if (get(task.uid)->shouldStop.load())
							{
								get(task.uid)->status.exchange((core::u32)TaskStatus::canceled);
							}
							else
							{
								get(task.uid)->status.exchange((core::u32)TaskStatus::done);
								get(task.uid)->taskProgress.exchange(1.0f);
							}
						}
					}
				}, stopSource);
			}
			
		}

		auto createTask(toy::core::UID uid, OnCompute onCompute)
		{
			TOY_ASSERT(!progressMap_.contains(uid));

			emplace(std::move(std::make_pair(uid, std::make_unique<TaskControl>())));
			get(uid)->status.store((core::u32)TaskStatus::queued);
			queue_.push(Task{ uid, onCompute });
			semaphore_.release();
		}

		auto requestProgress(const core::UID uid) -> float
		{
			TOY_ASSERT(contains(uid));
			return get(uid)->taskProgress.load();
		}

		auto stopTask(const core::UID uid)
		{
			TOY_ASSERT(contains(uid));
			return get(uid)->shouldStop.exchange(true);
		}

		auto status(const core::UID uid) -> TaskStatus
		{
			TOY_ASSERT(contains(uid));
			return (TaskStatus)get(uid)->status.load();
		}

		auto hasTask(const core::UID uid) -> bool
		{
			return contains(uid);
		}

		auto freeTask(const core::UID uid)
		{
			const auto stat = status(uid);
			TOY_ASSERT(stat != TaskStatus::onProcessing);
			erase(uid);
		}

		~BackgroundTasksSystem()
		{
			stopSource.request_stop();
			workers_.clear();
		}

	private:
		auto get(const core::UID uid) -> TaskControl* 
		{
			return std::as_const(progressMap_).at(uid).get();
		}

		auto contains(const core::UID uid) -> bool
		{
			return progressMap_.contains(uid);
		}

		auto erase(const core::UID uid) -> bool
		{
			
			return progressMap_.erase(uid);
		}

		auto emplace(std::pair<core::UID, std::unique_ptr<TaskControl>>&& value) -> void
		{
			progressMap_.emplace(std::move(value));
		}

		std::vector<std::unique_ptr<std::jthread>> workers_;

		//Reference: https://stackoverflow.com/questions/9685486/unordered-map-thread-safety
		std::unordered_map<core::UID, std::unique_ptr<TaskControl>> progressMap_;
		rigtorp::MPMCQueue<Task> queue_;
		std::counting_semaphore<std::numeric_limits<core::u32>::max()> semaphore_{0};
		std::stop_source stopSource;
	};
}