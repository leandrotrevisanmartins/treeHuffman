#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <codecvt>
#include <locale>
#include <vector>
#include <unordered_map>
#include <bitset>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <clocale>

constexpr int UNICODE_RANGE = 65536;

namespace tree {
    // Estrutura do nó da árvore de Huffman
    struct node {
        wchar_t value;
        int frequency;
        node* left;
        node* right;
    };

    // Functor para comparação de nós na fila de prioridade
    struct compare {
        bool operator()(node* l, node* r) {
            return l->frequency > r->frequency;
        }
    };

    // Cria um novo nó
    node* create_node(wchar_t value, int frequency) {
        node* p = new node;
        p->value = value;
        p->frequency = frequency;
        p->left = p->right = nullptr;
        return p;
    }

    // Constrói a árvore de Huffman a partir das frequências dos caracteres
    node* build_huffman_tree(int* frequencies) {
        std::priority_queue<node*, std::vector<node*>, compare> pq;

        for (int i = 0; i < UNICODE_RANGE; i++) {
            wchar_t character = static_cast<wchar_t>(i);
            if (frequencies[i] != 0 && std::iswprint(character)) {
                pq.push(create_node(character, frequencies[i]));
            }
        }

        while (pq.size() > 1) {
            node* left = pq.top(); pq.pop();
            node* right = pq.top(); pq.pop();
            node* new_node = create_node(L'\0', left->frequency + right->frequency);
            new_node->left = left;
            new_node->right = right;
            pq.push(new_node);
        }

        return pq.empty() ? nullptr : pq.top();
    }

    // Deleta a árvore de Huffman para liberar memória
    void delete_tree(node* root) {
        if (root) {
            delete_tree(root->left);
            delete_tree(root->right);
            delete root;
        }
    }

    // Gera os códigos de Huffman para cada caractere a partir da árvore
    void generate_codes(node* root, const std::wstring& prefix, std::unordered_map<wchar_t, std::wstring>& codes) {
        if (!root) return;
        if (!root->left && !root->right) {
            codes[root->value] = prefix;
        }
        generate_codes(root->left, prefix + L"0", codes);
        generate_codes(root->right, prefix + L"1", codes);
    }

    // Exibe a árvore de Huffman (somente os nós folhas)
    void show(node* root) {
        if (!root) return;
        std::queue<node*> q;
        q.push(root);
        while (!q.empty()) {
            int level_size = q.size();
            for (int i = 0; i < level_size; i++) {
                node* current = q.front();
                q.pop();
                if (!current->left && !current->right) {
                    if (std::iswprint(current->value)) {
                        std::wcout << current->value << L" (" << current->frequency << L") ";
                    }
                }
                if (current->left) q.push(current->left);
                if (current->right) q.push(current->right);
            }
            std::wcout << std::endl;
        }
    }

    // Exibe os códigos de Huffman e suas frequências
    void show_codes_with_frequencies(const std::unordered_map<wchar_t, std::wstring>& codes, int* frequencies) {
        std::wcout << L"Caractere - Frequência - Código\n";
        for (const auto& pair : codes) {
            wchar_t character = pair.first;
            std::wstring code = pair.second;
            int frequency = frequencies[character];
            std::wcout << character << L" - " << frequency << L" - " << code << std::endl;
        }
    }

