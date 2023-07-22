#pragma once

namespace toy::editor
{

	

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