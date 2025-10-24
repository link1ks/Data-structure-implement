#define _CRT_SECURE_NO_WARNINGS
#include "bst.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>

#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// 二叉树节点结构
typedef struct Node {
	void* data;           // 节点数据指针
	struct Node* left;    // 左子树指针
	struct Node* right;   // 右子树指针
} Node;

// 用于后序遍历验证的辅助结构
typedef struct Pair {
	long long min;        // 子树最小值
	long long max;        // 子树最大值
} Pair;

// 二叉搜索树主结构
struct BST {
	Node* root;           // 树根节点
	size_t element_size;  // 每个元素的大小（字节）
	size_t size;          // 树中元素个数
	CompareFunc compare;  // 元素比较函数
};

/**
 * @brief 创建新节点
 * @param data 要存储的数据指针
 * @param element_size 数据大小
 * @return 新节点指针，失败返回NULL
 */
static Node* _create_node(const void* data, size_t element_size) {
	Node* new_node = (Node*)malloc(sizeof(Node));
	if (new_node == NULL) {
		printf("Memory Allocation Failed!\n");
		return NULL;
	}

	new_node->data = malloc(element_size);
	if (new_node->data == NULL) {
		free(new_node);
		printf("Memory Allocation Failed!\n");
		return NULL;
	}

	// 拷贝数据到新节点
	memcpy(new_node->data, data, element_size);
	new_node->left = NULL;
	new_node->right = NULL;
	return new_node;
}

/**
 * @brief 递归销毁树（后序遍历方式）
 * @param node 要销毁的子树根节点
 */
static void _destroy_recursive(Node* node) {
	if (node == NULL) {
		return;
	}
	// 后序遍历：先销毁左右子树，再销毁当前节点
	_destroy_recursive(node->left);
	_destroy_recursive(node->right);
	free(node->data);
	free(node);
}

/**
 * @brief 递归插入新节点
 * @param bst BST指针
 * @param node 当前子树根节点
 * @param data 要插入的数据
 * @return 插入后子树的根节点
 */
static Node* _insert_recursive(BST* bst, Node* node, const void* data) {
	// 基本情况：找到插入位置，创建新节点
	if (node == NULL) {
		Node* new_node = _create_node(data, bst->element_size);
		if (new_node) bst->size++;  // 插入成功，增加树大小
		return new_node;
	}

	// 比较当前节点数据与要插入的数据
	int cmp = bst->compare(data, node->data);

	// 新数据小于当前节点，插入左子树
	if (cmp < 0) {
		node->left = _insert_recursive(bst, node->left, data);
	}
	// 新数据大于当前节点，插入右子树
	else if (cmp > 0) {
		node->right = _insert_recursive(bst, node->right, data);
	}
	// 相等时不插入（BST不允许重复值，或者根据具体实现决定）

	return node;  // 返回当前子树的根（可能已更新）
}

/**
 * @brief 查找子树中的最小节点（最左节点）
 * @param node 子树根节点
 * @return 最小节点指针
 */
static Node* _find_min_node(Node* node) {
	// 在BST中，最左边的节点就是最小值
	while (node && node->left) {
		node = node->left;
	}
	return node;
}

/**
 * @brief 查找子树中的最大节点（最右节点）
 * @param node 子树根节点
 * @return 最大节点指针
 */
static Node* _find_max_node(Node* node) {
	// 在BST中，最右边的节点就是最大值
	while (node && node->right) {
		node = node->right;
	}
	return node;
}

/**
 * @brief 递归删除指定键值的节点
 * @param bst BST指针
 * @param node 当前子树根节点
 * @param key 要删除的键值
 * @return 删除后子树的根节点
 */
