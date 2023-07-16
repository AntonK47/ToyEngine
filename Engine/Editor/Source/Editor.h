#pragma once

namespace toy::editor
{

	class Undo
	{
	public:
		void redo();
		void undo();
	};

	class Document
	{
	public:
		void save();
		void build();
	};

	class Asset
	{
		void load();
		void reload();
	};

	class EngineModule
	{
		
	};

	class Editor
	{
	public:
		void drawGui()
		{
			onDrawGui();
		}
	protected:

		virtual void onDrawGui() = 0;
	};
}