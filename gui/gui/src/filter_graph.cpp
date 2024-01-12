#include <gui/filter_graph.h>

#include <algorithm>
#include <iostream>
#include <stack>

const bool editor::filter_graph::add_node(const editor::Node& node)
{
	return _nodes.emplace(node.id, node).second 
		&& _nodes_in_id.emplace(node.in_id, node).second 
		&& _nodes_out_id.emplace(node.out_id, node).second;
}

const bool editor::filter_graph::remove_node(const int node_id)
{
	auto nit = _nodes.find(node_id);
	if (nit == _nodes.end())
		return false;

	std::vector<int> links_to_remove;

	// remove associated links
	// TODO: leverage _nodes_*_id maps to improve efficiency
	for (const auto& [link_id, link] : _links)
	{
		const int l_in = link.in_id;
		const int l_out = link.out_id;
		const int n_in = (*nit).second.in_id;
		const int n_out = (*nit).second.out_id;
		
		// not sure if i need to check n_out == l_out or n_in == l_in but it shouldn't hurt
		if (n_out == l_in || n_in == l_out || n_out == l_out || n_in == l_in)
			links_to_remove.push_back(link_id);
	}

	for (const auto& link_id : links_to_remove)
		remove_link(link_id);

	int in_id = nit->second.in_id;
	int out_id = nit->second.out_id;

	_nodes.erase(nit);

	// TODO: check complexity
	_nodes_in_id.erase(in_id);
	_nodes_out_id.erase(out_id);

	return true;
}

const bool editor::filter_graph::add_link(const editor::Link& link)
{
	// configure associated node flags
	_nodes.at(_nodes_in_id.at(link.out_id).id).is_in_linked = true;
	_nodes.at(_nodes_in_id.at(link.out_id).id).in_link = link;

	_nodes.at(_nodes_out_id.at(link.in_id).id).is_out_linked = true;
	_nodes.at(_nodes_out_id.at(link.in_id).id).out_link = link;

	return _links.emplace(link.id, link).second;
}

const bool editor::filter_graph::remove_link(const int link_id)
{
	auto link_it = _links.find(link_id);
	if (link_it == _links.end())
		return false;

	// configure associated node flags
	_nodes.at(_nodes_in_id.at(link_it->second.out_id).id).is_in_linked = false;
	_nodes.at(_nodes_in_id.at(link_it->second.out_id).id).in_link = {};
						   							
	_nodes.at(_nodes_out_id.at(link_it->second.in_id).id).is_out_linked = false;
	_nodes.at(_nodes_out_id.at(link_it->second.in_id).id).out_link = {};

	_links.erase(link_it);
	
	return true;
}

const bool editor::filter_graph::traverse(std::vector<editor::Node>& nodes) const
{
	if (_nodes.empty())
		return false;

	// find start node (node with no input link)
	std::vector<Node> _start_nodes;
	for (const auto& [node_id, node] : _nodes)
		if (!node.is_in_linked)
			_start_nodes.push_back(node);

	// if there isn't exactly 1 start node, then we have dangling nodes or a cycle
	if (_start_nodes.size() != 1)
		return false;

	std::vector<Node> node_path;
	const Node* curr_node = &_start_nodes[0];
	while (curr_node->is_out_linked)
	{
		node_path.push_back(*curr_node);
		const auto out_link = curr_node->out_link;
		const auto out_link_out_id = out_link.out_id;
		const auto* next_node = &_nodes_in_id.at(out_link_out_id);
		next_node = &_nodes.at(next_node->id);
		curr_node = next_node;

		//curr_node = &_nodes_out_id.at(curr_node->out_link.in_id);
		//curr_node = &_nodes.at(_nodes_in_id.at(curr_node->out_link.in_id).id);
	}
	node_path.push_back(*curr_node);

	nodes = std::move(node_path);

	return true;
}

const std::unordered_map<int, editor::Node>& editor::filter_graph::nodes() const
{
	return _nodes;
}

const std::unordered_map<int, editor::Link>& editor::filter_graph::links() const
{
	return _links;
}
