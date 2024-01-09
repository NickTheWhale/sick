#include <gui/filter_editor.h>

#include <vector>
#include <iostream>

#include <imnodes.h>
#include <imgui.h>

#include <gui/filter_factory.h>

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
	// user input
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup("###popup");

	if (ImGui::BeginPopup("###popup"))
	{
		ImGui::SeparatorText("Filter type");
		for (const auto& filter : filter_factory::types)
		{
			if (ImGui::Button(filter.first.c_str()))
			{
				add_node(filter.first);
			}
		}

		ImGui::EndPopup();
	}

	ImGui::Begin("node editor");
	imnodes::BeginNodeEditor();

	// draw nodes
	for (const auto& [node_id, node] : _graph.nodes())
	{
		const float node_width = ImGui::CalcTextSize(node.filter->type().c_str()).x;

		imnodes::BeginNode(node_id);

		imnodes::BeginNodeTitleBar();
		ImGui::Text(node.filter->type().c_str());
		imnodes::EndNodeTitleBar();

		{
			imnodes::BeginInputAttribute(node.in_id);
			const float label_width = ImGui::CalcTextSize("in").x;
			ImGui::TextUnformatted("in");
			imnodes::EndInputAttribute();
		}

		ImGui::Spacing();

		{
			imnodes::BeginOutputAttribute(node.out_id);
			const float label_width = ImGui::CalcTextSize("out").x;
			ImGui::Indent(node_width - label_width);
			ImGui::TextUnformatted("out");
			imnodes::EndOutputAttribute();
		}

		imnodes::EndNode();
	}

	// draw links
	for (const auto& [link_id, link] : _graph.links())
	{
		imnodes::Link(link.id, link.in_id, link.out_id);
	}

	imnodes::EndNodeEditor();

	// handle links
	int start, end;
	if (imnodes::IsLinkCreated(&start, &end))
	{
		_graph.add_link({ .id = ++curr_id, .in_id = start, .out_id = end });
	}

	int link_id;
	if (imnodes::IsLinkDestroyed(&link_id))
		_graph.remove_link(link_id);

	const int num_selected_links = imnodes::NumSelectedLinks();
	if (num_selected_links > 0 && ImGui::IsKeyReleased(ImGuiKey::ImGuiKey_Delete))
	{
		std::vector<int> sel_links;
		sel_links.resize(static_cast<size_t>(num_selected_links));
		imnodes::GetSelectedLinks(sel_links.data());
		for (const auto& link_id : sel_links)
			_graph.remove_link(link_id);
	}

	const int num_selected_nodes = imnodes::NumSelectedNodes();
	if (num_selected_nodes > 0 && ImGui::IsKeyReleased(ImGuiKey::ImGuiKey_Delete))
	{
		std::vector<int> sel_nodes;
		sel_nodes.resize(static_cast<size_t>(num_selected_nodes));
		imnodes::GetSelectedNodes(sel_nodes.data());

		for (const auto& node_id : sel_nodes)
			_graph.remove_node(node_id);
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
		std::move(filter)
	);

	return _graph.add_node(std::move(node));
}

const bool editor::filter_editor::remove_node(const int id)
{
	return false;
}

const std::optional<filter_pipeline> editor::filter_editor::pipeline() const
{
	return std::optional<filter_pipeline>();
}
