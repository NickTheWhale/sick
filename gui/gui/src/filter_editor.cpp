#include <gui/filter_editor.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

#include <imnodes.h>
#include <imgui.h>

#include <gui/filter_factory.h>

#include <json.hpp>

#include <spdlog/spdlog.h>

#include <ImGuiFileDialog.h>


editor::filter_editor::filter_editor(const std::string& editor_name) :
	//curr_node_id(0), curr_link_id(0)
	curr_id(0)
{
}

editor::filter_editor::~filter_editor()
{
}

const void editor::filter_editor::show()
{
	ImGui::Begin("Filter Editor");
	ImNodes::BeginNodeEditor();

	// user input
	if (ImNodes::IsEditorHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup("###add_node_popup");
	
	bool node_added = false;
	if (ImGui::BeginPopup("###add_node_popup"))
	{
		ImGui::SeparatorText("Filter type");
		for (const auto& filter : filter_factory::types)
		{
			if (ImGui::Button(filter.first.c_str()))
			{
				node_added = add_node(filter.first);
			}
		}

		ImGui::EndPopup();
	}

	// draw nodes
	for (const auto& [node_id, node] : _graph.nodes())
	{
		ImNodes::BeginNode(node_id);

		ImNodes::BeginNodeTitleBar();
		ImGui::Text(node.filter->type().c_str());
		ImNodes::EndNodeTitleBar();

		{
			ImNodes::BeginInputAttribute(node.in_id);
			ImNodes::EndInputAttribute();
		}

		ImGui::Spacing();

		{
			ImNodes::BeginOutputAttribute(node.out_id);
			ImNodes::EndOutputAttribute();
		}

		{
			nlohmann::json new_parameters;
			draw_node_inputs(node.filter->to_json(), new_parameters);
			node.filter->load_json(new_parameters);
		}
		ImNodes::EndNode();
	}

	ImGui::ShowDemoWindow();
	ImGui::ShowIDStackToolWindow();

	// draw links
	for (const auto& [link_id, link] : _graph.links())
	{
		ImNodes::Link(link.id, link.in_id, link.out_id);
	}

	ImNodes::MiniMap();
	ImNodes::EndNodeEditor();

	// handle adding links
	int start, end;
	if (ImNodes::IsLinkCreated(&start, &end))
	{
		// ensure only one input/output per node
		bool can_add = true;
		for (const auto& [link_id, link] : _graph.links())
		{
			if (start == link.in_id || end == link.out_id)
			{
				can_add = false;
				break;
			}
		}
		
		// prevent cycles: 
		//  if adding the link would create equal number of nodes and links, 
		//  we would create a cycle
		if (_graph.links().size() + 1 > _graph.nodes().size() - 1)
			can_add = false;

		if (can_add)
			_graph.add_link({ .id = ++curr_id, .in_id = start, .out_id = end });
	}

	// handle removing links and/or nodes
	int link_id;
	if (ImNodes::IsLinkDestroyed(&link_id))
		_graph.remove_link(link_id);

	const int num_selected_links = ImNodes::NumSelectedLinks();
	if (num_selected_links > 0 && ImGui::IsKeyReleased(ImGuiKey::ImGuiKey_Delete))
	{
		std::vector<int> sel_links;
		sel_links.resize(static_cast<size_t>(num_selected_links));
		ImNodes::GetSelectedLinks(sel_links.data());
		for (const auto& link_id : sel_links)
			_graph.remove_link(link_id);
	}

	const int num_selected_nodes = ImNodes::NumSelectedNodes();
	if (num_selected_nodes > 0 && ImGui::IsKeyReleased(ImGuiKey::ImGuiKey_Delete))
	{
		std::vector<int> sel_nodes;
		sel_nodes.resize(static_cast<size_t>(num_selected_nodes));
		ImNodes::GetSelectedNodes(sel_nodes.data());
		for (const auto& node_id : sel_nodes)
			_graph.remove_node(node_id);
	}

	// show_debug_windows();


	// load / save
	if (ImGui::Button("Save filters to file"))
	{
		IGFD::FileDialogConfig config;
		config.path = ".";
		ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", "", config);
	}

	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string path = ImGuiFileDialog::Instance()->GetFilePathName();
			// action
			filter_pipeline pipeline;
			if (create_pipeline(pipeline))
			{
				nlohmann::json j = pipeline.to_json();
				std::ofstream file(path);
				file << j.dump(4);
			}
			else
			{
				spdlog::get("ui")->error("Failed to save filters to file: invalid filter pipeline");
			}
		}
		// close
		ImGuiFileDialog::Instance()->Close();

	}

	ImGui::End();
}

const bool editor::filter_editor::add_node(const std::string& type)
{
	std::unique_ptr<filter_base> filter = filter_factory::create(type);
	if (!filter)
		return false;

	Node node(
		++curr_id,
		++curr_id,
		++curr_id,
		{},
		{},
		false,
		false,
		std::move(filter)
	);

	return _graph.add_node(std::move(node));
}

const bool editor::filter_editor::create_pipeline(filter_pipeline& pipeline) const
{
	if (_graph.nodes().empty())
		return true;

	std::vector<Node> filter_nodes;
	if (!_graph.traverse(filter_nodes))
		return false;

	nlohmann::json filter_json;
	for (const auto& node : filter_nodes)
	{
		nlohmann::json json = node.filter->to_json();
		filter_json.push_back(json);
	}
	
	pipeline.load_json(filter_json);

	return true;
}

