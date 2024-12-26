#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <unordered_map>
#include <algorithm>

using namespace std;

typedef unordered_map<string, int> NgramMap;//词组对应频次

// 读取文件并解析为分词后的行
vector<vector<string>> read_file(const string& file_path)
{
    ifstream file(file_path);
    vector<vector<string>> lines;
    vector<string> words;
    string line;

    if (!file.is_open())
    {
        cerr << "无法打开文件: " << file_path << endl;
        return lines;
    }

    while (getline(file, line))
    {
        istringstream stream(line);
        string word;
        while (stream >> word)
        {
            // 遍历 word 中的字符，寻找指定的分隔符
            string temp;
            for (char c : word)
            {
                if (c == '.' || c == ',' || c == '!' || c == '?' ||
                    c == ':' || c == '\'' || c == '\"')
                {
                    // 如果之前已经有部分内容，先将其加入当前行
                    if (!temp.empty())
                    {
                        words.push_back(temp);
                        temp.clear();
                    }
                    // 当前分隔符作为独立单词加入
                    words.push_back(string(1, c));

                    // 如果需要换行，结束当前行并开启新行
                    if (!words.empty())
                    {
                        lines.push_back(words);
                        words.clear();
                    }
                }
                else
                {
                    temp += c;
                }
            }
            // 处理剩余的部分
            if (!temp.empty())
            {
                words.push_back(temp);
            }
        }
        // 一行结束，若当前行有内容，保存
        if (!words.empty())
        {
            lines.push_back(words);
            words.clear();
        }
    }

    return lines;
}

// 生成 n-gram 统计
NgramMap generate_ngram(const vector<string>& words, int n) 
{
    NgramMap ngram_map;
    //遍历句子
    for (size_t i = 0; i + n <= words.size(); i++) 
    {
        string ngram;
        for (int j = 0; j < n; ++j) 
        {
            if (j > 0) ngram += " ";
            ngram += words[i + j];//生成一个ngram
        }
        ngram_map[ngram]++;
    }
    return ngram_map;
}

// 计算 n-gram 精度
double calculate_ngram_precision(const vector<string>& candidate, const vector<string>& reference, int n) 
{
    NgramMap candidate_ngrams = generate_ngram(candidate, n);
    NgramMap reference_ngrams = generate_ngram(reference, n);

    double match = 0;
    double total = 0;

    for (const auto& pair : candidate_ngrams) 
    {
        match += min(pair.second, reference_ngrams[pair.first]);//计算匹配次数
        total += pair.second;
    }
    //保证大于0
    total= total > 0 ? match / total : 0.0;
    return total;
}



// 计算短句惩罚
double calculate_brevity_penalty(const vector<string>& candidate, const vector<string>& reference) 
{
    int c_length = candidate.size();
    int r_length = reference.size();

    if (c_length > r_length) 
    {
        return 1.0;
    }
    else //小于
    {
        return exp(1.0 - static_cast<double>(r_length) / static_cast<double>(c_length));
    }
}

//计算 BLEU 分数
double calculate_bleu(const vector<string>& candidate, const vector<string>& reference) 
{
    vector<double> precisions;
    for (int n = 1; n <= 4; n++) //加和
    {
        precisions.push_back(calculate_ngram_precision(candidate, reference, n));
    }

    //计算几何平均
    double temp,total=0.0;
    for (int i = 0; i < 4; i++)
    {
        temp = static_cast <double>(1) / static_cast <double>(i);
        temp *= log(precisions[i]>0.0? precisions[i]:0.0);//保证大于0
        total += temp;
    }
  
    double exp_result = exp(total>0.0?total:0.0);//保证大于0

    //短句惩罚
    double bp = calculate_brevity_penalty(candidate, reference);

    return bp * exp_result;
}

//计算所有行的平均 BLEU 分数
double calculate_average_bleu(const string& candidate_file, const string& reference_file) 
{
    vector<vector<string>> candidates = read_file(candidate_file);
    vector<vector<string>> references = read_file(reference_file);

    double total_bleu = 0.0;
    int count = 0;

    for (size_t i = 0; i < candidates.size() && i < references.size(); i++)
    {
        total_bleu += calculate_bleu(candidates[i], references[i]);
        count++;
    }

    return total_bleu / count;
}

int main() 
{
    string candidate_file = "translation_result.txt";
    string reference_file = "reference.txt";

    double average_bleu = calculate_average_bleu(candidate_file, reference_file);
    cout << "BLEU: " << average_bleu << endl;

    return 0;
}
