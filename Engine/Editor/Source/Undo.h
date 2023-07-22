#pragma once

#include <vector>
#include <memory>
#include <string>

namespace toy::editor
{
	struct UndoAction
	{
		virtual void redo() = 0;
		virtual void undo() = 0;
		virtual std::string toString() { return std::string{}; }
		virtual ~UndoAction() {}
	};

	class Undo
	{
	public:

		Undo()
		{

		}

		void pushAction(std::unique_ptr<UndoAction> action)
		{
			for (int i = undoHistory_.size(); i > currentPointer_; i--)
			{
				undoHistory_.erase(undoHistory_.end()-1);
			}

			undoHistory_.push_back(std::move(action));
			currentPointer_++;
		}

		void undo()
		{
			if (currentPointer_ > 0)
			{
				currentPointer_--;
				auto action = undoHistory_[currentPointer_].get();
				action->undo();
				
			}
		}
		void redo()
		{
			if (currentPointer_ < undoHistory_.size())
			{
				auto action = undoHistory_[currentPointer_].get();
				action->redo();
				currentPointer_++;
			}
		}

		inline auto getUndoHistory() -> const std::vector<std::unique_ptr<UndoAction>>& const
		{
			return undoHistory_;
		}

		inline int pointer() { return currentPointer_; }

	private:

		int currentPointer_{ 0 };
		
		std::vector<std::unique_ptr<UndoAction>> undoHistory_;
	};
}