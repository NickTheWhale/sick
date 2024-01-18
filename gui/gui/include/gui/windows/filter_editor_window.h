#pragma once

#include <string>	
#include <vector>

#include "gui/windows/window_base.h"
#include "common/filter_pipeline.h"
#include "gui/filter_graph.h"
		 
#include "json.hpp"

namespace window
{
	class filter_editor_window : public window_base
	{
	public:
		filter_editor_window(const char* name, bool* p_open = (bool*)0, ImGuiWindowFlags flags = 0);
		~filter_editor_window() override;
		const bool create_pipeline(filter::filter_pipeline& pipeline) const;

	protected:
		void render_content() override;

	private:
		int curr_id;
		filter::filter_graph _graph;

		void handle_input();
		void render_nodes();
		void render_node_inputs(const nlohmann::json& json, nlohmann::json& new_json);
		void render_links();
		void handle_link_changes();
		void handle_node_changes();
		void handle_load_and_save();

		const bool add_node(const std::string& filter_type);
	};
}