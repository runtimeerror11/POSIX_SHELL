#ifndef TRIE_H
#define TRIE_H

#include <string>
#include <vector>
#define ALPHABET_SIZE 128

struct TrieNode {
    bool isEnd;
    TrieNode* children[ALPHABET_SIZE];
};

TrieNode* createNode();
void insertCommand(TrieNode* root, const char* command);
void dfs(TrieNode* node, const std::string& prefix, std::vector<std::string>& results);
void autocomplete(TrieNode* root, const char* prefix);

#endif