static Node* _destroy_node(BST* bst, Node* node, const void* key) {
	if (node == NULL) {
		return NULL;  // 未找到要删除的节点
	}

	int cmp = bst->compare(key, node->data);

	// 要删除的节点在左子树
	if (cmp < 0) {
		node->left = _destroy_node(bst, node->left, key);
	}
	// 要删除的节点在右子树
	else if (cmp > 0) {
		node->right = _destroy_node(bst, node->right, key);
	}
	// 找到要删除的节点
	else {
		// 情况1：叶子节点（无子节点）
		if (node->left == NULL && node->right == NULL) {
			free(node->data);
			free(node);
			bst->size--;
			return NULL;
		}
		// 情况2：只有左子节点
		else if (node->right == NULL) {
			Node* temp = node->left;
			free(node->data);
			free(node);
			bst->size--;
			return temp;  // 左子节点接替当前位置
		}
		// 情况3：只有右子节点
		else if (node->left == NULL) {
			Node* temp = node->right;
			free(node->data);
			free(node);
			bst->size--;
			return temp;  // 右子节点接替当前位置
		}
		// 情况4：有两个子节点（最复杂的情况）

		// 找到右子树中的最小节点（中序后继）
		Node* min_node_in_right = _find_min_node(node->right);

		// 用后继节点的数据替换当前节点数据
		memcpy(node->data, min_node_in_right->data, bst->element_size);

		// 递归删除那个后继节点
		node->right = _destroy_node(bst, node->right, min_node_in_right->data);
	}
	return node;
}

// ==================== 公共API实现 ====================

/**
 * @brief 向BST插入元素
 * @param bst BST指针
 * @param element_data 要插入的元素数据
 * @return 插入成功返回true，失败返回false
 */
bool bst_insert(BST* bst, const void* element_data) {
	if (!bst || !element_data) return false;
	size_t old_size = bst->size;
	bst->root = _insert_recursive(bst, bst->root, element_data);
	return bst->size > old_size;  // 如果大小增加，说明插入成功
}

/**
 * @brief 从BST删除元素
 * @param bst BST指针
 * @param element_data 要删除的元素数据
 * @return 删除成功返回true，失败返回false
 */
bool bst_remove(BST* bst, const void* element_data) {
	if (!bst || !element_data) {
		return false;
	}
	size_t old_size = bst->size;
	bst->root = _destroy_node(bst, bst->root, element_data);
	return bst->size < old_size;  // 如果大小减少，说明删除成功
}

/**
 * @brief 在BST中搜索元素
 * @param bst BST指针
 * @param key 要搜索的键值
 * @return 找到返回true，未找到返回false
 */
bool bst_search(const BST* bst, const void* key) {
	if (!bst || !key) return false;
	Node* current = bst->root;

	// 迭代搜索：从根节点开始，根据比较结果选择左子树或右子树
	while (current != NULL) {
		int cmp = bst->compare(key, current->data);
		if (cmp < 0) {
			current = current->left;    // 在左子树中继续搜索
		}
		else if (cmp > 0) {
			current = current->right;   // 在右子树中继续搜索
		}
		else {
			return true;  // 找到匹配的节点
		}
	}
	return false;  // 搜索到叶子节点仍未找到
}

/**
 * @brief 查找包含指定键值的节点（内部使用）
 * @param bst BST指针
 * @param key 要查找的键值
 * @return 找到的节点指针，未找到返回NULL
 */
static Node* bst_find_key(const BST* bst, const void* key) {
	if (!bst || !key) return false;
	Node* current = bst->root;
	while (current != NULL) {
		int cmp = bst->compare(key, current->data);
		if (cmp < 0) {
			current = current->left;
		}
		else if (cmp > 0) {
			current = current->right;
		}
		else {
			break;  // 找到节点，跳出循环
		}
	}
	return current;
}

// ==================== BST验证函数 ====================

/**
 * @brief 前序遍历验证BST性质
 * @param node 当前节点
 * @param left 当前子树允许的最小值
 * @param right 当前子树允许的最大值
 * @return 验证通过返回true，失败返回false
 */
