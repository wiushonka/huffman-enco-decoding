#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <queue>
#include <fstream>

using namespace std;

struct node {
    int frequency;
    char character;
    node *left;
    node *right;
    node(int freq, char ch) : frequency(freq), character(ch), left(nullptr), right(nullptr) {}
};

struct pq_bode {
    char c;
    int freq;
    node *node_ptr;
    pq_bode(char ch, int f, node *n) : c(ch), freq(f), node_ptr(n) {}
};

struct compare_pq_bode {
    bool operator()(const pq_bode &a, const pq_bode &b) {
        return a.freq > b.freq;  
    }
};

unordered_map<char, int> get_frequency(const string& input) {
    unordered_map<char, int> frequency_map;
    for (auto c : input) {
        frequency_map[c]++;
    }
    return frequency_map;
}

node *build_huffman_tree(priority_queue<pq_bode, vector<pq_bode>, compare_pq_bode>& freq) {
    if(freq.size() <= 1) {
        return nullptr; 
    } else if (freq.size() == 2) {
        pq_bode x1 = freq.top(); 
        freq.pop();
        node *n1 = x1.node_ptr;
        
        pq_bode x2 = freq.top();
        freq.pop();
        node *n2 = x2.node_ptr;
        
        node *root = new node(x1.freq + x2.freq, '$');
        root->left = n1;
        root->right = n2;
        return root;
    }
    
    pq_bode x1 = freq.top(); 
    freq.pop();
    node *n1 = x1.node_ptr;
    
    pq_bode x2 = freq.top();
    freq.pop();
    node *n2 = x2.node_ptr;
    
    node *root = new node(x1.freq + x2.freq, '$');
    root->left = n1;
    root->right = n2;

    freq.push(pq_bode('$', x1.freq + x2.freq, root));

    return build_huffman_tree(freq);
}

void create_codes(node *root, unordered_map<char, string> &huffman_codes, string current_code) {
    if (!root) {
        return;
    }

    if (!root->left && !root->right) {
        huffman_codes[root->character] = current_code;
        return;
    }

    create_codes(root->left, huffman_codes, current_code + "0");
    create_codes(root->right, huffman_codes, current_code + "1");
}

void free_tree(node *root) {
    if (!root) {
        return;
    }
    free_tree(root->left);
    free_tree(root->right);
    delete root;
}

unordered_map<char, string> huffman_encode(const string& input) {
    unordered_map<char, int> frequency_map = get_frequency(input);
    if (frequency_map.empty()) {
        return {};
    }

    if (frequency_map.size() == 1) {
        char single_char = frequency_map.begin()->first;
        return {{single_char, "0"}};   
    }

    priority_queue<pq_bode, vector<pq_bode>, compare_pq_bode> freq;
    for (const auto& pair : frequency_map) {
        node* n = new node(pair.second, pair.first);
        freq.push(pq_bode(pair.first, pair.second, n));
    }

    node *root = build_huffman_tree(freq);

    unordered_map<char, string> huffman_codes; 
    create_codes(root,huffman_codes,"");
    
    free_tree(root);
    
    return huffman_codes;
}

void compress_file_with_huffman(const string &input_filename, const string &output_filename) {
    ifstream infile(input_filename);
    if (!infile.is_open()) {
        cerr << "Failed to open input file.\n";
        return;
    }

    string input_text((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
    infile.close();

    unordered_map<char, string> codes = huffman_encode(input_text);

    string encoded_text;
    for (char c : input_text) {
        encoded_text += codes[c];
    }

    ofstream outfile(output_filename);
    if (!outfile.is_open()) {
        cerr << "Failed to open output file.\n";
        return;
    }

    outfile << encoded_text;
    outfile.close();
}

void decode_compressed_file(const string &output_filename, const unordered_map<char, string> &encoding) {
    unordered_map<string, char> reverse_map;
    for (const auto &pair : encoding) {
        reverse_map[pair.second] = pair.first;
    }

    ifstream infile(output_filename);
    if (!infile.is_open()) {
        cerr << "Failed to open output file for reading.\n";
        return;
    }
    string encoded_bits((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
    infile.close();

    string current_bits;
    string decoded_text;
    for (char bit : encoded_bits) {
        current_bits += bit;
        if (reverse_map.count(current_bits)) {
            decoded_text += reverse_map[current_bits];
            current_bits.clear();
        }
    }

    cout << "Decoded text:\n" << decoded_text << endl;
}


int main() {
    string input;

    ifstream infile("input.txt");
    if (!infile.is_open()) {
        cerr << "Failed to open input.txt\n";
        return 1;
    }
    string input_text((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
    infile.close();

    unordered_map<char, string> encoding = huffman_encode(input_text);

    compress_file_with_huffman("input.txt", "output.txt");

    decode_compressed_file("output.txt", encoding);

    return 0;
}
