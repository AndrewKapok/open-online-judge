#pragma once
#include <string>
#include <vector>
#include "json/single_include/nlohmann/json.hpp"
using nlohmann::json;

/**
 * @brief 单个测试用例
 */
struct testCase
{
    std::string inputPath;   ///< 输入文件路径
    std::string outputPath;  ///< 标准输出文件路径
    unsigned int memeryLimit; ///< 内存限制（MB）
    unsigned int timeLimit;   ///< 时间限制（ms）
    double score;             ///< 该测试用例分值
};
void to_json(json &j, const testCase &obj);
void from_json(const json &j, testCase &obj);

/**
 * @brief 判题配置
 */
struct judgeConfig
{
    std::vector<testCase> caseList;  ///< 测试用例列表
    unsigned int caseNumber;          ///< 测试用例总数
    std::string compileArgv;          ///< 编译参数
};
void to_json(json &j, const judgeConfig &obj);
void from_json(const json &j, judgeConfig &obj);

/**
 * @brief 题目
 */
struct problem
{
    unsigned int id;                     ///< 题目 ID
    std::string markdownFilePath;        ///< Markdown 题面文件路径
    std::string htmlFilePath;            ///< HTML 题面文件路径
    bool disable;                        ///< 是否禁用
    judgeConfig conf;                    ///< 判题配置
};
void to_json(json &j, const problem &obj);
void from_json(const json &j, problem &obj);

/**
 * @brief 比赛
 */
class contest
{
private:
    bool disable;                            ///< 是否禁用
    unsigned long long startTime;            ///< 开始时间（Unix 时间戳）
    unsigned long long endTime;              ///< 结束时间（Unix 时间戳）
    unsigned int id;                         ///< 比赛 ID
    unsigned int problemCount;               ///< 题目数量
    std::vector<problem> problemList;        ///< 题目列表
public:
    contest();
    /**
     * @brief 从 JSON 构造比赛对象
     * @param import 包含比赛数据的 JSON 对象
     */
    contest(json import);
    /**
     * @brief 导出比赛为 JSON
     * @return 包含比赛数据的 JSON 对象
     */
    json exportJSON() const;
    ~contest();
    friend void to_json(json &j, const contest &obj);
    friend void from_json(const json &j, contest &obj);
};
