#include "Job.h"

#include <stdio.h>

void WorkOrder::AddJob(Job *job) {
    if (m_jobs.empty()) {
        m_currentJob = job;
        m_jobs.push_back(job);
    } else {
        m_currentJob->SetNext(job);
        m_jobs.push_back(job);
    }
}


void WorkOrder::Next() {
    m_currentJob = m_currentJob->GetNext();
}