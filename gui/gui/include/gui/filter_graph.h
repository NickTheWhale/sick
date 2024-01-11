#pragma once

#include <unordered_map>
#include <gui/filter_base.h>

namespace editor
{
	struct Link
	{
		int id;
		int in_id, out_id;
	};

    struct Node
    {
        int id;
        int in_id, out_id;

		Link in_link, out_link;
		bool is_in_linked, is_out_linked;

        std::unique_ptr<filter_base> filter;

		Node(const int id,
			const int in_id,
			const int out_id,
			const Link in_link,
			const Link out_link,
			const bool is_in_linked,
			const bool is_out_linked,
			std::unique_ptr<filter_base> filter) :
			id(id), in_id(in_id), out_id(out_id), in_link(in_link), out_link(out_link), is_in_linked(is_in_linked), is_out_linked(is_out_linked), filter(std::move(filter))
		{}

        Node(const Node& other) :
			id(other.id), in_id(other.in_id), out_id(other.out_id), in_link(other.in_link), out_link(other.out_link), is_in_linked(other.is_in_linked), is_out_linked(other.is_out_linked)
        {
            if (other.filter) {
                filter = other.filter->clone();
            }
        }

		Node(Node&& other) noexcept 
			: id(other.id), in_id(other.in_id), out_id(other.out_id), in_link(other.in_link), out_link(other.out_link), is_in_linked(other.is_in_linked), is_out_linked(other.is_out_linked)
		{
			if (other.filter) {
				filter = other.filter->clone();
			}
		}

		Node& operator=(const Node& other)
		{
			if (this != &other)
			{
				if (other.filter)
				{
					filter = other.filter->clone();
				}
			}

			return *this;
		}
    };


	class filter_graph
	{
	public:
		const bool add_node(const editor::Node& node);
		const bool remove_node(const int node_id);
		const bool add_link(const editor::Link& link);
		const bool remove_link(const int link_id);

		const bool traverse(std::vector<editor::Node>& nodes) const;

		const std::unordered_map<int, Node>& nodes() const;
		const std::unordered_map<int, Link>& links() const;

	private:
		std::unordered_map<int, Node> _nodes;
		
		// used for fast lookup. INTERNAL USE ONLY!
		std::unordered_map<int, Node> _nodes_in_id;
		std::unordered_map<int, Node> _nodes_out_id;

		std::unordered_map<int, Link> _links;
	};
}