    // Converte uma string wide para UTF-8
    std::string to_utf8(const std::wstring& wstr) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.to_bytes(wstr);
    }

    // Escapa caracteres especiais em rótulos para o DOT
    std::string escape_label(const std::string& label) {
        std::string escaped_label;
        for (char c : label) {
            if (c == '\\' || c == '"') {
                escaped_label += '\\';
            }
            escaped_label += c;
        }
        return escaped_label;
    }

    // Exporta a árvore de Huffman para um arquivo DOT
    void export2dot(const node* root, const std::string& filename) {
        std::ofstream dot(filename);
        dot << "digraph G {\n";
        std::queue<const node*> q;
        q.push(root);

        while (!q.empty()) {
            const node* current = q.front();
            q.pop();
            std::string node_name = "node" + std::to_string(reinterpret_cast<uintptr_t>(current));

            std::string label;
            if (current->value == L'\0') {
                label = std::to_string(current->frequency);
            } else {
                std::wstring value_str(1, current->value);
                label = to_utf8(value_str) + " - " + std::to_string(current->frequency);
            }

            label = escape_label(label);

            dot << "    " << node_name << " [label=\"" << label << "\"];\n";

            if (current->left) {
                std::string left_node_name = "node" + std::to_string(reinterpret_cast<uintptr_t>(current->left));
                dot << "    " << node_name << " -> " << left_node_name << " [label=\"0\"];\n";
                q.push(current->left);
            }
            if (current->right) {
                std::string right_node_name = "node" + std::to_string(reinterpret_cast<uintptr_t>(current->right));
                dot << "    " << node_name << " -> " << right_node_name << " [label=\"1\"];\n";
                q.push(current->right);
            }
        }
        dot << "}\n";
    }

    // Desenha a árvore de Huffman usando o Graphviz
    void draw(const node* root) {
        const std::string dot_filename = "huffman_tree.dot";
        export2dot(root, dot_filename);

        std::system(("dot -Tx11 " + dot_filename).c_str());
    }
    }

    // Comprime o texto usando os códigos de Huffman
    std::wstring compress_file(const std::wstring& text, const std::unordered_map<wchar_t, std::wstring>& huffman_codes) {
        std::wstringstream compressed_text;
        for (wchar_t ch : text) {
            if (huffman_codes.find(ch) != huffman_codes.end()) {
                compressed_text << huffman_codes.at(ch);
            }
        }
        return compressed_text.str();
    }
    // Obtém o tamanho do arquivo
    std::streamsize get_file_size(const std::wstring& filename) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::string filename_str = converter.to_bytes(filename);
        std::ifstream file(filename_str, std::ios::binary | std::ios::ate);
        return file.tellg();
    }
    // Gera um arquivo CSV com os códigos de Huffman, frequências e tamanhos de arquivo
    void generate_csv(const std::unordered_map<wchar_t, std::wstring>& huffman_codes, int* frequencies, std::streamsize original_size, std::streamsize compressed_size) {
        std::ofstream csv_file("Huffman_Codes.csv");

        // Escreve o cabeçalho
        csv_file << "Caractere,Frequência,Código\n";

        // Escreve os dados
        for (const auto& pair : huffman_codes) {
            wchar_t character = pair.first;
            std::wstring code = pair.second;
            int frequency = frequencies[character];

            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            std::string char_str = converter.to_bytes(std::wstring(1, character));

            csv_file << char_str << "," << frequency << "," << converter.to_bytes(code) << "\n";
        }

        // Escreve os tamanhos e a comparação
        csv_file << "\nTamanho original (bytes)," << original_size << "\n";
        csv_file << "Tamanho compactado (bytes)," << compressed_size << "\n";
        double comparison = static_cast<double>(compressed_size) / original_size * 100;
        csv_file << "Comparação (%)," << comparison << "\n";

        csv_file.close();
    }

int main() {
    std::setlocale(LC_ALL, "en_US.UTF-8");
    using namespace tree;

    std::wstring nome;
    std::wcout << L"Digite o nome do arquivo: ";
    std::wcin >> nome;

    std::wstring caminho = L"../" + nome;

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string arquivo = converter.to_bytes(caminho);

    std::streamsize tamanho_original = get_file_size(caminho);

    std::wifstream arq(arquivo);
    arq.imbue(std::locale(arq.getloc(), new std::codecvt_utf8<wchar_t>));

    if (!arq) {
        std::wcout << L"Erro ao abrir o arquivo\n";
        return 1;
    }

    int cont[UNICODE_RANGE] = {};
    std::wstring text;
    wchar_t ch;

    while (arq.get(ch)) {
        if (std::iswprint(ch)) {
            cont[ch]++;
            text += ch;
        }
    }

    arq.close();

    node* huffman_tree = build_huffman_tree(cont);

    std::wcout << L"\nÁrvore de Huffman (nós folhas):\n";
    show(huffman_tree);

    std::unordered_map<wchar_t, std::wstring> huffman_codes;
    generate_codes(huffman_tree, L"", huffman_codes);

    std::wcout << L"\nCódigos de Huffman com Frequências:\n";
    show_codes_with_frequencies(huffman_codes, cont);

    std::wstring compressed_bits = compress_file(text, huffman_codes);

    size_t compressed_size_bits = compressed_bits.size();
    size_t compressed_size_bytes = (compressed_size_bits + 7) / 8;

    std::wcout << L"Tamanho original do arquivo: " << tamanho_original << L" bytes\n";
    std::wcout << L"Tamanho estimado do arquivo compactado: " << compressed_size_bytes << L" bytes\n";

    double comparacao = static_cast<double>(compressed_size_bytes) / tamanho_original * 100;
    std::wcout << L"Comparação entre os arquivos: " << std::fixed << std::setprecision(2) << comparacao << L"%\n";

    generate_csv(huffman_codes, cont, tamanho_original, compressed_size_bytes);

    draw(huffman_tree);

    delete_tree(huffman_tree);

    return 0;
}