static bool pre_order_traverse(const Node* node, long long left, long long right) {
	if (node == NULL) {
		return true;
	}
	int x = *(const int*)node->data;
	// 检查当前节点值是否在允许范围内，并递归验证左右子树
	return left < x && x < right &&
		pre_order_traverse(node->left, left, x) &&
		pre_order_traverse(node->right, x, right);
}

/**
 * @brief 中序遍历验证BST性质
 * @param node 当前节点
 * @param pre 指向前一个节点值的指针
 * @return 验证通过返回true，失败返回false
 */
static bool in_order_traverse(const Node* node, long long* pre) {
	if (node == NULL) {
		return true;
	}
	// 中序遍历：左子树 -> 当前节点 -> 右子树
	if (!in_order_traverse(node->left, pre)) {
		return false;
	}
	int x = *(const int*)node->data;
	// 检查是否严格递增（BST性质）
	if (x <= *pre) {
		return false;
	}
	*pre = x;  // 更新前一个节点值
	return in_order_traverse(node->right, pre);
}

/**
 * @brief 后序遍历验证BST性质
 * @param node 当前节点
 * @return 包含子树最小最大值的Pair结构
 */
static Pair post_order_traverse(const Node* node) {
	if (node == NULL) {
		return (Pair) { LLONG_MAX, LLONG_MIN };  // 空树的特殊值
	}
	Pair l = post_order_traverse(node->left);   // 左子树信息
	Pair r = post_order_traverse(node->right);  // 右子树信息
	int x = *(const int*)node->data;

	// 检查BST性质：当前节点值应大于左子树最大值，小于右子树最小值
	if (x <= l.max || x >= r.min) {
		return (Pair) { LLONG_MIN, LLONG_MAX };  // 验证失败的标志
	}
	return (Pair) { MIN(l.min, x), MAX(r.max, x) };  // 返回当前子树的范围
}

// ==================== 树深度计算 ====================

/**
 * @brief 计算子树的最大深度
 * @param node 子树根节点
 * @return 最大深度
 */
static int get_max_depth(const Node* node) {
	if (node == NULL) {
		return 0;  // 空树深度为0
	}
	int left_depth = get_max_depth(node->left) + 1;
	int right_depth = get_max_depth(node->right) + 1;
	return MAX(left_depth, right_depth);  // 取左右子树深度的最大值
}

/**
 * @brief 计算子树的最小深度
 * @param node 子树根节点
 * @return 最小深度
 */
static int get_min_depth(const Node* node) {
	if (node == NULL) {
		return 0;
	}
	int left_depth = get_min_depth(node->left) + 1;
	int right_depth = get_min_depth(node->right) + 1;
	// 处理一边子树为空的情况
	return (MIN(left_depth, right_depth) == 1) ? MAX(left_depth, right_depth) : MIN(left_depth, right_depth);
}

// ==================== 第k小元素查询 ====================

/**
 * @brief 递归查找第k小的元素
 * @param node 当前节点
 * @param k 指向剩余查找次数的指针
 * @return 找到的元素值，未找到返回-1
 */
static int get_kth(Node* node, int* k) {
	if (node == NULL) {
		return -1;
	}
	// 中序遍历：先搜索左子树
	int left_res = get_kth(node->left, k);
	if (left_res != -1) {
		return left_res;  // 在左子树中找到
	}
	// 检查当前节点
	if (--(*k) == 0) {
		return *(const int*)node->data;  // 当前节点就是第k小
	}
	// 在右子树中继续搜索
	return get_kth(node->right, k);
}

// ==================== 核心管理函数 ====================

/**
 * @brief 创建新的BST
 * @param element_size 每个元素的大小
 * @param comparefunc 元素比较函数
 * @return 新BST指针，失败返回NULL
 */
