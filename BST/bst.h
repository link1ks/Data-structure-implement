#pragma once
#include <stddef.h>
#include <stdbool.h>

typedef struct BST BST;

typedef int (*CompareFunc)(const void* a, const void* b);

typedef enum {
	IN_ORDER,
	PRE_ORDER,
	POST_ORDER,
} TraverseOrder;

BST* bst_create(const size_t element_size, CompareFunc comparefunc);

void bst_destroy(BST** p_bst);

bool bst_insert(BST* bst, const void* element_data);

bool bst_remove(BST* bst, const void* element_data);

bool bst_search(const BST* bst, const void* key);

void bst_traverse(const BST* bst, TraverseOrder);

bool is_bst_valid(const BST* bst, TraverseOrder);

bool bst_empty(const BST* bst);

size_t bst_size(const BST* bst);

// 获得二叉搜索树中最小值
const void* bst_min(const BST* bst);

// 获得二叉搜索树中最大值
const void* bst_max(const BST* bst);

// 获得二叉搜索树中最大深度
int get_maxDepth(const BST* bst);

// 获得二叉搜索树中最小深度
int get_minDepth(const BST* bst);

// 获得二叉搜索树中第k小的元素
int get_kthSmallest(const BST* bst, int k);
