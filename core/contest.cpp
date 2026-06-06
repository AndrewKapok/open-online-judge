#include "contest.h"
using nlohmann::json;

contest::contest(/* args */)
{
}

/**
 * @brief 从 JSON 数据构造比赛对象
 * @param import 包含比赛数据的 JSON 对象
 */
contest::contest(json import)
{
    from_json(import, *this);
}

contest::~contest()
{
}

/**
 * @brief 导出比赛为 JSON
 * @return 包含比赛完整数据的 JSON 对象
 */
json contest::exportJSON() const
{
    json result;
    to_json(result, *this);
    return result;
}

/**
 * @brief 将 testCase 序列化为 JSON
 * @param j  输出 JSON 对象
 * @param obj 源 testCase 对象
 */
void to_json(json &j, const testCase &obj)
{
    j = json{
        {"inputPath", obj.inputPath},
        {"outputPath", obj.outputPath},
        {"memeryLimit", obj.memeryLimit},
        {"timeLimit", obj.timeLimit},
        {"score", obj.score}
    };
}

/**
 * @brief 从 JSON 反序列化 testCase
 * @param j   输入 JSON 对象
 * @param obj 目标 testCase 对象
 */
void from_json(const json &j, testCase &obj)
{
    j.at("inputPath").get_to(obj.inputPath);
    j.at("outputPath").get_to(obj.outputPath);
    j.at("memeryLimit").get_to(obj.memeryLimit);
    j.at("timeLimit").get_to(obj.timeLimit);
    j.at("score").get_to(obj.score);
}

/**
 * @brief 将 judgeConfig 序列化为 JSON
 * @param j  输出 JSON 对象
 * @param obj 源 judgeConfig 对象
 */
void to_json(json &j, const judgeConfig &obj)
{
    j = json{
        {"caseNumber", obj.caseNumber},
        {"compileArgv", obj.compileArgv},
        {"caseList", obj.caseList}
    };
}

/**
 * @brief 从 JSON 反序列化 judgeConfig
 * @param j   输入 JSON 对象
 * @param obj 目标 judgeConfig 对象
 */
void from_json(const json &j, judgeConfig &obj)
{
    j.at("caseNumber").get_to(obj.caseNumber);
    j.at("compileArgv").get_to(obj.compileArgv);
    j.at("caseList").get_to(obj.caseList);
}

/**
 * @brief 将 problem 序列化为 JSON
 * @param j  输出 JSON 对象
 * @param obj 源 problem 对象
 */
void to_json(json &j, const problem &obj)
{
    j = json{
        {"id", obj.id},
        {"markdownFilePath", obj.markdownFilePath},
        {"htmlFilePath", obj.htmlFilePath},
        {"disable", obj.disable},
        {"conf", obj.conf}
    };
}

/**
 * @brief 从 JSON 反序列化 problem
 * @param j   输入 JSON 对象
 * @param obj 目标 problem 对象
 */
void from_json(const json &j, problem &obj)
{
    j.at("id").get_to(obj.id);
    j.at("markdownFilePath").get_to(obj.markdownFilePath);
    j.at("htmlFilePath").get_to(obj.htmlFilePath);
    j.at("disable").get_to(obj.disable);
    j.at("conf").get_to(obj.conf);
}

/**
 * @brief 将 contest 序列化为 JSON
 * @param j  输出 JSON 对象
 * @param obj 源 contest 对象
 */
void to_json(json &j, const contest &obj)
{
    j = json{
        {"disable", obj.disable},
        {"startTime", obj.startTime},
        {"endTime", obj.endTime},
        {"id", obj.id},
        {"problemCount", obj.problemCount},
        {"problemList", obj.problemList}
    };
}

/**
 * @brief 从 JSON 反序列化 contest
 * @param j   输入 JSON 对象
 * @param obj 目标 contest 对象
 */
void from_json(const json &j, contest &obj)
{
    j.at("disable").get_to(obj.disable);
    j.at("startTime").get_to(obj.startTime);
    j.at("endTime").get_to(obj.endTime);
    j.at("id").get_to(obj.id);
    j.at("problemCount").get_to(obj.problemCount);
    j.at("problemList").get_to(obj.problemList);
}