BST* bst_create(size_t element_size, CompareFunc comparefunc) {
	if (!element_size || !comparefunc) {
		printf("Invalid!\n");
		return NULL;
	}
	BST* bst = (BST*)malloc(sizeof(BST));
	if (bst == NULL) {
		printf("Memory Allocation Failed!\n");
		return NULL;
	}
	bst->element_size = element_size;
	bst->root = NULL;
	bst->size = 0;
	bst->compare = comparefunc;
	return bst;
}

/**
 * @brief 销毁BST并释放所有内存
 * @param p_bst 指向BST指针的指针
 */
void bst_destroy(BST** p_bst) {
	if (!p_bst || !*p_bst) {
		return;
	}
	BST* bst = *p_bst;
	_destroy_recursive(bst->root);  // 递归销毁所有节点
	free(bst);                      // 释放BST结构本身
	*p_bst = NULL;                  // 将指针设为NULL，避免野指针
}

/**
 * @brief 遍历BST（当前未实现）
 * @param bst BST指针
 * @param order 遍历顺序
 */
void bst_traverse(const BST* bst, TraverseOrder order) {
	if (!bst) {
		return;
	}
	// TODO: 实现BFS遍历或其他遍历方式
}

/**
 * @brief 验证BST是否有效
 * @param bst BST指针
 * @param order 使用的验证方法（前序、中序、后序）
 * @return 有效返回true，无效返回false
 */
bool is_bst_valid(const BST* bst, TraverseOrder order) {
	if (!bst || order > 2 || order < 0) {
		return false;
	}

	bool isValid = false;

	switch (order) {
	case PRE_ORDER:
		isValid = pre_order_traverse(bst->root, LLONG_MIN, LLONG_MAX);
		break;
	case IN_ORDER: {
		long long pre = LLONG_MIN;
		isValid = in_order_traverse(bst->root, &pre);
		break;
	}
	case POST_ORDER:
		isValid = (post_order_traverse(bst->root).max != LLONG_MAX);
		break;
	}
	if (isValid) {
		printf("The tree is a valid Binary Search Tree!\n");
	}
	else {
		printf("The tree is not a valid Binary Search Tree!\n");
	}
	return isValid;
}

// ==================== 工具函数 ====================

/**
 * @brief 获取BST中的最小值
 * @param bst BST指针
 * @return 最小值指针，空树返回NULL
 */
const void* bst_min(const BST* bst) {
	if (!bst || !bst->root) {
		return NULL;
	}
	return _find_min_node(bst->root)->data;
}

/**
 * @brief 获取BST中的最大值
 * @param bst BST指针
 * @return 最大值指针，空树返回NULL
 */
const void* bst_max(const BST* bst) {
	if (!bst || !bst->root) {
		return NULL;
	}
	return _find_max_node(bst->root)->data;
}

/**
 * @brief 获取BST的最大深度
 * @param bst BST指针
 * @return 最大深度，空树返回-1
 */
int get_maxDepth(const BST* bst) {
	if (!bst || !bst->root) {
		return -1;
	}
	return get_max_depth(bst->root);
}

/**
 * @brief 获取BST的最小深度
 * @param bst BST指针
 * @return 最小深度，空树返回-1
 */
int get_minDepth(const BST* bst) {
	if (!bst || !bst->root) {
		return -1;
	}
	return get_min_depth(bst->root);
}

/**
 * @brief 获取第k小的元素
 * @param bst BST指针
 * @param k 排名（从1开始）
 * @return 第k小的元素值，无效输入返回-1
 */
int get_kthSmallest(const BST* bst, int k) {
	if (!bst || k == 0 || bst->size < k) {
		return -1;
	}
	int kth = k;
	return get_kth(bst->root, &kth);
}

/**
 * @brief 检查BST是否为空
 * @param bst BST指针
 * @return 为空返回true，否则返回false
 */
bool bst_empty(const BST* bst) {
	return !bst || bst->size == 0;
}

/**
 * @brief 获取BST中元素的数量
 * @param bst BST指针
 * @return 元素数量
 */
size_t bst_size(const BST* bst) {
	return bst ? bst->size : 0;
}
