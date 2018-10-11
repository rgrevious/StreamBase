//#pragma once
#include "Event.h"
#include <queue> 
using namespace std;

class CEventManager
{
public:

	//prioritize event queue by user type
	//ie; admin or host events should be processed first
	class CCompareEvent {
	public:
		bool operator()(CStreamEvent& event1, CStreamEvent& event2) // Returns true if t1 has higher privileges than t2
		{
			return event1 < event2;
		}
	};
	class CCompareEventPtr {
	public:
		bool operator()(CStreamEvent* event1, CStreamEvent* event2) 
		{
			return event1->GetPriority() < event2->GetPriority();
		}
	};

	CEventManager() {};
	CEventManager(int size) { SetMaxSize(size); }
	~CEventManager() {};

	//server callback when a user has updated their profile
	virtual void OnUserUpdated(CUser & user){}
	//server callback when user is requesting login
	virtual CUser * OnUserLogin(CUser & user) { return NULL; }//return a pointer to the object from storage

	void SetMaxSize(int size) {
		m_maxHeapSize = size;
	}

	//events can be requests to do something
	//events are considered requests if they were received and added the request queue
	bool AddRequest(CStreamEvent & event) {
		//m_queue.push(event);//std does not work well with referenced CObjects
		m_requests.push(new CStreamEvent(event));//use pointers instead of object references
		m_heapSize = m_requests.size() * sizeof(event);
		return true;
	}

	//treated as true events waiting to update users about a change in state
	bool AddEvent(CStreamEvent & event) {
		//m_queue.push(event);//std does not work well with referenced CObjects
		m_responses.push(new CStreamEvent(event));
		event.SetEvent(EACK);
		m_heapSize = m_responses.size() * sizeof(event);
		return true;
	}

	//get the next event based on user priority
	void GetNextEvent(CStreamEvent ** pevent) {
		*pevent = m_requests.top();
		m_requests.pop();

	}
	
	//main event routing
	bool GetResponse(CStreamEvent & requestEvent, CStreamEvent * presponseEvent);


private:
	//priority_queue<CStreamEvent, vector<CStreamEvent>, CCompareEvent > m_queue;
	priority_queue<CStreamEvent*, vector<CStreamEvent *>, CCompareEventPtr > m_requests;
	priority_queue<CStreamEvent*, vector<CStreamEvent *>, CCompareEventPtr > m_responses;
	int m_heapSize, m_maxHeapSize;
};