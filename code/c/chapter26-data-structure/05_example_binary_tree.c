/**
 * @file 05_example_binary_tree.c
 * @brief 二叉树示例: 创建、遍历、搜索、BST插入
 * @description 对应文档: 26-链表与数据结构
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct TreeNode {
    int data;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

TreeNode *tree_node_create(int data) {
    TreeNode *n = (TreeNode *)malloc(sizeof(TreeNode));
    if (n) {
        n->data = data;
        n->left = NULL;
        n->right = NULL;
    }
    return n;
}

void tree_preorder(const TreeNode *root) {
    if (!root) return;
    printf("%d ", root->data);
    tree_preorder(root->left);
    tree_preorder(root->right);
}

void tree_inorder(const TreeNode *root) {
    if (!root) return;
    tree_inorder(root->left);
    printf("%d ", root->data);
    tree_inorder(root->right);
}

void tree_postorder(const TreeNode *root) {
    if (!root) return;
    tree_postorder(root->left);
    tree_postorder(root->right);
    printf("%d ", root->data);
}

TreeNode *bst_insert(TreeNode *root, int data) {
    if (!root) return tree_node_create(data);
    if (data < root->data) {
        root->left = bst_insert(root->left, data);
    } else if (data > root->data) {
        root->right = bst_insert(root->right, data);
    }
    return root;
}

TreeNode *bst_search(const TreeNode *root, int data) {
    if (!root) return NULL;
    if (data == root->data) return (TreeNode *)root;
    if (data < root->data) return bst_search(root->left, data);
    return bst_search(root->right, data);
}

TreeNode *bst_find_min(TreeNode *root) {
    while (root && root->left) root = root->left;
    return root;
}

TreeNode *bst_delete(TreeNode *root, int data) {
    if (!root) return NULL;
    if (data < root->data) {
        root->left = bst_delete(root->left, data);
    } else if (data > root->data) {
        root->right = bst_delete(root->right, data);
    } else {
        if (!root->left) {
            TreeNode *temp = root->right;
            free(root);
            return temp;
        }
        if (!root->right) {
            TreeNode *temp = root->left;
            free(root);
            return temp;
        }
        TreeNode *min_node = bst_find_min(root->right);
        root->data = min_node->data;
        root->right = bst_delete(root->right, min_node->data);
    }
    return root;
}

int tree_height(const TreeNode *root) {
    if (!root) return 0;
    int left_h = tree_height(root->left);
    int right_h = tree_height(root->right);
    return (left_h > right_h ? left_h : right_h) + 1;
}

void tree_destroy(TreeNode *root) {
    if (!root) return;
    tree_destroy(root->left);
    tree_destroy(root->right);
    free(root);
}

void demo_tree_traversal(void) {
    printf("\n=== demo_tree_traversal ===\n");

    TreeNode *root = tree_node_create(1);
    root->left = tree_node_create(2);
    root->right = tree_node_create(3);
    root->left->left = tree_node_create(4);
    root->left->right = tree_node_create(5);
    root->right->left = tree_node_create(6);
    root->right->right = tree_node_create(7);

    printf("    1\n");
    printf("   / \\\n");
    printf("  2   3\n");
    printf(" / \\ / \\\n");
    printf("4  5 6  7\n\n");

    printf("前序遍历(根左右): ");
    tree_preorder(root);
    printf("\n");

    printf("中序遍历(左根右): ");
    tree_inorder(root);
    printf("\n");

    printf("后序遍历(左右根): ");
    tree_postorder(root);
    printf("\n");

    printf("树高度: %d\n", tree_height(root));

    tree_destroy(root);
}

void demo_bst_operations(void) {
    printf("\n=== demo_bst_operations ===\n");

    TreeNode *root = NULL;
    int values[] = {50, 30, 70, 20, 40, 60, 80};
    printf("插入: ");
    for (int i = 0; i < 7; i++) {
        printf("%d ", values[i]);
        root = bst_insert(root, values[i]);
    }
    printf("\n");

    printf("中序遍历(应有序): ");
    tree_inorder(root);
    printf("\n");

    TreeNode *found = bst_search(root, 40);
    printf("查找40: %s\n", found ? "找到" : "未找到");

    found = bst_search(root, 45);
    printf("查找45: %s\n", found ? "找到" : "未找到");

    printf("\n删除30:\n");
    root = bst_delete(root, 30);
    printf("中序遍历: ");
    tree_inorder(root);
    printf("\n");

    printf("删除50(根节点):\n");
    root = bst_delete(root, 50);
    printf("中序遍历: ");
    tree_inorder(root);
    printf("\n");

    tree_destroy(root);
}

void demo_bst_properties(void) {
    printf("\n=== demo_bst_properties ===\n");
    printf("BST(二叉搜索树)性质:\n");
    printf("  1. 左子树所有节点值 < 根节点值\n");
    printf("  2. 右子树所有节点值 > 根节点值\n");
    printf("  3. 左右子树也是BST\n\n");

    printf("BST操作复杂度:\n");
    printf("  查找: 平均O(log n), 最坏O(n)\n");
    printf("  插入: 平均O(log n), 最坏O(n)\n");
    printf("  删除: 平均O(log n), 最坏O(n)\n");
    printf("  中序遍历: O(n), 得到有序序列\n\n");

    printf("最坏情况(退化为链表):\n");
    TreeNode *worst = NULL;
    for (int i = 1; i <= 5; i++) worst = bst_insert(worst, i);
    printf("  插入1,2,3,4,5 -> 高度=%d (退化为链表)\n", tree_height(worst));
    tree_destroy(worst);

    printf("\n遍历应用:\n");
    printf("  前序: 复制树、表达式前缀表示\n");
    printf("  中序: BST排序、表达式中缀表示\n");
    printf("  后序: 删除树、表达式后缀表示\n");
}

int main(void) {
    printf("二叉树示例: 创建、遍历、搜索、BST插入\n");

    demo_tree_traversal();
    demo_bst_operations();
    demo_bst_properties();

    printf("\n所有演示完成!\n");
    return 0;
}
