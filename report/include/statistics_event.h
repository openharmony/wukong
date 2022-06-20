/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TEST_WUKONG_STATISTICS_EVENT_H
#define TEST_WUKONG_STATISTICS_EVENT_H

#include <iomanip>
#include <string>

#include "data_set.h"
#include "statistics.h"
#include "wukong_define.h"

namespace OHOS {
namespace WuKong {
class ElementStatisticsRecord {
public:
    std::string elementType_ = "";
    uint32_t execTimes_ = 0;
};

class ElementStatisticsMsg {
public:
    /*
     * @brief find elementType position in elementTypes_
     * @param elementType
     * @return index
     */
    uint32_t ElementTypesIndex(const std::string &elementType)
    {
        uint32_t index = elementTypes_.size();
        DEBUG_LOG_STR("elementTypes_.size{%d}", index);
        std::vector<std::string>::iterator elementTypesIter;
        elementTypesIter = find(elementTypes_.begin(), elementTypes_.end(), elementType);
        if (elementTypesIter != elementTypes_.end()) {
            index = elementTypesIter - elementTypes_.begin();
            DEBUG_LOG_STR("find index{%d}", index);
        }
        DEBUG_LOG_STR("find index{%d}", index);
        return index;
    }
    std::vector<std::string> elementTypes_;
    std::vector<std::shared_ptr<ElementStatisticsRecord>> elementTypeRecord_;
    uint32_t elementTypeTotal_ = 0;
};

class StatisticsElemnt : public Statistics {
public:
    StatisticsElemnt() = default;
    ~StatisticsElemnt() = default;
    void StatisticsDetail(std::vector<std::map<std::string, std::string>> srcDatas,
                          std::map<std::string, std::shared_ptr<Table>> &destTables);

private:
    /*
     * @brief statistics msg update to line
     * @param ElementStatisticsRecordPtr store statistics msg
     * @param elementTypeTotal Proportion to calculate the total
     * @param line output
     * @return void
     */
    void UpdateLine(std::shared_ptr<ElementStatisticsRecord> elementStatisticsRecordPtr, uint32_t elementTypeTotal,
                    std::vector<std::string> &line);
    /*
     * @brief Realize secondary classification and update statistics of source data through bundleName and element
     * methods
     * @param srcDatas filtered data
     * @return void
     */
    void SrcDatasPretreatment(std::vector<std::map<std::string, std::string>> srcDatas);
    /*
     * @brief Global Statistics for ElementTypesmethods
     * @return void
     */
    void GlobalElementTypesStatistics();

    // bundle map ElementStatisticsMsgPtr
    std::map<std::string, std::shared_ptr<ElementStatisticsMsg>> elementStatisticsMsg_;
    // all elementTypes record for global statistics used
    std::vector<std::string> globalElementTypes_;
    
    std::vector<std::string> headers_ = {"type", "execTimes", "proportion"};
    std::vector<std::vector<std::string>> record_;
    int execCount_ = 0;
};
}  // namespace WuKong
}  // namespace OHOS
#endif