const void editor::filter_editor::draw_node_inputs(const nlohmann::json& json, nlohmann::json& new_parameters)
{
	try
	{
		const auto& parameters = json["parameters"];
		const auto& type = json["type"];
		const auto& flat = parameters.flatten();
		const auto& items = flat.items();
		for (const auto& item : items)
		{
			ImGui::PushID(item.key().c_str());
			if (item.value().is_number())
			{
				ImGui::PushItemWidth(100.0f);
				if (item.value().is_number_float())
				{
					double value = item.value().get<double>();
					ImGui::InputDouble(item.key().c_str(), &value);
					new_parameters[item.key()] = value;
				}
				else if (item.value().is_number_integer())
				{
					int value = item.value().get<int>();
					ImGui::InputInt(item.key().c_str(), &value);
					new_parameters[item.key()] = value;
				}
				else
				{
					ImGui::Text("Not an integer or float: %s", item.key().c_str());
				}
				ImGui::PopItemWidth();
			}
			else
			{
				ImGui::Text("NAN: %s", item.key().c_str());
			}
			ImGui::PopID();
		}
		nlohmann::json j = { {"parameters",  new_parameters.unflatten() }, {"type", type} };
		new_parameters = j;
	}
	catch (const std::exception& e)
	{
		spdlog::get("ui")->error(e.what());
	}
}

const void editor::filter_editor::show_debug_windows() const
{
	// debugging windows
	const int num_selected_nodes = ImNodes::NumSelectedNodes();
	ImGui::Begin("graph nodes");
	for (const auto& node_pair : _graph.nodes())
	{

		std::vector<int> sel_nodes;
		if (num_selected_nodes > 0) {
			sel_nodes.resize(static_cast<size_t>(num_selected_nodes));
			ImNodes::GetSelectedNodes(sel_nodes.data());
		}

		auto node = node_pair.second;
		bool isNodeSelected = (num_selected_nodes > 0) &&
			(std::find(sel_nodes.begin(), sel_nodes.end(), node.id) != sel_nodes.end());

		if (isNodeSelected) {
			ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(0, 255, 0, 255));
		}

		ImGui::SeparatorText(node.filter->type().c_str());

		if (isNodeSelected) {
			ImGui::PopStyleColor();
		}

		ImGui::Text("id:             %d", node.id);
		ImGui::Text("in_id:          %d", node.in_id);
		ImGui::Text("out_id:         %d", node.out_id);

		ImGui::Text("is_in_linked:  ");
		if (node.is_in_linked)
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(13, 219, 27, 255));
		else
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(219, 13, 13, 255));
		ImGui::SameLine();
		ImGui::Text("%s", node.is_in_linked ? "true" : "false");
		ImGui::PopStyleColor();

		ImGui::Text("is_out_linked: ");
		if (node.is_out_linked)
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(13, 219, 27, 255));
		else
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(219, 13, 13, 255));
		ImGui::SameLine();
		ImGui::Text("%s", node.is_out_linked ? "true" : "false");
		ImGui::PopStyleColor();

		ImGui::Separator();
	}
	ImGui::End();

	ImGui::Begin("filter path");
	std::vector<Node> node_path;
	if (_graph.traverse(node_path))
	{
		for (const auto& node : node_path)
		{

			std::vector<int> sel_nodes;
			if (num_selected_nodes > 0) {
				sel_nodes.resize(static_cast<size_t>(num_selected_nodes));
				ImNodes::GetSelectedNodes(sel_nodes.data());
			}

			bool isNodeSelected = (num_selected_nodes > 0) &&
				(std::find(sel_nodes.begin(), sel_nodes.end(), node.id) != sel_nodes.end());

			if (isNodeSelected) {
				ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(0, 255, 0, 255));
			}

			ImGui::SeparatorText(node.filter->type().c_str());

			if (isNodeSelected) {
				ImGui::PopStyleColor();
			}

			ImGui::Text("id:             %d", node.id);
			ImGui::Text("in_id:          %d", node.in_id);
			ImGui::Text("out_id:         %d", node.out_id);

			ImGui::Text("is_in_linked:  ");
			if (node.is_in_linked)
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(13, 219, 27, 255));
			else
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(219, 13, 13, 255));
			ImGui::SameLine();
			ImGui::Text("%s", node.is_in_linked ? "true" : "false");
			ImGui::PopStyleColor();

			ImGui::Text("is_out_linked: ");
			if (node.is_out_linked)
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(13, 219, 27, 255));
			else
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(219, 13, 13, 255));
			ImGui::SameLine();
			ImGui::Text("%s", node.is_out_linked ? "true" : "false");
			ImGui::PopStyleColor();

			if (node.filter->to_json().contains("parameters"))
			{
				std::string json_str = node.filter->to_json()["parameters"].dump(2);
				ImGui::TextWrapped("%s", json_str.c_str());
			}

			ImGui::Separator();
		}
	}
	ImGui::End();

	ImGui::Begin("pipeline");
	filter_pipeline pipeline;
	if (create_pipeline(pipeline))
	{
		nlohmann::json pipe_json = pipeline.to_json();
		std::string pipe_json_str = pipe_json.dump(2);
		ImGui::TextWrapped(pipe_json_str.c_str());
	}
	ImGui::End();
}
