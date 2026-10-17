#include "trie.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib> // for malloc

TrieNode* createNode() {
    TrieNode* node = (TrieNode*)malloc(sizeof(TrieNode));
    node->isEnd = false;
    for(int i=0; i<ALPHABET_SIZE; i++)
        node->children[i] = nullptr;
    return node;
}

void insertCommand(TrieNode* root, const char* command) {
    TrieNode* curr = root;
    for(int i=0; command[i]; i++) {
        int idx = (int)command[i];
        if(!curr->children[idx])
            curr->children[idx] = createNode();
        curr = curr->children[idx];
    }
    curr->isEnd = true;
}

void dfs(TrieNode* node, const std::string& prefix, std::vector<std::string>& results) {
    if(node->isEnd)
        results.push_back(prefix);
    for(int i=0; i<ALPHABET_SIZE; i++) {
        if(node->children[i]) {
            dfs(node->children[i], prefix + (char)i, results);
        }
    }
}

void autocomplete(TrieNode* root, const char* prefix) {
    TrieNode* curr = root;
    for(int i=0; prefix[i]; i++) {
        int idx = (int)prefix[i];
        if(!curr->children[idx]) return;
        curr = curr->children[idx];
    }
    std::vector<std::string> results;
    dfs(curr, std::string(prefix), results);

    for(auto& s: results)
        std::cout << s << "  ";
    if(!results.empty())
        std::cout << "\n";
}
