#pragma once

#include <unordered_map>
#include <gui/filter_base.h>

namespace editor
{
    struct Node
    {
        int id;
        int in_id, out_id;
        std::unique_ptr<filter_base> filter;

		Node(const int id, const int in_id, const int out_id, std::unique_ptr<filter_base> filter) :
			id(id), in_id(in_id), out_id(out_id), filter(std::move(filter))
		{}

        Node(const Node& other) :
            id(other.id), in_id(other.in_id), out_id(other.out_id)
        {
            if (other.filter) {
                filter = other.filter->clone(); // Assuming filter_base has a clone method
            }
        }
    };

	struct Link
	{
		int id;
		int in_id, out_id;
	};

	class filter_graph
	{
	public:
		const bool add_node(const editor::Node& node);
		const bool remove_node(const int node_id);
		const bool add_link(const editor::Link& link);
		const bool remove_link(const int link_id);

		const std::unordered_map<int, Node>& nodes() const;
		const std::unordered_map<int, Link>& links() const;

	private:
		std::unordered_map<int, Node> _nodes;
		std::unordered_map<int, Link> _links;
	};
}

