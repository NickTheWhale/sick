#pragma once

#include <string>
#include <vector>
#include <gui/filter_pipeline.h>
#include <gui/filter_graph.h>

namespace editor
{
	class filter_editor
	{
	public:
		filter_editor(const std::string& editor_name = "");
		~filter_editor();

		const void show();
		const bool add_node(const std::string& type);
		const bool remove_node(const int id);
		const std::optional<filter_pipeline> pipeline() const;

	private:
		const std::string editor_name;
		//int curr_node_id;
		//int curr_link_id;
		
		int curr_id;

		filter_graph _graph;
	};
}

