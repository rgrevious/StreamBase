#pragma once
#include <afx.h>
#include "EventHandler.h"
using namespace std;

class CMsg;
class CSerialize
{
public:
	virtual bool serializeToBuffer(void* pbuf, int bufSize)  { return false; }
	virtual bool deserializeFromBuffer(void* pbuf, int bufSize) { return false; }
	virtual bool deserializeFromMsg(CMsg &msg) { return false; }
};


class CMsg
{
	struct HEADER {
		int target_user_id;
		int data_size;
		EVENTTYPE event;
		void * pdata;
	};

public:
	CMsg() {};
	CMsg(int size) {
		m_ppacket = malloc(size);
		m_pHeader = (HEADER *)m_ppacket;
	}
	CMsg(void * buffer, int size)
	{
		memcpy(m_pHeader, buffer, size);

	}

	~CMsg() {};
	int size() {
		return m_size;
	}
	void setPriority(EPRIORITY priority) {
		m_priority = priority;
	}

	void create(void * pbuffer, int size)
	{
		memcpy((void *)m_pHeader, pbuffer, sizeof(HEADER));
		memcpy(m_pData, (byte *)pbuffer + sizeof(HEADER),size- sizeof(HEADER));
	}
	int getPriority() {
		return m_priority;
	}

	bool operator<(const CMsg& msg)
	{
		return m_priority < msg.m_priority;
	}
	bool operator>(const CMsg& msg)
	{
		return m_priority > msg.m_priority;
	}

private:
	void * m_ppacket;
	HEADER * m_pHeader;
	void * m_pData;
	int m_size;
	EPRIORITY m_priority;
	int	m_id;

};

