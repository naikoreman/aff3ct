#include "Tools/Exception/exception.hpp"
#include "Tools/Algo/Tree/Binary_tree.hpp"

namespace aff3ct
{
namespace tools
{
template <typename T>
Binary_tree<T>
::Binary_tree(int depth)
: depth(depth), root(nullptr)
{
	std::vector<int> lanes(depth +1);
	for (unsigned i = 0; i < lanes.size(); i++)
		lanes[i] = 0;

	auto cur_depth = 0;
	this->root = new Binary_node<T>(nullptr, nullptr, nullptr, nullptr, cur_depth, lanes[cur_depth]);

	this->create_nodes(this->root, cur_depth +1, lanes);

	recursive_get_leaves(this->get_root());
}

template <typename T>
Binary_tree<T>
::~Binary_tree()
{
	this->delete_nodes(this->root);
}

template <typename T>
Binary_node<T>* Binary_tree<T>
::get_root() const
{
	return this->root;
}

template <typename T>
void Binary_tree<T>
::create_nodes(Binary_node<T>* cur_node, int cur_depth, std::vector<int> &lanes)
{
	if (cur_node->left != nullptr)
		throw runtime_error(__FILE__, __LINE__, __func__, "'cur_node->left' can't be null.");
	if (cur_node->right != nullptr)
		throw runtime_error(__FILE__, __LINE__, __func__, "'cur_node->right' can't be null.");

	if (cur_depth < this->depth)
	{
		cur_node->left  = new Binary_node<T>(cur_node, nullptr, nullptr, nullptr, cur_depth, lanes[cur_depth]++);
		cur_node->right = new Binary_node<T>(cur_node, nullptr, nullptr, nullptr, cur_depth, lanes[cur_depth]++);

		this->create_nodes(cur_node->left,  cur_depth +1, lanes);
		this->create_nodes(cur_node->right, cur_depth +1, lanes);
	}
}

template <typename T>
void Binary_tree<T>
::delete_nodes(Binary_node<T>* cur_node)
{
	if(cur_node != nullptr)
	{
		this->delete_nodes(cur_node->left);
		this->delete_nodes(cur_node->right);

		delete cur_node;
	}
}

template<typename T>
void Binary_tree<T>
::recursive_get_leaves(Binary_node<T>* cur_node){
	if(cur_node->is_leaf())
		leaves.push_back(cur_node);
	else
	{
		recursive_get_leaves(cur_node->get_left());
		recursive_get_leaves(cur_node->get_right());
	}
}

template<typename T>
std::vector<Binary_node<T>*> Binary_tree<T>
::get_leaves() const{
	return this->leaves;
}
}
}
