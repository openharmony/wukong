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
#include "statistics_event.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>

namespace OHOS {
namespace WuKong {
namespace {
using namespace std;
const uint32_t DECIMAL_LENGTH = 2;
const float PERCENTAGE = 100.0;
}  // namespace

void StatisticsElemnt::StatisticsDetail(vector<map<string, string>> srcDatas,
                                        map<string, shared_ptr<Table>> &destTables)
{
    SrcDatasPretreatment(srcDatas);
    // loop bundle stroe event statistics msg
    for (auto bundle : elementStatisticsMsg_) {
        DEBUG_LOG_STR("start bundlename{%s}", bundle.first.c_str());
        shared_ptr<ElementStatisticsMsg> curElementStatisticsMsgPtr = bundle.second;
        uint32_t curElementTypeLength = curElementStatisticsMsgPtr->elementTypes_.size();
        vector<string> line;
        shared_ptr<ElementStatisticsRecord> curElementStatisticsRecordPtr = nullptr;
        shared_ptr<ElementStatisticsRecord> curBundleAllStatisticsPtr = make_shared<ElementStatisticsRecord>();
        curBundleAllStatisticsPtr->elementType_ = "total";
        for (uint32_t i = 0; i < curElementTypeLength; i++) {
            curElementStatisticsRecordPtr = curElementStatisticsMsgPtr->elementTypeRecord_[i];
            UpdateLine(curElementStatisticsRecordPtr, curElementStatisticsMsgPtr->elementTypeTotal_, line);
            record_.push_back(line);
            curBundleAllStatisticsPtr->execTimes_ += curElementStatisticsRecordPtr->execTimes_;
        }

        UpdateLine(curBundleAllStatisticsPtr, curElementStatisticsMsgPtr->elementTypeTotal_, line);
        record_.push_back(line);
        shared_ptr<Table> table = make_shared<Table>(headers_, record_);
        table->SetName(bundle.first);
        table->SetDetail("element");
        destTables[bundle.first] = table;
        record_.clear();
    }

    GlobalElementTypesStatistics();
    shared_ptr<Table> globalTable = make_shared<Table>(headers_, record_);
    globalTable->SetName("all");
    globalTable->SetDetail("element");
    destTables["all"] = globalTable;
    record_.clear();
}

void StatisticsElemnt::SrcDatasPretreatment(std::vector<std::map<std::string, std::string>> srcDatas)
{
    map<string, shared_ptr<ElementStatisticsMsg>>::iterator elementStatisticsMsgIter;
    vector<string>::iterator globalElementTypesIter;
    for (auto srcData : srcDatas) {
        DEBUG_LOG_STR("bundlename{%s} | elementType{%s}", srcData["bundleName"].c_str(), srcData["event"].c_str());
        globalElementTypesIter = find(globalElementTypes_.begin(), globalElementTypes_.end(), srcData["event"]);
        if (globalElementTypesIter == globalElementTypes_.end()) {
            DEBUG_LOG_STR("push event{%s} to globalElementTypes_", srcData["event"].c_str());
            globalElementTypes_.push_back(srcData["event"]);
        }

        // check whether bundle is entered resolve create or reuse already exist  StatisticsMsgPtr
        shared_ptr<ElementStatisticsMsg> curStatisticsMsgPtr = make_shared<ElementStatisticsMsg>();
        elementStatisticsMsgIter = elementStatisticsMsg_.find(srcData["bundleName"]);
        if (elementStatisticsMsgIter != elementStatisticsMsg_.end()) {
            DEBUG_LOG_STR("use inited curStatisticsMsgPtr by bundleName{%s}", srcData["bundleName"].c_str());
            curStatisticsMsgPtr = elementStatisticsMsg_[srcData["bundleName"]];
        }
        // check whether elementType is entered resolve create or reuse already exist ElementStatisticsRecordPtr
        shared_ptr<ElementStatisticsRecord> curElementStatisticsRecordPtr = make_shared<ElementStatisticsRecord>();
        uint32_t index = curStatisticsMsgPtr->ElementTypesIndex(srcData["event"]);
        uint32_t curElementTypeTotal = curStatisticsMsgPtr->elementTypeTotal_;
        if (index != curStatisticsMsgPtr->elementTypes_.size()) {
            curElementStatisticsRecordPtr = curStatisticsMsgPtr->elementTypeRecord_[index];
            DEBUG_LOG_STR("use inited curElementStatisticsRecordPtr in index{%d} | event{%s}", index,
                          srcData["event"].c_str());
        }
        // update record msg
        curElementStatisticsRecordPtr->elementType_ = srcData["event"];
        curElementStatisticsRecordPtr->execTimes_++;

        if (curStatisticsMsgPtr->elementTypeRecord_.size() > index) {
            curStatisticsMsgPtr->elementTypeRecord_[index] = curElementStatisticsRecordPtr;
            curStatisticsMsgPtr->elementTypes_[index] = srcData["event"];
        } else {
            curStatisticsMsgPtr->elementTypeRecord_.push_back(curElementStatisticsRecordPtr);
            curStatisticsMsgPtr->elementTypes_.push_back(srcData["event"]);
        }

        curElementTypeTotal++;
        DEBUG_LOG_STR("curElementTypeTotal{%d}", curElementTypeTotal);
        curStatisticsMsgPtr->elementTypeTotal_ = curElementTypeTotal;
        elementStatisticsMsg_[srcData["bundleName"]] = curStatisticsMsgPtr;
        execCount_++;
    }
}
void StatisticsElemnt::UpdateLine(std::shared_ptr<ElementStatisticsRecord> ElementStatisticsRecordPtr,
                                  uint32_t elementTypeTotal, std::vector<std::string> &line)
{
    stringstream bufferStream;
    string curElementType = ElementStatisticsRecordPtr->elementType_;
    string curExecTimes = to_string(ElementStatisticsRecordPtr->execTimes_);
    string curProportionStr = "";
    if (elementTypeTotal > 0) {
        float proportion = (ElementStatisticsRecordPtr->execTimes_ * PERCENTAGE) / elementTypeTotal;
        bufferStream.str("");
        bufferStream << std::setiosflags(std::ios::fixed) << std::setprecision(DECIMAL_LENGTH) << proportion;
        curProportionStr = bufferStream.str() + "%";
    }

    DEBUG_LOG_STR("line content curElementType{%s} | curExecTimes{%s}", curElementType.c_str(), curExecTimes.c_str());
    line = {curElementType, curExecTimes, curProportionStr};
}

void StatisticsElemnt::GlobalElementTypesStatistics()
{
    vector<string> line;
    shared_ptr<ElementStatisticsRecord> globalAllStatisticsPtr = make_shared<ElementStatisticsRecord>();
    globalAllStatisticsPtr->elementType_ = "total";
    for (auto elementType : globalElementTypes_) {
        shared_ptr<ElementStatisticsRecord> elementStatisticsRecordPtr = make_shared<ElementStatisticsRecord>();
        elementStatisticsRecordPtr->elementType_ = elementType;
        for (auto bundle : elementStatisticsMsg_) {
            shared_ptr<ElementStatisticsMsg> curElementStatisticsMsgPtr = bundle.second;
            uint32_t index = curElementStatisticsMsgPtr->ElementTypesIndex(elementType);
            if (curElementStatisticsMsgPtr->elementTypeRecord_.size() > index) {
                shared_ptr<ElementStatisticsRecord> curElementStatisticsRecordPtr =
                    curElementStatisticsMsgPtr->elementTypeRecord_[index];
                elementStatisticsRecordPtr->execTimes_ += curElementStatisticsRecordPtr->execTimes_;
            }
        }
        globalAllStatisticsPtr->execTimes_ += elementStatisticsRecordPtr->execTimes_;
        UpdateLine(elementStatisticsRecordPtr, execCount_, line);
        record_.push_back(line);
    }
    UpdateLine(globalAllStatisticsPtr, execCount_, line);
    record_.push_back(line);
}
}  // namespace WuKong
}  // namespace OHOS
