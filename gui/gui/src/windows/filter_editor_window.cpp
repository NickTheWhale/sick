#include <gui/windows/filter_editor_window.h>
#include <gui/filter_factory.h>

#include <spdlog/spdlog.h>

#include <imnodes.h>

#include <ImGuiFileDialog.h>

window::filter_editor_window::filter_editor_window(const char* name, bool* p_open, ImGuiWindowFlags flags)
	: window_base(name, p_open, flags), curr_id(0), _graph({})
{
}

window::filter_editor_window::~filter_editor_window()
{
}

const bool window::filter_editor_window::create_pipeline(filter::filter_pipeline& pipeline) const
{
	if (_graph.nodes().empty())
		return true;

	std::vector<filter::Node> filter_nodes;
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

void window::filter_editor_window::render_content()
{
	ImNodes::BeginNodeEditor();

	handle_input();
	render_nodes();
	render_links();
	ImNodes::MiniMap();

	ImNodes::EndNodeEditor();

	handle_link_changes();
	handle_node_changes();
	handle_load_and_save();
}

void window::filter_editor_window::handle_input()
{
	if (ImNodes::IsEditorHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup("###add_node_popup");

	bool node_added = false;
	if (ImGui::BeginPopup("###add_node_popup"))
	{
		ImGui::SeparatorText("Filter type");
		for (const auto& filter : filter::types)
		{
			if (ImGui::Button(filter.first.c_str()))
			{
				node_added = add_node(filter.first);
			}
		}

		ImGui::EndPopup();
	}
}

void window::filter_editor_window::render_nodes()
{
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
			render_node_inputs(node.filter->to_json(), new_parameters);
			node.filter->load_json(new_parameters);
		}
		ImNodes::EndNode();
	}
}

void window::filter_editor_window::render_node_inputs(const nlohmann::json& json, nlohmann::json& new_json)
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
					new_json[item.key()] = value;
				}
				else if (item.value().is_number_integer())
				{
					int value = item.value().get<int>();
					ImGui::InputInt(item.key().c_str(), &value);
					new_json[item.key()] = value;
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
		nlohmann::json j = { {"parameters",  new_json.unflatten() }, {"type", type} };
		new_json = j;
	}
	catch (const std::exception& e)
	{
		spdlog::get("ui")->error(e.what());
	}
}

void window::filter_editor_window::render_links()
{
	for (const auto& [link_id, link] : _graph.links())
	{
		ImNodes::Link(link.id, link.in_id, link.out_id);
	}
}

void window::filter_editor_window::handle_link_changes()
{
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
}

void window::filter_editor_window::handle_node_changes()
{
	const int num_selected_nodes = ImNodes::NumSelectedNodes();
	if (num_selected_nodes > 0 && ImGui::IsKeyReleased(ImGuiKey::ImGuiKey_Delete))
	{
		std::vector<int> sel_nodes;
		sel_nodes.resize(static_cast<size_t>(num_selected_nodes));
		ImNodes::GetSelectedNodes(sel_nodes.data());
		for (const auto& node_id : sel_nodes)
			_graph.remove_node(node_id);
	}
}

void window::filter_editor_window::handle_load_and_save()
{
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
			filter::filter_pipeline pipeline;
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
}

const bool window::filter_editor_window::add_node(const std::string& filter_type)
{
	std::unique_ptr<filter::filter_base> filter = filter::create(filter_type);
	if (!filter)
		return false;

	filter::Node node(
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
