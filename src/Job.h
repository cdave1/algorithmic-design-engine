/*
 * Copyright (c) 2017 David Petrie david@davidpetrie.com
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software. Permission is granted to anyone to use this software for
 * any purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim
 * that you wrote the original software. If you use this software in a product, an
 * acknowledgment in the product documentation would be appreciated but is not
 * required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef _JOB_H_
#define _JOB_H_

#include "json.h"
#include "LogoGenerator.h"
#include "PatternIterator.h"
#include "VertexBuffer.h"

#include <list>
#include <iostream>
#include <fstream>
#include <sstream>


class Job {

private:

    Job *m_next = nullptr;

    Json::Value m_input;

public:

    LogoGenerator *logoGenerator = nullptr;

    PatternIterator *patternIterator = nullptr;

    VertexBuffer *vertexBuffer = nullptr;

    std::string pngInputLocation = "";


    std::string pngOutputLocation = "";

    std::string svgOutputLocation = "";

    std::string svgMeshOutputLocation = "";

    bool exportFontMetaData = false;

public:

    Job() {}

    Job(const Json::Value &value) : m_input(value) {}

    Job * GetNext() const {
        return m_next;
    }

    void SetNext(Job *next) {
        m_next = next;
    }

};


class WorkOrder {
private:

    Json::Value m_input;

    std::list<Job *> m_jobs;

    Job *m_currentJob;


public:

    void AddJob(Job *job);

    void Next();


public:

    WorkOrder() : m_input(), m_jobs() {}

    WorkOrder(const Json::Value &value) : m_input(value), m_jobs() {}

};


#endif
