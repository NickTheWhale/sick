#include <gui/filter_graph.h>

#include <algorithm>
#include <iostream>

const bool editor::filter_graph::add_node(const editor::Node& node)
{
	return _nodes.emplace(node.id, node).second;
}

const bool editor::filter_graph::remove_node(const int node_id)
{
	auto nit = _nodes.find(node_id);
	if (nit == _nodes.end())
		return false;

	std::vector<int> links_to_remove;

	// remove associated links
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

	_nodes.erase(nit);

	return true;
}

const bool editor::filter_graph::add_link(const editor::Link& link)
{
	return _links.emplace(link.id, link).second;
}

const bool editor::filter_graph::remove_link(const int link_id)
{
	auto it = _links.find(link_id);
	if (it == _links.end())
		return false;

	_links.erase(it);
	
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
