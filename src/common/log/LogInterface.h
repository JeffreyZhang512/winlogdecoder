/*
 * Copyright (C) 2023 Zhang, Jeffrey <jeffreyzh512@outlook.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
 
#ifndef LOGINTERFACE_H
#define LOGINTERFACE_H

#include <string>


typedef enum
{
    LOG_OK = 0,
    LOG_INFO,
    LOG_DEBUG,
    LOG_WARNING,
    LOG_ERROR,
    LOG_TEST,
    LOG_STATUS
}LogType;


//Generic Logging Interface.
class LogInterface
{
public:
    virtual ~LogInterface() {}
    virtual void Log(std::string log, LogType type = LOG_INFO) = 0;
};


#endif // LOGINTERFACE_H
